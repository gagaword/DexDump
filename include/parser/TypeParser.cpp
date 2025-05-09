//
// Created by GaGa on 25-5-9.
//

#include "TypeParser.h"

#include "core/DexContext.h"
#include "log/log.h"

namespace dex::parser
{
    TypeParser::TypeParser(const uint8_t* fileData, size_t fileSize): BaseParser(
                                                                          fileData, fileSize,
                                                                          &DexContext::getInstance().getHeader()),
                                                                      typeIds_(nullptr),
                                                                      typeIdsSize_(0)
    {
        if (fileData == nullptr || fileSize == 0)
        {
            LOGE("文件数据为空");
            return;
        }
    }

    bool TypeParser::parse()
    {
        typeIdsSize_ = BaseHeader_->typeIdsSize;
        typeIds_ = reinterpret_cast<const DexTypeId*>(BaseFileData_+BaseHeader_->typeIdsOff);
        if (typeIds_ == nullptr)
        {
            setError("类型ID表为空");
            return false;
        }
        if (BaseHeader_->typeIdsOff + typeIdsSize_ * sizeof(DexTypeId) > BaseFileSize_)
        {
            setError("TypeID表偏移量或大小无效");
            return false;
        }

        for (int i = 0; i < typeIdsSize_; ++i)
        {
            const uint32_t stringIdx = typeIds_[i].descriptor_idx;
            if (stringIdx >= BaseFileSize_)
            {
                setError("类型ID表字符串索引无效");
                return false;
            }
        }

        DexContext::getInstance().setTypeIds(typeIds_, typeIdsSize_);
        LOGI("类型ID表解析成功，大小: %u", typeIdsSize_);
        return true;
    }
}
