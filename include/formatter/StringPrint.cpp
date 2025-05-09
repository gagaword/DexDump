//
// Created by GaGa on 25-5-10.
//

#include "StringPrint.h"
#include <cstdio>
#include <cstring>
#include <iostream>
#include <iomanip>
#include "log/log.h"

namespace dex::print
{
    void StringPrint::print()
    {
        // 获取上下文
        const DexContext& context = getContext();
        
        // 检查是否有效
        if (!context.isValid())
        {
            LOGE("DEX解析未完成或无效，无法打印字符串表");
            return;
        }
        
        // 获取字符串数量
        const uint32_t stringCount = context.getStringIdsCount();
        if (stringCount == 0)
        {
            LOGW("字符串表为空\n");
            return;
        }
        
        printf("\n/----------------------------------------------------------\\\n");
        printf("|                    DEX String Table                    |\n");
        printf("+------+----------------+-------------------------------+\n");
        printf("| %-4s | %-14s | %-29s |\n", "索引", "偏移量", "字符串内容");
        printf("+------+----------------+-------------------------------+\n");
        
        // 获取字符串ID表
        const std::vector<DexStringId>& stringIds = context.getStringIds();
        
        // 打印字符串表
        for (uint32_t i = 0; i < stringCount; i++)
        {
            // 获取字符串内容
            std::string content = context.getString(i);
            
            // 处理过长的字符串
            if (content.length() > 30)
            {
                content = content.substr(0, 27) + "...";
            }
            
            // 处理控制字符
            for (char& c : content)
            {
                if (c < 32 || c > 126)
                {
                    c = '.';
                }
            }
            
            // 打印行
            printf("| %4u | 0x%12X | %-29s |\n", 
                i, stringIds[i].stringDataOff, content.c_str());
            
            // 每25行打印一次表头
            if ((i + 1) % 25 == 0 && i + 1 < stringCount)
            {
                printf("+------+----------------+-------------------------------+\n");
                printf("| %-4s | %-14s | %-29s |\n", "索引", "偏移量", "字符串内容");
                printf("+------+----------------+-------------------------------+\n");
            }
        }
        
        printf("+------+----------------+-------------------------------+\n");
        printf("| 共计: %-42u |\n", stringCount);
        printf("\\----------------------------------------------------------/\n");
    }
} 