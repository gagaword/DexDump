//
// Created by GaGa on 25-5-13.
//

#include "MethodParser.h"
#include "core/DexContext.h"
#include "log/log.h"

namespace dex::parser
{
    MethodParser::MethodParser(const uint8_t* fileData, size_t fileSize)
        : BaseParser(fileData, fileSize, &DexContext::getInstance().getHeader()),
          methodIds_(nullptr),
          methodIdsSize_(0)
    {
        if (fileData == nullptr || fileSize == 0)
        {
            LOGE("文件数据为空");
            return;
        }
    }

    bool MethodParser::parse()
    {
        // 检查文件数据是否有效
        if (!BaseFileData_)
        {
            setError("文件数据为空，无法解析Method表");
            return false;
        }

        // 从上下文获取头部信息
        const DexHeader& header = *BaseHeader_;
        
        // 检查Method ID表是否存在
        if (header.methodIdsSize == 0)
        {
            LOGI("Method ID表为空");
            return true; // 空表也是有效的
        }

        // 检查Method ID表偏移量是否有效
        if (header.methodIdsOff + header.methodIdsSize * sizeof(DexMethodId) > BaseFileSize_)
        {
            setError("Method ID表偏移量或大小无效");
            return false;
        }

        // 获取Method ID表指针
        methodIdsSize_ = header.methodIdsSize;
        methodIds_ = reinterpret_cast<const DexMethodId*>(BaseFileData_ + header.methodIdsOff);
        
        // 验证Method ID表数据
        for (uint32_t i = 0; i < methodIdsSize_; i++)
        {
            // 验证classIdx
            if (methodIds_[i].classIdx >= header.typeIdsSize)
            {
                setError("Method ID %u 的 classIdx 无效: %u", i, methodIds_[i].classIdx);
                return false;
            }
            
            // 验证protoIdx
            if (methodIds_[i].protoIdx >= header.protoIdsSize)
            {
                setError("Method ID %u 的 protoIdx 无效: %u", i, methodIds_[i].protoIdx);
                return false;
            }
            
            // 验证nameIdx
            if (methodIds_[i].nameIdx >= header.stringIdsSize)
            {
                setError("Method ID %u 的 nameIdx 无效: %u", i, methodIds_[i].nameIdx);
                return false;
            }
        }
        
        // 将Method ID表存入全局上下文
        DexContext::getInstance().setMethodIds(methodIds_, methodIdsSize_);
        
        LOGI("成功解析Method ID表，共 %u 个条目", methodIdsSize_);
        return true;
    }
} 