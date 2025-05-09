//
// Created by GaGa on 25-5-8.
//

#ifndef UTIL_H
#define UTIL_H

#include <cstdint>
#include <cstddef>
#include <unordered_map>

namespace util {

    /**
     * 文件映射相关信息结构体
     */
    struct FileMapping {
#ifdef _WIN32
        void* fileHandle;      // 文件句柄
        void* mappingHandle;   // 映射对象句柄
#else
        size_t size;           // 映射大小
#endif
        uint8_t* data;         // 映射数据指针
    };

    // 存储所有活动的文件映射，键为数据指针
    static std::unordered_map<uint8_t*, FileMapping> fileMappings;

    /**
     * 处理文件的主函数
     * @param fileName 文件路径
     * @return 0表示成功，负值表示错误码
     */
    int32_t processFile(const char* fileName);

    /**
     * 使用内存映射方式打开文件
     * @param fileName 文件路径
     * @param fileData 输出参数，指向文件数据的指针
     * @param fileSize 输出参数，文件大小
     * @return 0表示成功，负值表示错误码
     */
    int32_t mapFile(const char* fileName, uint8_t** fileData, size_t* fileSize);

    /**
     * 释放内存映射文件
     * @param fileData 文件数据指针
     * @return 0表示成功，负值表示错误码
     */
    int32_t unmapFile(uint8_t* fileData);
}

#endif // UTIL_H