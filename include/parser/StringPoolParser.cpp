//
// Created by GaGa on 25-5-8.
//

#include "StringPoolParser.h"

#include "core/DexContext.h"
#include "log/log.h"

namespace dex::parser
{
    StringPoolParser::StringPoolParser(const uint8_t* fileData, size_t fileSize)
        : BaseParser(fileData, fileSize, &DexContext::getInstance().getHeader()),
          stringIds_(nullptr),
          stringIdsSize_(0)
    {
        // 构造函数
        if (fileData == nullptr || fileSize == 0)
        {
            LOGE("文件数据为空");
            return;
        }
    }

    bool StringPoolParser::parse()
    {
        // 检查文件数据是否有效
        if (!BaseFileData_)
        {
            setError("文件数据为空，无法解析字符串池");
            return false;
        }

        // 从上下文获取头部信息
        const DexHeader& header = *BaseHeader_;
        
        // 检查字符串ID表是否存在
        if (header.stringIdsSize == 0)
        {
            LOGI("字符串ID表为空");
            return true; // 空表也是有效的
        }

        // 检查字符串ID表偏移量是否有效
        if (header.stringIdsOff + header.stringIdsSize * sizeof(DexStringId) > BaseFileSize_)
        {
            setError("字符串ID表偏移量或大小无效");
            return false;
        }

        // 获取字符串ID表指针
        stringIdsSize_ = header.stringIdsSize;
        stringIds_ = reinterpret_cast<const DexStringId*>(BaseFileData_ + header.stringIdsOff);
        
        // 验证字符串数据偏移量
        for (uint32_t i = 0; i < stringIdsSize_; i++)
        {
            const uint32_t offset = stringIds_[i].stringDataOff;
            if (offset >= BaseFileSize_)
            {
                setError("字符串数据偏移量无效: %u", offset);
                return false;
            }
            
            // 简单检查字符串格式
            const uint8_t* data = BaseFileData_ + offset;
            uint8_t byte;
            uint32_t len = 0;
            uint32_t shift = 0;
            
            // 解析ULEB128长度
            do {
                if (offset + (data - (BaseFileData_ + offset)) >= BaseFileSize_)
                {
                    setError("字符串长度解析超出文件范围");
                    return false;
                }
                
                byte = *data++;
                len |= (byte & 0x7F) << shift;
                shift += 7;
            } while (byte & 0x80);
            
            // 检查字符串内容不会超出文件范围
            if (offset + (data - (BaseFileData_ + offset)) + len > BaseFileSize_)
            {
                setError("字符串内容超出文件范围");
                return false;
            }
        }


        // 将字符串ID表存入全局上下文
        DexContext::getInstance().setStringIds(stringIds_, stringIdsSize_);
        
        LOGI("成功解析字符串ID表，共 %u 个条目", stringIdsSize_);
        return true;
    }
}
