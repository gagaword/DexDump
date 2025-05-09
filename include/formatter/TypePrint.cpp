//
// Created by GaGa on 25-5-9.
//

#include "TypePrint.h"

#include "log/log.h"

namespace dex::print
{
    void TypePrint::print()
    {
        // 通过上下文获取TypeID表
        const DexContext& context = getContext();

        if (!context.isValid())
        {
            LOGE("DEX解析未完成或无效，无法打印类型表");
            return;
        }

        //  获取类型数量
        const uint32_t typeCount = context.getTypeIdsCount();
        if (typeCount == 0)
        {
            LOGW("类型表为空\n");
            return;
        }
        printf("\n/----------------------------------------------------------\\\n");
        printf("|                       DEX Type Table                              |\n");
        printf("+------+----------------+-------------------------------+\n");
        printf("| %-4s | %-14s | %-29s |\n", "索引", "偏移量", "字符串内容");
        printf("+------+----------------+-------------------------------+\n");


        // 获取TypeIds表
        const std::vector<DexTypeId>& typeIds = context.getTypeIds();
        // 打印类型表
        for (int i = 0; i < typeCount; ++i)
        {
            std::string type = context.getType(i);
            // 处理过长的字符串
            if (type.length() > 30)
            {
                type = type.substr(0, 27) + "...";
            }

            // 处理控制字符
            for (char& c : type)
            {
                if (c < 32 || c > 126)
                {
                    c = '.';
                }
            }

            // 打印行
            printf("| %4u | 0x%12X | %-29s |\n",
                   i, typeIds[i].descriptor_idx, type.c_str());

            // 每25行打印一次表头
            if ((i + 1) % 25 == 0 && i + 1 < typeCount)
            {
                printf("+------+----------------+-------------------------------+\n");
                printf("| %-4s | %-14s | %-29s |\n", "索引", "偏移量", "字符串内容");
                printf("+------+----------------+-------------------------------+\n");
            }
        }
    }
}
