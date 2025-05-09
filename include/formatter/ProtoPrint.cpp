//
// Created by GaGa on 25-5-11.
//

#include "ProtoPrint.h"
#include <cstdio>
#include <cstring>
#include "log/log.h"

namespace dex::print
{
    void ProtoPrint::print()
    {
        // 获取上下文
        const dex::DexContext& context = getContext();

        // 检查是否有效
        if (!context.isValid())
        {
            LOGE("DEX解析未完成或无效，无法打印Proto表");
            return;
        }

        // 获取Proto数量
        const uint32_t protoCount = context.getProtoIdsCount();
        if (protoCount == 0)
        {
            printf("Proto表为空\n");
            return;
        }

        printf("\n/----------------------------------------------------------\\\n");
        printf("|                     DEX Proto Table                     |\n");
        printf("+------+----------------+----------------+----------------+\n");
        printf("| %-4s | %-14s | %-14s | %-14s |\n", "索引", "Shorty", "返回类型", "参数");
        printf("+------+----------------+----------------+----------------+\n");

        // 获取Proto表
        const std::vector<DexProtoId>& protoIds = context.getProtoIds();

        // 打印Proto表
        for (uint32_t i = 0; i < protoCount; i++)
        {
            // 获取Proto信息
            std::string shorty = context.getProtoShorty(i);
            std::string returnType = context.getProtoReturnType(i);
            const TypeListData* parameters = context.getProtoParameters(i);

            // 处理过长的字符串
            if (shorty.length() > 15)
            {
                shorty = shorty.substr(0, 12) + "...";
            }
            if (returnType.length() > 15)
            {
                returnType = returnType.substr(0, 12) + "...";
            }

            // 格式化参数列表
            std::string paramStr = "(";
            if (parameters != nullptr && parameters->size > 0)
            {
                for (uint32_t j = 0; j < parameters->size && j < 3; j++)
                {
                    if (j > 0) paramStr += ",";
                    std::string type = context.getType(parameters->items[j].typeIdx);

                    // 简化类型名称，仅保留最后一部分
                    size_t pos = type.find_last_of('/');
                    if (pos != std::string::npos && pos < type.length() - 1)
                    {
                        type = type.substr(pos + 1);
                    }

                    if (type.length() > 8)
                    {
                        type = type.substr(0, 5) + "...";
                    }

                    paramStr += type;
                }

                if (parameters->size > 3)
                {
                    paramStr += ",...";
                }
            }
            paramStr += ")";

            if (paramStr.length() > 15)
            {
                paramStr = paramStr.substr(0, 12) + "...";
            }

            // 打印行
            printf("| %4u | %-14s | %-14s | %-14s |\n",
                   i, shorty.c_str(), returnType.c_str(), paramStr.c_str());

            // 每20行打印一次表头
            if ((i + 1) % 20 == 0 && i + 1 < protoCount)
            {
                printf("+------+----------------+----------------+----------------+\n");
                printf("| %-4s | %-14s | %-14s | %-14s |\n", "索引", "Shorty", "返回类型", "参数");
                printf("+------+----------------+----------------+----------------+\n");
            }
        }

        printf("+------+----------------+----------------+----------------+\n");
        printf("| 共计: %-42u |\n", protoCount);
        printf("\\----------------------------------------------------------/\n");
    }
}
