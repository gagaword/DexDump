//
// Created by GaGa on 25-5-14.
//

#include "ClassPrint.h"
#include <cstdio>
#include <cstring>
#include <string>
#include <sstream>
#include "log/log.h"
#include "core/DexContext.h"
#include "FormatUtil.h"

namespace dex::print
{
    // 打印方法信息的辅助函数
    void printMethodInfo(const dex::DexContext& context, const dex::ClassDefInfo::ClassDataInfo::EncodedMethodInfo& method, uint32_t index)
    {
        // 从方法索引获取完整方法信息
        if (method.methodIdx < context.getMethodIdsCount())
        {
            dex::MethodInfo methodInfo = context.getMethodInfo(method.methodIdx);
            
            // 格式化方法签名（包含参数列表）
            std::string signature = formatMethodSignature(methodInfo);
            
            // 简化返回类型显示
            std::string returnType = simplifyTypeName(methodInfo.returnType);
            
            printf("      [%u] %s %s (访问标志: %s, 代码偏移量: 0x%08X)\n", 
                   index, returnType.c_str(), signature.c_str(), 
                   dex::DexContext::getAccessFlagsString(method.accessFlags).c_str(),
                   method.codeOff);
            
            // 打印详细的参数信息（如果有）
            if (methodInfo.hasParameterList && !methodInfo.parameterTypes.empty())
            {
                printf("          参数列表(%zu):\n", methodInfo.parameterTypes.size());
                for (size_t i = 0; i < methodInfo.parameterTypes.size(); i++)
                {
                    printf("            [%zu] %s\n", i, methodInfo.parameterTypes[i].c_str());
                }
            }
        }
        else
        {
            printf("      [%u] 方法索引无效: %u (访问标志: %s, 代码偏移量: 0x%08X)\n", 
                   index, method.methodIdx, 
                   dex::DexContext::getAccessFlagsString(method.accessFlags).c_str(),
                   method.codeOff);
        }
    }

    void ClassPrint::print()
    {
        // 获取上下文
        const dex::DexContext& context = getContext();
        
        // 检查是否有效
        if (!context.isValid())
        {
            LOGE("DEX解析未完成或无效，无法打印Class表");
            return;
        }
        
        // 获取Class数量
        const uint32_t classCount = context.getClassDefsCount();
        if (classCount == 0)
        {
            printf("Class表为空\n");
            return;
        }
        
        printf("\n/-------------------------------------------------------------------------\\\n");
        printf("|                           DEX Class Table                              |\n");
        printf("+------+--------------------+--------------------+----------------------+\n");
        printf("| %-4s | %-18s | %-18s | %-20s |\n", "索引", "类名", "父类", "访问标志");
        printf("+------+--------------------+--------------------+----------------------+\n");
        
        // 打印Class表
        for (uint32_t i = 0; i < classCount; i++)
        {
            // 获取Class信息
            dex::ClassDefInfo classInfo = context.getClassDefInfo(i);
            
            // 简化类名显示（只保留最后一部分）
            std::string className = simplifyTypeName(classInfo.className);
            if (className.length() > 18)
            {
                className = className.substr(0, 15) + "...";
            }
            
            // 简化父类名显示
            std::string superClassName = classInfo.superClassName.empty() ? "(无)" : simplifyTypeName(classInfo.superClassName);
            if (superClassName.length() > 18)
            {
                superClassName = superClassName.substr(0, 15) + "...";
            }
            
            // 获取访问标志字符串
            std::string accessFlags = dex::DexContext::getAccessFlagsString(classInfo.accessFlags);
            if (accessFlags.length() > 20)
            {
                accessFlags = accessFlags.substr(0, 17) + "...";
            }
            
            // 打印行
            printf("| %4u | %-18s | %-18s | %-20s |\n", 
                   i, className.c_str(), superClassName.c_str(), accessFlags.c_str());
            
            // 每15行打印一次表头
            if ((i + 1) % 15 == 0 && i + 1 < classCount)
            {
                printf("+------+--------------------+--------------------+----------------------+\n");
                printf("| %-4s | %-18s | %-18s | %-20s |\n", "索引", "类名", "父类", "访问标志");
                printf("+------+--------------------+--------------------+----------------------+\n");
            }
        }
        
        printf("+------+--------------------+--------------------+----------------------+\n");
        
        // 打印接口和源文件信息
        printf("\n详细信息:\n");
        for (uint32_t i = 0; i < classCount; i++)
        {
            dex::ClassDefInfo classInfo = context.getClassDefInfo(i);
            
            printf("\n[%u] %s\n", i, classInfo.className.c_str());
            printf("  访问标志: %s\n", dex::DexContext::getAccessFlagsString(classInfo.accessFlags).c_str());
            printf("  父类: %s\n", classInfo.superClassName.empty() ? "(无)" : classInfo.superClassName.c_str());
            printf("  源文件: %s\n", classInfo.sourceFileName.empty() ? "(未知)" : classInfo.sourceFileName.c_str());
            
            // 显示类数据偏移量信息
            printf("  类数据偏移量: 0x%08X\n", classInfo.classDataOff);
            
            // 显示类数据内容
            if (classInfo.classDataOff != 0 && classInfo.classData.isLoaded)
            {
                printf("  类数据:\n");
                printf("    静态字段: %u\n", classInfo.classData.staticFieldsSize);
                printf("    实例字段: %u\n", classInfo.classData.instanceFieldsSize);
                printf("    直接方法: %u\n", classInfo.classData.directMethodsSize);
                printf("    虚拟方法: %u\n", classInfo.classData.virtualMethodsSize);
                
                // 打印静态字段
                if (classInfo.classData.staticFieldsSize > 0)
                {
                    printf("    静态字段列表:\n");
                    for (uint32_t j = 0; j < classInfo.classData.staticFields.size(); j++)
                    {
                        const auto& field = classInfo.classData.staticFields[j];
                        printf("      [%u] %s %s (访问标志: %s)\n", 
                               j, field.type.c_str(), field.name.c_str(), 
                               dex::DexContext::getAccessFlagsString(field.accessFlags).c_str());
                    }
                }
                
                // 打印实例字段
                if (classInfo.classData.instanceFieldsSize > 0)
                {
                    printf("    实例字段列表:\n");
                    for (uint32_t j = 0; j < classInfo.classData.instanceFields.size(); j++)
                    {
                        const auto& field = classInfo.classData.instanceFields[j];
                        printf("      [%u] %s %s (访问标志: %s)\n", 
                               j, field.type.c_str(), field.name.c_str(), 
                               dex::DexContext::getAccessFlagsString(field.accessFlags).c_str());
                    }
                }
                
                // 打印直接方法
                if (classInfo.classData.directMethodsSize > 0)
                {
                    printf("    直接方法列表:\n");
                    for (uint32_t j = 0; j < classInfo.classData.directMethods.size(); j++)
                    {
                        printMethodInfo(context, classInfo.classData.directMethods[j], j);
                    }
                }
                
                // 打印虚拟方法
                if (classInfo.classData.virtualMethodsSize > 0)
                {
                    printf("    虚拟方法列表:\n");
                    for (uint32_t j = 0; j < classInfo.classData.virtualMethods.size(); j++)
                    {
                        printMethodInfo(context, classInfo.classData.virtualMethods[j], j);
                    }
                }
            }
            
            if (!classInfo.interfaces.empty())
            {
                printf("  实现接口:\n");
                for (uint32_t j = 0; j < classInfo.interfaces.size(); j++)
                {
                    printf("    - %s\n", classInfo.interfaces[j].c_str());
                }
            }
            
            // 每3个类后添加分隔线
            if ((i + 1) % 3 == 0 && i + 1 < classCount)
            {
                printf("\n-------------------------------------------------------------\n");
            }
        }
        
        printf("\n共计: %u 个类定义\n", classCount);
    }
} 