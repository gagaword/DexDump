//
// Created by GaGa on 25-5-14.
//

#include "ClassDefsParser.h"
#include "core/DexContext.h"
#include "log/log.h"

namespace dex::parser
{
    ClassDefsParser::ClassDefsParser(const uint8_t* fileData, size_t fileSize)
        : BaseParser(fileData, fileSize, &DexContext::getInstance().getHeader()),
          classDefs_(nullptr),
          classDefsSize_(0)
    {
        if (fileData == nullptr || fileSize == 0)
        {
            LOGE("文件数据为空");
            return;
        }
    }

    bool ClassDefsParser::parse()
    {
        // 检查文件数据是否有效
        if (!BaseFileData_)
        {
            setError("文件数据为空，无法解析ClassDef表");
            return false;
        }

        // 从上下文获取头部信息
        const DexHeader& header = *BaseHeader_;
        
        // 检查ClassDef表是否存在
        if (header.classDefsSize == 0)
        {
            LOGI("ClassDef表为空");
            return true; // 空表也是有效的
        }

        // 检查ClassDef表偏移量是否有效
        if (header.classDefsOff + header.classDefsSize * sizeof(DexClassDef) > BaseFileSize_)
        {
            setError("ClassDef表偏移量或大小无效");
            return false;
        }

        // 获取ClassDef表指针
        classDefsSize_ = header.classDefsSize;
        classDefs_ = reinterpret_cast<const DexClassDef*>(BaseFileData_ + header.classDefsOff);
        
        // 验证ClassDef表数据
        for (uint32_t i = 0; i < classDefsSize_; i++)
        {
            // 验证classIdx
            if (classDefs_[i].classIdx >= header.typeIdsSize)
            {
                setError("ClassDef %u 的 classIdx 无效: %u", i, classDefs_[i].classIdx);
                return false;
            }
            
            // 验证superclassIdx（NO_INDEX==0xFFFFFFFF表示没有父类，如java.lang.Object）
            if (classDefs_[i].superclassIdx != 0xFFFFFFFF && 
                classDefs_[i].superclassIdx >= header.typeIdsSize)
            {
                setError("ClassDef %u 的 superclassIdx 无效: %u", i, classDefs_[i].superclassIdx);
                return false;
            }
            
            // 验证sourceFileIdx（NO_INDEX==0xFFFFFFFF表示没有源文件信息）
            if (classDefs_[i].sourceFileIdx != 0xFFFFFFFF && 
                classDefs_[i].sourceFileIdx >= header.stringIdsSize)
            {
                setError("ClassDef %u 的 sourceFileIdx 无效: %u", i, classDefs_[i].sourceFileIdx);
                return false;
            }
            
            // 验证interfacesOff
            if (classDefs_[i].interfacesOff != 0 && 
                classDefs_[i].interfacesOff >= BaseFileSize_)
            {
                setError("ClassDef %u 的 interfacesOff 无效: %u", i, classDefs_[i].interfacesOff);
                return false;
            }
            
            // 验证annotationsOff
            if (classDefs_[i].annotationsOff != 0 && 
                classDefs_[i].annotationsOff >= BaseFileSize_)
            {
                setError("ClassDef %u 的 annotationsOff 无效: %u", i, classDefs_[i].annotationsOff);
                return false;
            }
            
            // 验证classDataOff
            if (classDefs_[i].classDataOff != 0 && 
                classDefs_[i].classDataOff >= BaseFileSize_)
            {
                setError("ClassDef %u 的 classDataOff 无效: %u", i, classDefs_[i].classDataOff);
                return false;
            }
            
            // 验证staticValuesOff
            if (classDefs_[i].staticValuesOff != 0 && 
                classDefs_[i].staticValuesOff >= BaseFileSize_)
            {
                setError("ClassDef %u 的 staticValuesOff 无效: %u", i, classDefs_[i].staticValuesOff);
                return false;
            }
        }
        
        // 将ClassDef表存入全局上下文
        DexContext::getInstance().setClassDefs(classDefs_, classDefsSize_);
        
        LOGI("成功解析ClassDef表，共 %u 个条目", classDefsSize_);
        return true;
    }
} 