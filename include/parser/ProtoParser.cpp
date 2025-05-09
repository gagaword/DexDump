//
// Created by GaGa on 25-5-11.
//

#include "ProtoParser.h"

#include "core/DexContext.h"
#include "log/log.h"


namespace dex::parser
{
    ProtoParser::ProtoParser(const uint8_t* fileData, size_t fileSize)
        : BaseParser(fileData, fileSize, &DexContext::getInstance().getHeader()),
          protoIds_(nullptr),
          protoIdsSize_(0)
    {
        if (fileData == nullptr || fileSize == 0)
        {
            LOGE("文件数据为空");
            return;
        }
    }

    bool ProtoParser::parse()
    {
        // 检查文件数据是否有效
        if (!BaseFileData_)
        {
            setError("文件数据为空，无法解析Proto表");
            return false;
        }

        // 从上下文获取头部信息
        const DexHeader& header = *BaseHeader_;
        
        // 检查Proto ID表是否存在
        if (header.protoIdsSize == 0)
        {
            LOGI("Proto ID表为空");
            return true; // 空表也是有效的
        }

        // 检查Proto ID表偏移量是否有效
        if (header.protoIdsOff + header.protoIdsSize * sizeof(DexProtoId) > BaseFileSize_)
        {
            setError("Proto ID表偏移量或大小无效");
            return false;
        }

        // 获取Proto ID表指针
        protoIdsSize_ = header.protoIdsSize;
        protoIds_ = reinterpret_cast<const DexProtoId*>(BaseFileData_ + header.protoIdsOff);
        
        // 验证Proto ID表数据
        for (uint32_t i = 0; i < protoIdsSize_; i++)
        {
            // 验证shorty_idx
            if (protoIds_[i].shorty_idx >= header.stringIdsSize)
            {
                setError("Proto ID %u 的 shorty_idx 无效: %u", i, protoIds_[i].shorty_idx);
                return false;
            }
            
            // 验证return_type_idx
            if (protoIds_[i].return_type_idx >= header.typeIdsSize)
            {
                setError("Proto ID %u 的 return_type_idx 无效: %u", i, protoIds_[i].return_type_idx);
                return false;
            }
            
            // 验证parameters_off（如果有）
            if (protoIds_[i].parameters_off != 0)
            {
                if (protoIds_[i].parameters_off >= BaseFileSize_)
                {
                    setError("Proto ID %u 的 parameters_off 无效: %u", i, protoIds_[i].parameters_off);
                    return false;
                }
                
                // 验证TypeList结构
                const DexTypeList* typeList = reinterpret_cast<const DexTypeList*>(
                    BaseFileData_ + protoIds_[i].parameters_off);
                
                // 检查TypeList大小是否有效
                if (protoIds_[i].parameters_off + sizeof(typeList->size) +
                    typeList->size * sizeof(DexTypeItem) > BaseFileSize_)
                {
                    setError("Proto ID %u 的 TypeList 大小无效: %u", i, typeList->size);
                    return false;
                }
                
                // 验证每个TypeItem
                for (uint32_t j = 0; j < typeList->size; j++)
                {
                    if (typeList->list[j].typeIdx >= header.typeIdsSize)
                    {
                        setError("Proto ID %u 的 TypeList 项 %u 的 typeIdx 无效: %u",
                                i, j, typeList->list[j].typeIdx);
                        return false;
                    }
                }
            }
        }
        
        // 将Proto ID表存入全局上下文
        DexContext::getInstance().setProtoIds(protoIds_, protoIdsSize_);
        
        LOGI("成功解析Proto ID表，共 %u 个条目", protoIdsSize_);
        return true;
    }
}
