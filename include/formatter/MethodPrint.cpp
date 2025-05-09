//
// Created by GaGa on 25-5-13.
//

#include "MethodPrint.h"
#include <cstdio>
#include <cstring>
#include <string>
#include <sstream>
#include "log/log.h"
#include "core/DexContext.h"
#include "FormatUtil.h"

namespace dex::print
{
    void MethodPrint::print()
    {
        // 获取上下文
        const dex::DexContext& context = getContext();
        
        // 检查是否有效
        if (!context.isValid())
        {
            LOGE("DEX解析未完成或无效，无法打印Method表");
            return;
        }
        
        // 获取Method数量
        const uint32_t methodCount = context.getMethodIdsCount();
        if (methodCount == 0)
        {
            printf("Method表为空\n");
            return;
        }
        
        printf("\n/---------------------------------------------------------------------\\\n");
        printf("|                         DEX Method Table                           |\n");
        printf("+------+----------------+----------------+----------------+----------+\n");
        printf("| %-4s | %-14s | %-14s | %-14s | %-8s |\n", "索引", "类名", "返回类型", "方法名", "参数数量");
        printf("+------+----------------+----------------+----------------+----------+\n");
        
        // 打印Method表
        for (uint32_t i = 0; i < methodCount; i++)
        {
            // 获取Method信息
            dex::MethodInfo methodInfo = context.getMethodInfo(i);
            
            // 简化类名显示
            std::string className = simplifyTypeName(methodInfo.className);
            if (className.length() > 15)
            {
                className = className.substr(0, 12) + "...";
            }
            
            // 简化返回类型名显示
            std::string returnType = simplifyTypeName(methodInfo.returnType);
            if (returnType.length() > 15)
            {
                returnType = returnType.substr(0, 12) + "...";
            }
            
            // 简化方法名
            std::string methodName = methodInfo.name;
            if (methodName.length() > 15)
            {
                methodName = methodName.substr(0, 12) + "...";
            }
            
            // 获取参数数量
            int paramCount = methodInfo.parameterTypes.size();
            
            // 打印行
            printf("| %4u | %-14s | %-14s | %-14s | %8d |\n", 
                i, className.c_str(), returnType.c_str(), methodName.c_str(), paramCount);
            
            // 每20行打印一次表头
            if ((i + 1) % 20 == 0 && i + 1 < methodCount)
            {
                printf("+------+----------------+----------------+----------------+----------+\n");
                printf("| %-4s | %-14s | %-14s | %-14s | %-8s |\n", "索引", "类名", "返回类型", "方法名", "参数数量");
                printf("+------+----------------+----------------+----------------+----------+\n");
            }
        }
        
        printf("+------+----------------+----------------+----------------+----------+\n");
        printf("| 共计: %-54u |\n", methodCount);
        printf("\\---------------------------------------------------------------------/\n");
        
        // 打印详细的方法信息
        printf("\n方法详细信息:\n");
        printf("==========================================\n");
        
        for (uint32_t i = 0; i < methodCount; i++)
        {
            dex::MethodInfo methodInfo = context.getMethodInfo(i);
            
            // 格式化方法签名
            std::string signature = formatMethodSignature(methodInfo);
            
            printf("[%u] %s\n", i, signature.c_str());
            printf("  类名: %s\n", methodInfo.className.c_str());
            
            // 打印参数列表
            if (methodInfo.hasParameterList && !methodInfo.parameterTypes.empty())
            {
                printf("  参数列表(%zu):\n", methodInfo.parameterTypes.size());
                for (size_t j = 0; j < methodInfo.parameterTypes.size(); j++)
                {
                    printf("    [%zu] %s\n", j, methodInfo.parameterTypes[j].c_str());
                }
            }
            else
            {
                printf("  参数列表: 无\n");
            }
            
            // 每10个方法后添加分隔线
            if ((i + 1) % 10 == 0 && i + 1 < methodCount)
            {
                printf("------------------------------------------\n");
            }
        }
        
        printf("==========================================\n");
    }
} 