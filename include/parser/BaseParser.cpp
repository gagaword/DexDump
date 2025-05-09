//
// Created by GaGa on 25-5-8.
//

#include "BaseParser.h"

#include "log/log.h"

namespace dex::parser
{
    BaseParser::BaseParser(const uint8_t* fileData, size_t fileSize, const DexHeader* header)
        : BaseFileData_(fileData), BaseFileSize_(fileSize), BaseHeader_(header), lastError_()
    {
    }

    BaseParser::BaseParser(): BaseFileData_(nullptr), BaseFileSize_(0), BaseHeader_(nullptr)
    {
    }

    const std::string& BaseParser::getLastError() const
    {
        return lastError_;
    }

    bool BaseParser::isValidOffset(const uint32_t offset, const uint32_t size) const
    {
        // 检查偏移量是否在文件范围内
        if (offset >= BaseFileSize_)
        {
            return false;
        }

        // 如果提供了大小，则检查偏移量+大小是否超出文件范围
        if (size > 0 && offset + size > BaseFileSize_)
        {
            return false;
        }

        return true;
    }

    void BaseParser::setError(const std::string& error) const
    {
        lastError_ = error;
        LOGE("%s", error.c_str());
    }

    void BaseParser::setError(const char* fmt, ...) const
    {
        char buffer[1024]; // 足够大的缓冲区
        va_list args;
        va_start(args, fmt);
        vsnprintf(buffer, sizeof(buffer), fmt, args);
        va_end(args);

        lastError_ = buffer;
        LOGE("%s", buffer);
    }
}
