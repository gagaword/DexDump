//
// Created by GaGa on 25-5-9.
//

#include "HeaderPrint.h"
#include <cstdio>
#include <cstring>
#include "log/log.h"

namespace dex::print
{
    void HeaderPrint::print()
    {
        // 获取头部结构
        const DexHeader& header = getContext().getHeader();
        
        if (!getContext().isValid())
        {
            LOGE("DEX解析未完成或无效，无法打印头部信息");
            return;
        }
        
        printf("/-----------------------------------------------\\\n");
        printf("|              DEX Header Info                |\n");
        printf("+-----------------------+-----------------------+\n");

        // Magic & Version
        char dex[4] = {0};
        char version[4] = {0};
        memcpy(dex, &header.magic, 3);
        memcpy(version, &header.magic[4], 3);
        printf("| %-22s | %-15s |\n", "Magic (dex):", dex);
        printf("| %-22s | %-15s |\n", "Magic (version):", version);
        printf("+-----------------------+-----------------------+\n");

        // Checksum & Signature
        printf("| %-22s | 0x%-13X |\n", "Checksum:", header.checksum);
        
        // 输出SHA1签名的前8位
        printf("| %-22s | ", "Signature:");
        for (int i = 0; i < kSHA1DigestLen; i++)
        {
            printf("%02X", header.signature[i]);
        }
        printf("... |\n");
        printf("+-----------------------+-----------------------+\n");
        
        // 文件大小和头部大小
        printf("| %-22s | %-15u |\n", "File Size:", header.fileSize);
        printf("| %-22s | %-15u |\n", "Header Size:", header.headerSize);
        printf("+-----------------------+-----------------------+\n");
        
        // 字节序和链接段信息
        printf("| %-22s | 0x%-13X |\n", "Endian Tag:", header.endianTag);
        printf("| %-22s | %-15u |\n", "Link Size:", header.linkSize);
        printf("| %-22s | 0x%-13X |\n", "Link Offset:", header.linkOff);
        printf("+-----------------------+-----------------------+\n");
        
        // Map段信息
        printf("| %-22s | 0x%-13X |\n", "Map Offset:", header.mapOff);
        printf("+-----------------------+-----------------------+\n");
        
        // 字符串ID表信息
        printf("| %-22s | %-15u |\n", "String IDs Size:", header.stringIdsSize);
        printf("| %-22s | 0x%-13X |\n", "String IDs Offset:", header.stringIdsOff);
        printf("+-----------------------+-----------------------+\n");
        
        // 类型ID表信息
        printf("| %-22s | %-15u |\n", "Type IDs Size:", header.typeIdsSize);
        printf("| %-22s | 0x%-13X |\n", "Type IDs Offset:", header.typeIdsOff);
        printf("+-----------------------+-----------------------+\n");
        
        // 原型ID表信息
        printf("| %-22s | %-15u |\n", "Proto IDs Size:", header.protoIdsSize);
        printf("| %-22s | 0x%-13X |\n", "Proto IDs Offset:", header.protoIdsOff);
        printf("+-----------------------+-----------------------+\n");
        
        // 字段ID表信息
        printf("| %-22s | %-15u |\n", "Field IDs Size:", header.fieldIdsSize);
        printf("| %-22s | 0x%-13X |\n", "Field IDs Offset:", header.fieldIdsOff);
        printf("+-----------------------+-----------------------+\n");
        
        // 方法ID表信息
        printf("| %-22s | %-15u |\n", "Method IDs Size:", header.methodIdsSize);
        printf("| %-22s | 0x%-13X |\n", "Method IDs Offset:", header.methodIdsOff);
        printf("+-----------------------+-----------------------+\n");
        
        // 类定义表信息
        printf("| %-22s | %-15u |\n", "Class Defs Size:", header.classDefsSize);
        printf("| %-22s | 0x%-13X |\n", "Class Defs Offset:", header.classDefsOff);
        printf("+-----------------------+-----------------------+\n");
        
        // 数据段信息
        printf("| %-22s | %-15u |\n", "Data Size:", header.dataSize);
        printf("| %-22s | 0x%-13X |\n", "Data Offset:", header.dataOff);
        printf("\\-----------------------------------------------/\n");
    }
}

