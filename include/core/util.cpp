//
// Created by GaGa on 25-5-8.
//

#include "util.h"

#include <cstdio>
#include <cstdint>

#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#include "log/log.h"

/**
 * 使用内存映射方式处理文件
 * @param fileName 文件路径
 * @param fileData 输出参数，指向文件数据的指针
 * @param fileSize 输出参数，文件大小
 * @return 0表示成功，负值表示错误码
 */
int32_t util::mapFile(const char* fileName, uint8_t** fileData, size_t* fileSize)
{
    if (fileName == nullptr)
    {
        // LOGE("文件名不能为空");
        return -1;
    }

    // Windows实现
    HANDLE hFile = CreateFileA(
        fileName,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE)
    {
        DWORD error = GetLastError();
        char errorMsg[1024];
        FormatMessageA(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            error,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            errorMsg,
            sizeof(errorMsg),
            nullptr
        );
        LOGE("打开文件失败: %s (错误码: %lu)", errorMsg, error);
        return -2;
    }

    DWORD dwFileSize = GetFileSize(hFile, nullptr);
    if (dwFileSize == INVALID_FILE_SIZE)
    {
        DWORD error = GetLastError();
        char errorMsg[1024];
        FormatMessageA(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            error,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            errorMsg,
            sizeof(errorMsg),
            nullptr
        );
        LOGE("获取文件大小失败: %s (错误码: %lu)", errorMsg, error);
        CloseHandle(hFile);
        return -3;
    }

    // 创建文件映射对象
    HANDLE hFileMapping = CreateFileMappingA(
        hFile,
        nullptr,
        PAGE_READONLY,
        0,
        0,
        nullptr
    );

    if (hFileMapping == nullptr)
    {
        DWORD error = GetLastError();
        char errorMsg[1024];
        FormatMessageA(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            error,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            errorMsg,
            sizeof(errorMsg),
            nullptr
        );
        LOGE("创建文件映射失败: %s (错误码: %lu)", errorMsg, error);
        CloseHandle(hFile);
        return -4;
    }

    // 映射文件到内存
    LPVOID lpFileData = MapViewOfFile(
        hFileMapping,
        FILE_MAP_READ,
        0,
        0,
        0
    );

    if (lpFileData == nullptr)
    {
        DWORD error = GetLastError();
        char errorMsg[1024];
        FormatMessageA(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            error,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            errorMsg,
            sizeof(errorMsg),
            nullptr
        );
        LOGE("映射文件视图失败: %s (错误码: %lu)", errorMsg, error);
        CloseHandle(hFileMapping);
        CloseHandle(hFile);
        return -5;
    }

    // 设置输出参数
    *fileData = static_cast<uint8_t*>(lpFileData);
    *fileSize = static_cast<size_t>(dwFileSize);

    // 注意：在使用完毕后需要调用unmapFile来释放资源
    // 我们在这里不关闭句柄，因为它们需要在映射期间保持有效
    LOGI("成功映射文件: %s, Size: %d 字节", fileName, *fileSize);

    // 存储句柄以便稍后关闭
    util::FileMapping mapping{};
    mapping.fileHandle = hFile;
    mapping.mappingHandle = hFileMapping;
    mapping.data = *fileData;
    util::fileMappings[*fileData] = mapping;

    return 0;
}

/**
 * 释放内存映射文件
 * @param fileData 文件数据指针
 * @return 0表示成功，负值表示错误码
 */
int32_t util::unmapFile(uint8_t* fileData)
{
    if (fileData == nullptr)
    {
        LOGE("文件数据指针为空");
        return -1;
    }

    // 查找映射信息
    auto it = util::fileMappings.find(fileData);
    if (it == util::fileMappings.end())
    {
        LOGE("找不到指定的文件映射信息");
        return -2;
    }


    // Windows实现
    if (!UnmapViewOfFile(fileData))
    {
        DWORD error = GetLastError();
        char errorMsg[1024];
        FormatMessageA(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            error,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            errorMsg,
            sizeof(errorMsg),
            nullptr
        );
        LOGE("解除文件映射失败: %s (错误码: %lu)", errorMsg, error);
        return -3;
    }

    // 关闭映射句柄
    if (!CloseHandle(it->second.mappingHandle))
    {
        DWORD error = GetLastError();
        char errorMsg[1024];
        FormatMessageA(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            error,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            errorMsg,
            sizeof(errorMsg),
            nullptr
        );
        LOGE("关闭映射句柄失败: %s (错误码: %lu)", errorMsg, error);
        return -4;
    }

    // 关闭文件句柄
    if (!CloseHandle(it->second.fileHandle))
    {
        DWORD error = GetLastError();
        char errorMsg[1024];
        FormatMessageA(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            error,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            errorMsg,
            sizeof(errorMsg),
            nullptr
        );
        LOGE("关闭文件句柄失败: %s (错误码: %lu)", errorMsg, error);
        return -5;
    }
    // 从映射表中移除
    util::fileMappings.erase(it);
    LOGI("成功解除文件映射");

    return 0;
}

int32_t util::processFile(const char* fileName)
{
    if (fileName == nullptr)
    {
        // LOGE("文件名不能为空");
        return -1;
    }

    uint8_t* fileData = nullptr;
    size_t fileSize = 0;

    // 使用内存映射打开文件
    int32_t result = mapFile(fileName, &fileData, &fileSize);
    if (result != 0)
    {
        LOGE("映射文件失败: %d", result);
        return result;
    }

    // 这里可以处理映射后的文件数据
    // 例如：解析DEX头部信息
    LOGI("文件大小: %zu 字节", fileSize);


    /*// 处理完毕后解除映射
    result = unmapFile(fileData);
    if (result != 0)
    {
        LOGE("解除文件映射失败: %d", result);
        return result;
    }*/

    return 0;
}
