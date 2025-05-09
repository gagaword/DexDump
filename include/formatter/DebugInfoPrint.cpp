 //
// Created by DexDump on 2025-5-16.
//

#include "DebugInfoPrint.h"
#include <cstdio>
#include "log/log.h"
#include "core/DexContext.h"
#include "FormatUtil.h"

namespace dex::print
{
    void DebugInfoPrint::printMethodDebugInfo(uint32_t methodIdx)
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

        printf("\n方法调试信息: %s\n", signature.c_str());
        printf("类: %s\n", methodInfo.className.c_str());

        // 查找方法代码偏移量和调试信息
        bool foundDebugInfo = false;

        // 遍历所有类定义
        for (uint32_t i = 0; i < context.getClassDefsCount(); i++)
        {
            dex::ClassDefInfo classInfo = context.getClassDefInfo(i);

            // 检查是否有类数据
            if (classInfo.classDataOff != 0 && classInfo.classData.isLoaded)
            {
                // 查找直接方法
                for (const auto& method : classInfo.classData.directMethods)
                {
                    if (method.methodIdx == methodIdx)
                    {
                        if (method.codeOff == 0)
                        {
                            printf("\n该方法没有代码段（可能是抽象方法、接口方法或本地方法）\n");
                            return;
                        }

                        // 确保方法代码已解析
                        if (!method.codeInfo.isLoaded)
                        {
                            context.parseMethodCode(methodIdx);
                        }

                        if (method.codeInfo.debugInfoOff == 0)
                        {
                            printf("\n该方法没有调试信息\n");
                            return;
                        }

                        printDebugInfo(method.codeInfo.debugInfoOff);
                        foundDebugInfo = true;
                        break;
                    }
                }

                // 如果在直接方法中没找到，查找虚拟方法
                if (!foundDebugInfo)
                {
                    for (const auto& method : classInfo.classData.virtualMethods)
                    {
                        if (method.methodIdx == methodIdx)
                        {
                            if (method.codeOff == 0)
                            {
                                printf("\n该方法没有代码段（可能是抽象方法、接口方法或本地方法）\n");
                                return;
                            }

                            // 确保方法代码已解析
                            if (!method.codeInfo.isLoaded)
                            {
                                context.parseMethodCode(methodIdx);
                            }

                            if (method.codeInfo.debugInfoOff == 0)
                            {
                                printf("\n该方法没有调试信息\n");
                                return;
                            }

                            printDebugInfo(method.codeInfo.debugInfoOff);
                            foundDebugInfo = true;
                            break;
                        }
                    }
                }

                if (foundDebugInfo)
                {
                    break;
                }
            }
        }

        if (!foundDebugInfo)
        {
            printf("\n未找到该方法的调试信息\n");
        }
    }

    void DebugInfoPrint::printDebugInfo(uint32_t debugInfoOff)
    {
        const dex::DexContext& context = getContext();

        // 创建调试信息数据结构
        dex::DebugInfoData debugInfo;

        // 解析调试信息
        if (!context.parseDebugInfo(debugInfoOff, debugInfo))
        {
            LOGE("解析调试信息失败: 0x%08X", debugInfoOff);
            return;
        }

        // 打印调试信息概览
        printf("\n调试信息概览:\n");
        printf("  调试信息偏移量: 0x%08X\n", debugInfo.debugInfoOff);
        printf("  起始行号: %u\n", debugInfo.lineStart);

        // 打印参数名称
        if (debugInfo.parametersSize > 0)
        {
            printf("\n参数名称(%u):\n", debugInfo.parametersSize);
            for (uint32_t i = 0; i < debugInfo.parameterNames.size(); i++)
            {
                if (!debugInfo.parameterNames[i].empty())
                {
                    printf("  [%u] %s\n", i, debugInfo.parameterNames[i].c_str());
                }
                else
                {
                    printf("  [%u] <未命名>\n", i);
                }
            }
        }

        // 打印局部变量信息
        if (!debugInfo.localVars.empty())
        {
            printLocalVariables(debugInfo.localVars);
        }

        // 打印位置信息（行号映射）
        if (!debugInfo.positions.empty())
        {
            printPositions(debugInfo.positions);
        }
    }

    void DebugInfoPrint::print()
    {
        const dex::DexContext& context = getContext();

        // 检查是否有效
        if (!context.isValid())
        {
            LOGE("DEX解析未完成或无效，无法打印调试信息");
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
        printf("|                       DEX 方法调试信息概览                         |\n");
        printf("\\--------------------------------------------------------------------/\n");

        // 遍历所有类
        int methodWithDebugCount = 0;
        for (uint32_t i = 0; i < classCount; i++)
        {
            dex::ClassDefInfo classInfo = context.getClassDefInfo(i);

            if (classInfo.classDataOff != 0 && classInfo.classData.isLoaded)
            {
                bool hasDebugInfo = false;

                // 检查是否有任何方法具有调试信息
                for (const auto& method : classInfo.classData.directMethods)
                {
                    if (method.codeOff != 0 && method.codeInfo.isLoaded && method.codeInfo.debugInfoOff != 0)
                    {
                        hasDebugInfo = true;
                        break;
                    }
                }

                if (!hasDebugInfo)
                {
                    for (const auto& method : classInfo.classData.virtualMethods)
                    {
                        if (method.codeOff != 0 && method.codeInfo.isLoaded && method.codeInfo.debugInfoOff != 0)
                        {
                            hasDebugInfo = true;
                            break;
                        }
                    }
                }

                // 如果有调试信息，才打印类信息
                if (hasDebugInfo)
                {
                    printf("\n类: %s\n", classInfo.className.c_str());
                    printf("===========================================================\n");

                    // 打印直接方法的调试信息
                    for (const auto& method : classInfo.classData.directMethods)
                    {
                        if (method.codeOff != 0 && method.codeInfo.isLoaded && method.codeInfo.debugInfoOff != 0)
                        {
                            dex::MethodInfo methodInfo = context.getMethodInfo(method.methodIdx);
                            std::string signature = formatMethodSignature(methodInfo);

                            printf("\n[%d] 方法: %s (直接方法)\n", ++methodWithDebugCount, signature.c_str());

                            // 打印调试信息概览
                            const dex::DebugInfoData& debugInfo = method.codeInfo.debugInfo;
                            printf("  调试信息偏移量: 0x%08X\n", debugInfo.debugInfoOff);
                            printf("  起始行号: %u\n", debugInfo.lineStart);
                            printf("  局部变量数量: %zu\n", debugInfo.localVars.size());
                            printf("  位置映射数量: %zu\n", debugInfo.positions.size());

                            printf("---------------------------------------------------------\n");
                        }
                    }

                    // 打印虚拟方法的调试信息
                    for (const auto& method : classInfo.classData.virtualMethods)
                    {
                        if (method.codeOff != 0 && method.codeInfo.isLoaded && method.codeInfo.debugInfoOff != 0)
                        {
                            dex::MethodInfo methodInfo = context.getMethodInfo(method.methodIdx);
                            std::string signature = formatMethodSignature(methodInfo);

                            printf("\n[%d] 方法: %s (虚拟方法)\n", ++methodWithDebugCount, signature.c_str());

                            // 打印调试信息概览
                            const dex::DebugInfoData& debugInfo = method.codeInfo.debugInfo;
                            printf("  调试信息偏移量: 0x%08X\n", debugInfo.debugInfoOff);
                            printf("  起始行号: %u\n", debugInfo.lineStart);
                            printf("  局部变量数量: %zu\n", debugInfo.localVars.size());
                            printf("  位置映射数量: %zu\n", debugInfo.positions.size());

                            printf("---------------------------------------------------------\n");
                        }
                    }
                }
            }
        }

        printf("\n总计: %d 个方法含有调试信息\n", methodWithDebugCount);
    }

    void DebugInfoPrint::printLocalVariables(const std::vector<dex::LocalVarInfo>& localVars)
    {
        if (localVars.empty())
        {
            return;
        }

        printf("\n局部变量信息(%zu):\n", localVars.size());
        printf("+------+------------------+------------------+------------------+\n");
        printf("| %-4s | %-16s | %-16s | %-16s |\n", "寄存器", "变量名", "类型", "类型签名");
        printf("+------+------------------+------------------+------------------+\n");

        for (const auto& var : localVars)
        {
            printf("| v%-4u | %-16s | %-16s | %-16s |\n",
                   var.registerNum,
                   var.name.empty() ? "<未命名>" : var.name.c_str(),
                   var.type.empty() ? "<未知>" : simplifyTypeName(var.type).c_str(),
                   var.signature.empty() ? "-" : var.signature.c_str());
        }

        printf("+------+------------------+------------------+------------------+\n");
    }

    void DebugInfoPrint::printPositions(const std::vector<dex::PositionInfo>& positions)
    {
        if (positions.empty())
        {
            return;
        }

        printf("\n位置信息(%zu):\n", positions.size());
        printf("+--------+--------+\n");
        printf("| %-6s | %-6s |\n", "地址", "行号");
        printf("+--------+--------+\n");

        for (const auto& pos : positions)
        {
            printf("| 0x%04X | %-6u |\n", pos.address, pos.lineNum);
        }

        printf("+--------+--------+\n");
    }

    void DebugInfoPrint::printTryCatchBlocks(const std::vector<dex::TryBlockInfo>& tries)
    {
        if (tries.empty())
        {
            return;
        }

        printf("\nTry/Catch块(%zu):\n", tries.size());

        for (size_t i = 0; i < tries.size(); i++)
        {
            const auto& tryBlock = tries[i];

            printf("\n[%zu] Try块: 0x%04X - 0x%04X (长度: %u)\n",
                   i, tryBlock.startAddr, tryBlock.startAddr + tryBlock.insnCount - 1, tryBlock.insnCount);

            if (!tryBlock.catches.empty())
            {
                printf("  Catch处理器:\n");
                for (const auto& catchInfo : tryBlock.catches)
                {
                    printf("    类型: %-30s  处理器地址: 0x%04X\n",
                           catchInfo.typeName.empty() ? "<未知>" : simplifyTypeName(catchInfo.typeName).c_str(),
                           catchInfo.address);
                }
            }

            if (tryBlock.hasCatchAll)
            {
                printf("  Catch-All处理器: 0x%04X\n", tryBlock.catchAllAddr);
            }
        }
    }
}