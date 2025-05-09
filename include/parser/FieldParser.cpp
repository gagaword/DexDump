//
// Created by GaGa on 25-5-12.
//

#include "FieldParser.h"
#include "core/DexContext.h"
#include "log/log.h"

namespace dex::parser
{
    FieldParser::FieldParser(const uint8_t* fileData, size_t fileSize)
        : BaseParser(fileData, fileSize, &DexContext::getInstance().getHeader()),
          fieldIds_(nullptr),
          fieldIdsSize_(0)
    {
        if (fileData == nullptr || fileSize == 0)
        {
            LOGE("文件数据为空");
            return;
        }
    }

    bool FieldParser::parse()
    {
        // 检查文件数据是否有效
        if (!BaseFileData_)
        {
            setError("文件数据为空，无法解析Field表");
            return false;
        }

        // 从上下文获取头部信息
        const DexHeader& header = *BaseHeader_;
        
        // 检查Field ID表是否存在
        if (header.fieldIdsSize == 0)
        {
            LOGI("Field ID表为空");
            return true; // 空表也是有效的
        }

        // 检查Field ID表偏移量是否有效
        if (header.fieldIdsOff + header.fieldIdsSize * sizeof(DexFieldId) > BaseFileSize_)
        {
            setError("Field ID表偏移量或大小无效");
            return false;
        }

        // 获取Field ID表指针
        fieldIdsSize_ = header.fieldIdsSize;
        fieldIds_ = reinterpret_cast<const DexFieldId*>(BaseFileData_ + header.fieldIdsOff);
        
        // 验证Field ID表数据
        for (uint32_t i = 0; i < fieldIdsSize_; i++)
        {
            // 验证classIdx
            if (fieldIds_[i].classIdx >= header.typeIdsSize)
            {
                setError("Field ID %u 的 classIdx 无效: %u", i, fieldIds_[i].classIdx);
                return false;
            }
            
            // 验证typeIdx
            if (fieldIds_[i].typeIdx >= header.typeIdsSize)
            {
                setError("Field ID %u 的 typeIdx 无效: %u", i, fieldIds_[i].typeIdx);
                return false;
            }
            
            // 验证nameIdx
            if (fieldIds_[i].nameIdx >= header.stringIdsSize)
            {
                setError("Field ID %u 的 nameIdx 无效: %u", i, fieldIds_[i].nameIdx);
                return false;
            }
        }
        
        // 将Field ID表存入全局上下文
        DexContext::getInstance().setFieldIds(fieldIds_, fieldIdsSize_);
        
        LOGI("成功解析Field ID表，共 %u 个条目", fieldIdsSize_);
        return true;
    }
} 