//
// Created by DexDump on 2025-5-15.
//

#include "CodePrint.h"
#include <cstdio>
#include <iomanip>
#include <sstream>
#include "log/log.h"
#include "core/DexContext.h"
#include "parser/CodeParser.h"
#include "FormatUtil.h"

namespace dex::print
{
    void CodePrint::printMethodCode(uint32_t methodIdx)
    {
        const dex::DexContext& context = getContext();
        
        // 检查方法索引是否有效
        if (methodIdx >= context.getMethodIdsCount())
        {
            LOGE("方法索引无效: %u", methodIdx);
            return;
        }
        
        // 获取方法信息
        dex::MethodInfo methodInfo = context.getMethodInfo(methodIdx);
        std::string signature = formatMethodSignature(methodInfo);
        
        printf("\n方法: %s\n", signature.c_str());
        printf("类: %s\n", methodInfo.className.c_str());
        printf("返回类型: %s\n", methodInfo.returnType.c_str());
        
        if (methodInfo.hasParameterList)
        {
            printf("参数列表(%zu):\n", methodInfo.parameterTypes.size());
            for (size_t i = 0; i < methodInfo.parameterTypes.size(); i++)
            {
                printf("  [%zu] %s\n", i, methodInfo.parameterTypes[i].c_str());
            }
        }
        else
        {
            printf("参数列表: 无\n");
        }
        
        // 获取所有类定义
        bool foundCode = false;
        for (uint32_t i = 0; i < context.getClassDefsCount(); i++)
        {
            dex::ClassDefInfo classInfo = context.getClassDefInfo(i);
            
            // 检查是否有类数据
            if (classInfo.classDataOff != 0 && classInfo.classData.isLoaded)
            {
                // 查找直接方法
                for (const auto& method : classInfo.classData.directMethods)
                {
                    if (method.methodIdx == methodIdx && method.codeOff != 0)
                    {
                        printf("访问标志: %s\n", dex::DexContext::getAccessFlagsString(method.accessFlags).c_str());
                        printf("代码偏移量: 0x%08X\n", method.codeOff);
                        printCode(method.codeOff);
                        foundCode = true;
                        break;
                    }
                }
                
                // 如果在直接方法中没找到，查找虚拟方法
                if (!foundCode)
                {
                    for (const auto& method : classInfo.classData.virtualMethods)
                    {
                        if (method.methodIdx == methodIdx && method.codeOff != 0)
                        {
                            printf("访问标志: %s\n", dex::DexContext::getAccessFlagsString(method.accessFlags).c_str());
                            printf("代码偏移量: 0x%08X\n", method.codeOff);
                            printCode(method.codeOff);
                            foundCode = true;
                            break;
                        }
                    }
                }
                
                if (foundCode)
                {
                    break;
                }
            }
        }
        
        if (!foundCode)
        {
            printf("\n该方法没有代码段（可能是抽象方法、接口方法或本地方法）\n");
        }
    }
    
    void CodePrint::printCode(uint32_t codeOffset)
    {
        const dex::DexContext& context = getContext();
        
        // 检查偏移量是否有效
        if (codeOffset == 0 || codeOffset >= context.getFileSize())
        {
            LOGE("代码偏移量无效: 0x%08X", codeOffset);
            return;
        }
        
        // 创建代码解析器
        dex::parser::CodeParser codeParser(context.getFileData(), context.getFileSize());
        
        // 解析代码
        dex::parser::CodeSectionInfo codeInfo = codeParser.parseCode(codeOffset);
        
        // 打印代码段信息
        printf("\n代码段信息:\n");
        printf("  寄存器数量: %u\n", codeInfo.registersSize);
        printf("  参数数量: %u\n", codeInfo.insSize);
        printf("  调用其他方法时的参数寄存器数量: %u\n", codeInfo.outsSize);
        printf("  try块数量: %u\n", codeInfo.triesSize);
        printf("  调试信息偏移量: 0x%08X\n", codeInfo.debugInfoOff);
        printf("  指令数量: %u 个16位字\n", codeInfo.insnsSize);
        
        // 打印指令
        if (!codeInfo.instructions.empty())
        {
            printf("\n指令列表:\n");
            printf("+--------+-------+--------------------+-----------------------------------+\n");
            printf("| %-6s | %-5s | %-18s | %-33s |\n", "偏移量", "大小", "助记符", "操作数");
            printf("+--------+-------+--------------------+-----------------------------------+\n");
            
            // 计算指令基础偏移量
            uint32_t baseOffset = codeOffset + offsetof(DexCode, insns);
            
            for (const auto& instruction : codeInfo.instructions)
            {
                printInstruction(instruction, baseOffset);
            }
            
            printf("+--------+-------+--------------------+-----------------------------------+\n");
            printf("| 共计: %-58u |\n", static_cast<uint32_t>(codeInfo.instructions.size()));
            printf("+--------+-------+--------------------+-----------------------------------+\n");
        }
        else
        {
            printf("\n没有发现指令\n");
        }
    }
    
    void CodePrint::print()
    {
        const dex::DexContext& context = getContext();
        
        // 检查是否有效
        if (!context.isValid())
        {
            LOGE("DEX解析未完成或无效，无法打印代码");
            return;
        }
        
        // 获取类数量
        const uint32_t classCount = context.getClassDefsCount();
        if (classCount == 0)
        {
            printf("没有发现类定义\n");
            return;
        }
        
        printf("\n/--------------------------------------------------------------------\\\n");
        printf("|                         DEX 方法代码概览                           |\n");
        printf("\\--------------------------------------------------------------------/\n");
        
        // 遍历所有类
        int methodCount = 0;
        for (uint32_t i = 0; i < classCount; i++)
        {
            dex::ClassDefInfo classInfo = context.getClassDefInfo(i);
            
            if (classInfo.classDataOff != 0 && classInfo.classData.isLoaded)
            {
                bool hasCode = false;
                
                // 检查是否有任何方法具有代码
                for (const auto& method : classInfo.classData.directMethods)
                {
                    if (method.codeOff != 0)
                    {
                        hasCode = true;
                        break;
                    }
                }
                
                if (!hasCode)
                {
                    for (const auto& method : classInfo.classData.virtualMethods)
                    {
                        if (method.codeOff != 0)
                        {
                            hasCode = true;
                            break;
                        }
                    }
                }
                
                // 如果有代码，才打印类信息
                if (hasCode)
                {
                    printf("\n类: %s\n", classInfo.className.c_str());
                    printf("===========================================================\n");
                    
                    // 打印直接方法的代码
                    for (const auto& method : classInfo.classData.directMethods)
                    {
                        if (method.codeOff != 0)
                        {
                            dex::MethodInfo methodInfo = context.getMethodInfo(method.methodIdx);
                            std::string signature = formatMethodSignature(methodInfo);
                            
                            printf("\n[%d] 方法: %s (直接方法)\n", ++methodCount, signature.c_str());
                            printf("访问标志: %s\n", dex::DexContext::getAccessFlagsString(method.accessFlags).c_str());
                            
                            // 创建代码解析器
                            dex::parser::CodeParser codeParser(context.getFileData(), context.getFileSize());
                            dex::parser::CodeSectionInfo codeInfo = codeParser.parseCode(method.codeOff);
                            
                            printf("代码概览: 寄存器数=%u, 指令数=%u\n", 
                                   codeInfo.registersSize, codeInfo.insnsSize);
                            
                            // 只显示部分指令
                            if (!codeInfo.instructions.empty())
                            {
                                printf("指令摘要 (前5条):\n");
                                uint32_t baseOffset = method.codeOff + offsetof(DexCode, insns);

                                uint32_t count = codeInfo.instructions.size();
                                for (uint32_t j = 0; j < count; j++)
                                {
                                    printf("  [0x%04X] %-16s %s\n", 
                                           codeInfo.instructions[j].offset * 2, 
                                           codeInfo.instructions[j].mnemonic.c_str(), 
                                           codeInfo.instructions[j].operands.c_str());
                                }
                                
                                if (codeInfo.instructions.size() > 5)
                                {
                                    printf("  ... 更多指令 (共%zu条) ...\n", codeInfo.instructions.size());
                                }
                            }
                            
                            printf("---------------------------------------------------------\n");
                        }
                    }
                    
                    // 打印虚拟方法的代码
                    for (const auto& method : classInfo.classData.virtualMethods)
                    {
                        if (method.codeOff != 0)
                        {
                            dex::MethodInfo methodInfo = context.getMethodInfo(method.methodIdx);
                            std::string signature = formatMethodSignature(methodInfo);
                            
                            printf("\n[%d] 方法: %s (虚拟方法)\n", ++methodCount, signature.c_str());
                            printf("访问标志: %s\n", dex::DexContext::getAccessFlagsString(method.accessFlags).c_str());
                            
                            // 创建代码解析器
                            parser::CodeParser codeParser(context.getFileData(), context.getFileSize());
                            parser::CodeSectionInfo codeInfo = codeParser.parseCode(method.codeOff);
                            
                            printf("代码概览: 寄存器数=%u, 指令数=%u\n", 
                                   codeInfo.registersSize, codeInfo.insnsSize);
                            
                            // 只显示部分指令
                            if (!codeInfo.instructions.empty())
                            {
                                printf("指令摘要 (前5条):\n");
                                uint32_t baseOffset = method.codeOff + offsetof(DexCode, insns);

                                uint32_t count = codeInfo.instructions.size();
                                for (uint32_t j = 0; j < count; j++)
                                {
                                    printf("  [0x%04X] %-16s %s\n", 
                                           codeInfo.instructions[j].offset * 2, 
                                           codeInfo.instructions[j].mnemonic.c_str(), 
                                           codeInfo.instructions[j].operands.c_str());
                                }
                                
                                if (codeInfo.instructions.size() > 5)
                                {
                                    printf("  ... 更多指令 (共%zu条) ...\n", codeInfo.instructions.size());
                                }
                            }
                            
                            printf("---------------------------------------------------------\n");
                        }
                    }
                }
            }
        }
        
        printf("\n总计: %d 个方法含有代码\n", methodCount);
    }
    
    void CodePrint::printInstruction(const dex::parser::InstructionInfo& instruction, uint32_t baseOffset)
    {
        // 计算指令的绝对地址
        uint32_t absoluteOffset = baseOffset + instruction.offset * 2;
        
        // 获取操作数的描述
        std::string operandDesc = getOperandDescription(instruction.opcode, instruction.operands);
        
        // 打印指令信息
        printf("| 0x%04X | %-5u | %-18s | %-33s |\n", 
               instruction.offset * 2,  // 相对偏移（以16位字为单位）
               instruction.length,      // 指令长度（以16位字为单位）
               instruction.mnemonic.c_str(), 
               operandDesc.c_str());
    }
    
    std::string CodePrint::getOperandDescription(uint16_t opcode, const std::string& operands)
    {
        const dex::DexContext& context = getContext();
        std::string description = operands;
        
        // 对特定操作码进行更详细的描述
        if (opcode == 0x1a || opcode == 0x1b)  // const-string, const-string/jumbo
        {
            // 从操作数中提取字符串索引
            size_t pos = operands.find(", ");
            if (pos != std::string::npos)
            {
                std::string indexStr = operands.substr(pos + 2);
                if (!indexStr.empty() && indexStr[0] == '#')
                {
                    indexStr = indexStr.substr(1);
                    try {
                        uint32_t stringIdx = std::stoul(indexStr);
                        std::string stringValue = context.getString(stringIdx);
                        
                        // 截断过长的字符串
                        if (stringValue.length() > 20)
                        {
                            stringValue = stringValue.substr(0, 17) + "...";
                        }
                        
                        // 转义特殊字符
                        std::stringstream ss;
                        for (char c : stringValue)
                        {
                            if (c == '\n') ss << "\\n";
                            else if (c == '\r') ss << "\\r";
                            else if (c == '\t') ss << "\\t";
                            else if (c < 32 || c > 126) ss << "\\x" << std::hex << std::setw(2) << std::setfill('0') << (int)(unsigned char)c;
                            else ss << c;
                        }
                        
                        description = operands.substr(0, pos + 2) + "\"" + ss.str() + "\"";
                    }
                    catch (...) {
                        // 保持原始描述
                    }
                }
            }
        }
        else if (opcode == 0x1c || opcode == 0x22)  // const-class, new-instance
        {
            // 从操作数中提取类型索引
            size_t pos = operands.find(", ");
            if (pos != std::string::npos)
            {
                std::string indexStr = operands.substr(pos + 2);
                if (!indexStr.empty() && indexStr[0] == '#')
                {
                    indexStr = indexStr.substr(1);
                    try {
                        uint32_t typeIdx = std::stoul(indexStr);
                        std::string typeName = context.getType(typeIdx);
                        
                        // 简化类型名称
                        std::string simplifiedType = simplifyTypeName(typeName);
                        
                        description = operands.substr(0, pos + 2) + simplifiedType;
                    }
                    catch (...) {
                        // 保持原始描述
                    }
                }
            }
        }
        else if ((opcode >= 0x6e && opcode <= 0x72) || // invoke-virtual等
                 (opcode >= 0x74 && opcode <= 0x78))   // invoke-static等
        {
            // 从操作数中提取方法索引
            size_t pos = operands.find("}, ");
            if (pos != std::string::npos)
            {
                std::string indexStr = operands.substr(pos + 3);
                try {
                    uint32_t methodIdx = std::stoul(indexStr);
                    dex::MethodInfo methodInfo = context.getMethodInfo(methodIdx);
                    
                    std::string methodName = methodInfo.name;
                    std::string className = simplifyTypeName(methodInfo.className);
                    std::string signature = formatMethodSignature(methodInfo);
                    
                    description = operands.substr(0, pos + 3) + className + "->" + signature;
                }
                catch (...) {
                    // 保持原始描述
                }
            }
        }
        else if (opcode >= 0x52 && opcode <= 0x58)  // iget, iput等
        {
            // 从操作数中提取字段索引
            size_t pos = operands.rfind(", ");
            if (pos != std::string::npos)
            {
                std::string indexStr = operands.substr(pos + 2);
                try {
                    uint32_t fieldIdx = std::stoul(indexStr);
                    dex::FieldInfo fieldInfo = context.getFieldInfo(fieldIdx);
                    
                    std::string fieldName = fieldInfo.name;
                    std::string className = simplifyTypeName(fieldInfo.className);
                    std::string typeName = simplifyTypeName(fieldInfo.typeName);
                    
                    description = operands.substr(0, pos + 2) + className + "->" + fieldName + ":" + typeName;
                }
                catch (...) {
                    // 保持原始描述
                }
            }
        }
        
        return description;
    }
} 