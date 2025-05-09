//
// Created by GaGa on 25-5-12.
//

#include "FieldPrint.h"
#include <cstdio>
#include <cstring>
#include <string>
#include "log/log.h"

namespace dex::print
{
    void FieldPrint::print()
    {
        // 获取上下文
        const dex::DexContext& context = getContext();
        
        // 检查是否有效
        if (!context.isValid())
        {
            LOGE("DEX解析未完成或无效，无法打印Field表");
            return;
        }
        
        // 获取Field数量
        const uint32_t fieldCount = context.getFieldIdsCount();
        if (fieldCount == 0)
        {
            printf("Field表为空\n");
            return;
        }
        
        printf("\n/----------------------------------------------------------\\\n");
        printf("|                     DEX Field Table                     |\n");
        printf("+------+----------------+----------------+----------------+\n");
        printf("| %-4s | %-14s | %-14s | %-14s |\n", "索引", "类名", "类型", "名称");
        printf("+------+----------------+----------------+----------------+\n");
        
        // 打印Field表
        for (uint32_t i = 0; i < fieldCount; i++)
        {
            // 获取Field信息
            dex::FieldInfo fieldInfo = context.getFieldInfo(i);
            
            // 简化类名显示（只保留最后一部分）
            std::string className = fieldInfo.className;
            size_t classPos = className.find_last_of('/');
            if (classPos != std::string::npos && classPos < className.length() - 1)
            {
                className = className.substr(classPos + 1);
            }
            if (className.length() > 15)
            {
                className = className.substr(0, 12) + "...";
            }
            
            // 简化类型名显示
            std::string typeName = fieldInfo.typeName;
            size_t typePos = typeName.find_last_of('/');
            if (typePos != std::string::npos && typePos < typeName.length() - 1)
            {
                typeName = typeName.substr(typePos + 1);
            }
            if (typeName.length() > 15)
            {
                typeName = typeName.substr(0, 12) + "...";
            }
            
            // 简化字段名
            std::string fieldName = fieldInfo.name;
            if (fieldName.length() > 15)
            {
                fieldName = fieldName.substr(0, 12) + "...";
            }
            
            // 打印行
            printf("| %4u | %-14s | %-14s | %-14s |\n", 
                i, className.c_str(), typeName.c_str(), fieldName.c_str());
            
            // 每20行打印一次表头
            if ((i + 1) % 20 == 0 && i + 1 < fieldCount)
            {
                printf("+------+----------------+----------------+----------------+\n");
                printf("| %-4s | %-14s | %-14s | %-14s |\n", "索引", "类名", "类型", "名称");
                printf("+------+----------------+----------------+----------------+\n");
            }
        }
        
        printf("+------+----------------+----------------+----------------+\n");
        printf("| 共计: %-42u |\n", fieldCount);
        printf("\\----------------------------------------------------------/\n");
    }
} 