//
// Created by DexDump on 2025-5-10.
//

#include "DexContext.h"
#include <cstring>
#include "log/log.h"

namespace dex
{
    DexContext& DexContext::getInstance()
    {
        static DexContext instance;
        return instance;
    }

    DexContext::DexContext() : fileData_(nullptr), fileSize_(0), stringsLoaded_(false), typeSLoad_(false),
                               protoLoad_(false), fieldsLoaded_(false), methodsLoaded_(false),
                               classDefsLoaded_(false), isValid_(false)
    {
        // 清空头部结构和DexFile结构
        memset(&header_, 0, sizeof(DexHeader));
        memset(&dexFile_, 0, sizeof(DexFile));
    }

    void DexContext::setFileData(const uint8_t* fileData, size_t fileSize)
    {
        fileData_ = fileData;
        fileSize_ = fileSize;
    }

    const uint8_t* DexContext::getFileData() const
    {
        return fileData_;
    }

    size_t DexContext::getFileSize() const
    {
        return fileSize_;
    }

    // Header头
    void DexContext::setHeader(const DexHeader& header)
    {
        // 复制头部结构
        memcpy(&header_, &header, sizeof(DexHeader));

        // 更新全局DexFile中的头部指针
        dexFile_.pHeader = &header_;
    }

    const DexHeader& DexContext::getHeader() const
    {
        return header_;
    }


    // String字符串
    void DexContext::setStringIds(const DexStringId* stringIds, uint32_t count)
    {
        // 清空现有字符串ID表
        stringIds_.clear();
        stringCache_.clear();
        stringsLoaded_ = false;

        // 复制字符串ID表
        if (stringIds != nullptr && count > 0)
        {
            stringIds_.assign(stringIds, stringIds + count);

            // 初始化字符串缓存，但不加载内容
            stringCache_.resize(count);

            // 更新全局DexFile中的字符串ID表指针
            dexFile_.pStringIds = stringIds_.data();
        }
    }

    const std::vector<DexStringId>& DexContext::getStringIds() const
    {
        return stringIds_;
    }

    uint32_t DexContext::getStringIdsCount() const
    {
        return static_cast<uint32_t>(stringIds_.size());
    }

    std::string DexContext::getString(uint32_t idx) const
    {
        // 检查索引是否有效
        if (idx >= stringIds_.size() || fileData_ == nullptr)
        {
            return "";
        }

        // 如果已经缓存了字符串内容，直接返回
        if (stringsLoaded_ && idx < stringCache_.size() && !stringCache_[idx].empty())
        {
            return stringCache_[idx];
        }

        // 获取字符串数据的偏移量
        const uint32_t offset = stringIds_[idx].stringDataOff;

        // 检查偏移量是否有效
        if (offset >= fileSize_)
        {
            LOGW("字符串偏移量无效: %u", offset);
            return "";
        }

        // 获取字符串数据指针并解码
        const uint8_t* stringData = fileData_ + offset;
        return decodeMUTF8(stringData);
    }

    bool DexContext::loadAllStrings() const
    {
        // 如果已经加载了所有字符串，返回true
        if (stringsLoaded_)
        {
            return true;
        }

        // 确保字符串ID表存在
        if (stringIds_.empty() || fileData_ == nullptr)
        {
            LOGE("字符串ID表不存在或文件数据为空");
            return false;
        }

        // 重新分配缓存大小
        stringCache_.resize(stringIds_.size());

        // 逐个解析字符串内容
        for (uint32_t i = 0; i < stringIds_.size(); i++)
        {
            stringCache_[i] = getString(i);
        }

        // 标记为已加载所有字符串
        stringsLoaded_ = true;

        LOGI("加载了 %zu 个字符串", stringCache_.size());
        return true;
    }


    // Type类型
    void DexContext::setTypeIds(const DexTypeId* typeIds, uint32_t count)
    {
        // 清空现有TypeID表
        typeIds_.clear();
        typeCache_.clear();
        typeSLoad_ = false;
        // 复制TypeID表
        if (typeIds != nullptr && count > 0)
        {
            typeIds_.assign(typeIds, typeIds + count);

            typeCache_.resize(count);

            // 更新全局DexFile中的TypeID表指针
            dexFile_.pTypeIds = typeIds_.data();
        }
    }

    const std::vector<DexTypeId>& DexContext::getTypeIds() const
    {
        return typeIds_;
    }

    uint32_t DexContext::getTypeIdsCount() const
    {
        return static_cast<uint32_t>(typeIds_.size());
    }

    bool DexContext::loadStringType() const
    {
        if (typeSLoad_)
        {
            return true;
        }

        if (typeIds_.empty() || fileData_ == nullptr)
        {
            LOGE("类型ID表不存在或文件数据为空");
            return false;
        }

        typeCache_.resize(typeIds_.size());

        for (int i = 0; i < typeIds_.size(); ++i)
        {
            typeCache_[i] = getString(typeIds_[i].descriptor_idx);
        }
        typeSLoad_ = true;
        LOGI("加载了 %zu 个类型", typeCache_.size());
        return true;
    }

    std::string DexContext::getType(uint32_t idx) const
    {
        if (idx >= typeIds_.size() || fileData_ == nullptr)
        {
            LOGE("索引值无效");
            return nullptr;
        }

        if (typeSLoad_ && idx < typeCache_.size() && typeCache_[idx].empty())
        {
            return typeCache_[idx];
        }
        const uint32_t offset = typeIds_[idx].descriptor_idx;
        if (offset >= fileSize_)
        {
            LOGW("类型偏移量无效: %u", offset);
            return "";
        }
        std::string type = getString(offset);
        return type;
    }

    const TypeListData* DexContext::parseTypeList(uint32_t offset) const
    {
        // 检查偏移量是否有效
        if (offset == 0 || offset >= fileSize_)
        {
            LOGE("TypeList偏移量无效: %u", offset);
            return nullptr;
        }

        // 检查缓存中是否已存在
        auto it = typeListCache_.find(offset);
        if (it != typeListCache_.end())
        {
            return &(it->second);
        }

        // 指向TypeList的指针
        const DexTypeList* rawTypeList = reinterpret_cast<const DexTypeList*>(fileData_ + offset);

        // 检查size字段是否有效
        if (offset + sizeof(uint32_t) + rawTypeList->size * sizeof(DexTypeItem) > fileSize_)
        {
            LOGE("TypeList大小无效: %u", rawTypeList->size);
            return nullptr;
        }

        // 创建新的TypeListData并缓存
        TypeListData& typeListData = typeListCache_[offset];
        typeListData.size = rawTypeList->size;

        // 复制所有TypeItem
        if (rawTypeList->size > 0)
        {
            typeListData.items.resize(rawTypeList->size);
            for (uint32_t i = 0; i < rawTypeList->size; i++)
            {
                typeListData.items[i] = rawTypeList->list[i];
            }
        }

        return &typeListData;
    }

    void DexContext::setProtoIds(const DexProtoId* proto_id, uint32_t count)
    {
        protoIds_.clear();
        protoCacheShort_.clear();
        protoCacheShort_.clear();
        protoCacheParameter_.clear();
        typeListCache_.clear();
        protoLoad_ = false;

        if (proto_id != nullptr && count > 0)
        {
            protoIds_.assign(proto_id, proto_id + count);

            protoCacheShort_.resize(count);
            protoCacheReturn_.resize(count);
            protoCacheParameter_.resize(count);
            dexFile_.pProtoIds = protoIds_.data();
        }

        if (!loadAllProtos())
        {
            LOGE("加载Proto错误");
            return;
        }
    }

    bool DexContext::loadAllProtos() const
    {
        if (protoLoad_)
        {
            return true;
        }

        if (protoIds_.empty() || fileData_ == nullptr)
        {
            LOGE("Proto不存在或数据为空\n");
            return false;
        }
        protoCacheShort_.resize(protoIds_.size());
        protoCacheReturn_.resize(protoIds_.size());
        protoCacheParameter_.resize(protoIds_.size());


        for (int i = 0; i < protoIds_.size(); ++i)
        {
            protoCacheShort_[i] = getString(protoIds_[i].shorty_idx);
            protoCacheReturn_[i] = getType(protoIds_[i].return_type_idx);
            if (protoIds_[i].parameters_off != 0)
            {
                const unsigned char* offset = fileData_ + protoIds_[i].parameters_off;
                const DexTypeList* typeList{};

                parseTypeList(protoIds_[i].parameters_off);
            }
        }
        protoLoad_ = true;
        LOGI("加载了:%zu 个Proto", protoIds_.size());
        return true;
    }

    const std::vector<DexProtoId>& DexContext::getProtoIds() const
    {
        return protoIds_;
    }

    uint32_t DexContext::getProtoIdsCount() const
    {
        return static_cast<uint32_t>(protoIds_.size());
    }

    std::string DexContext::getProtoShorty(uint32_t idx) const
    {
        if (idx >= protoIds_.size())
        {
            LOGE("Proto索引无效");
            return "";
        }

        if (protoLoad_ && idx < protoCacheShort_.size() && !protoCacheShort_[idx].empty())
        {
            return protoCacheShort_[idx];
        }

        return getString(protoIds_[idx].shorty_idx);
    }

    std::string DexContext::getProtoReturnType(uint32_t idx) const
    {
        if (idx >= protoIds_.size())
        {
            LOGE("Proto索引无效");
            return "";
        }

        if (protoLoad_ && idx < protoCacheReturn_.size() && !protoCacheReturn_[idx].empty())
        {
            return protoCacheReturn_[idx];
        }

        return getType(protoIds_[idx].return_type_idx);
    }

    const TypeListData* DexContext::getProtoParameters(uint32_t idx) const
    {
        if (idx >= protoIds_.size())
        {
            LOGE("Proto索引无效");
            return nullptr;
        }

        const uint32_t parameters_off = protoIds_[idx].parameters_off;
        if (parameters_off == 0)
        {
            // 无参数列表
            return nullptr;
        }

        return parseTypeList(parameters_off);
    }

    // Field相关方法
    void DexContext::setFieldIds(const DexFieldId* fieldIds, uint32_t count)
    {
        // 清空现有FieldId表和缓存
        fieldIds_.clear();
        fieldCache_.clear();
        fieldsLoaded_ = false;

        // 复制FieldId表
        if (fieldIds != nullptr && count > 0)
        {
            fieldIds_.assign(fieldIds, fieldIds + count);

            // 初始化字段信息缓存
            fieldCache_.resize(count);

            // 更新全局DexFile中的字段ID表指针
            dexFile_.pFieldIds = fieldIds_.data();
        }
    }

    const std::vector<DexFieldId>& DexContext::getFieldIds() const
    {
        return fieldIds_;
    }

    uint32_t DexContext::getFieldIdsCount() const
    {
        return static_cast<uint32_t>(fieldIds_.size());
    }

    FieldInfo DexContext::getFieldInfo(uint32_t idx) const
    {
        // 准备空的字段信息
        FieldInfo info = {};

        // 检查索引是否有效
        if (idx >= fieldIds_.size() || fileData_ == nullptr)
        {
            LOGE("字段索引无效: %u", idx);
            return info;
        }

        // 如果已加载所有字段信息，直接返回缓存
        if (fieldsLoaded_ && idx < fieldCache_.size())
        {
            return fieldCache_[idx];
        }

        // 获取字段ID
        const DexFieldId& fieldId = fieldIds_[idx];

        // 填充基本信息
        info.classIdx = fieldId.classIdx;
        info.typeIdx = fieldId.typeIdx;
        info.nameIdx = fieldId.nameIdx;

        // 解析类名、类型名和字段名
        if (fieldId.classIdx < typeIds_.size())
        {
            info.className = getType(fieldId.classIdx);
        }

        if (fieldId.typeIdx < typeIds_.size())
        {
            info.typeName = getType(fieldId.typeIdx);
        }

        if (fieldId.nameIdx < stringIds_.size())
        {
            info.name = getString(fieldId.nameIdx);
        }

        return info;
    }

    bool DexContext::loadAllFields() const
    {
        // 如果已加载所有字段信息，直接返回成功
        if (fieldsLoaded_)
        {
            return true;
        }

        // 检查字段ID表是否存在
        if (fieldIds_.empty() || fileData_ == nullptr)
        {
            LOGE("字段ID表不存在或文件数据为空");
            return false;
        }

        // 重新调整缓存大小
        fieldCache_.resize(fieldIds_.size());

        // 逐个解析字段信息
        for (uint32_t i = 0; i < fieldIds_.size(); i++)
        {
            // 获取字段ID
            const DexFieldId& fieldId = fieldIds_[i];

            // 填充基本信息
            fieldCache_[i].classIdx = fieldId.classIdx;
            fieldCache_[i].typeIdx = fieldId.typeIdx;
            fieldCache_[i].nameIdx = fieldId.nameIdx;

            // 解析类名、类型名和字段名
            if (fieldId.classIdx < typeIds_.size())
            {
                fieldCache_[i].className = getType(fieldId.classIdx);
            }

            if (fieldId.typeIdx < typeIds_.size())
            {
                fieldCache_[i].typeName = getType(fieldId.typeIdx);
            }

            if (fieldId.nameIdx < stringIds_.size())
            {
                fieldCache_[i].name = getString(fieldId.nameIdx);
            }
        }

        // 标记为已加载所有字段信息
        fieldsLoaded_ = true;

        LOGI("加载了 %zu 个字段信息", fieldCache_.size());
        return true;
    }

    // Method相关方法
    void DexContext::setMethodIds(const DexMethodId* methodIds, uint32_t count)
    {
        // 清空现有MethodId表和缓存
        methodIds_.clear();
        methodCache_.clear();
        methodsLoaded_ = false;

        // 复制MethodId表
        if (methodIds != nullptr && count > 0)
        {
            methodIds_.assign(methodIds, methodIds + count);

            // 初始化方法信息缓存
            methodCache_.resize(count);

            // 更新全局DexFile中的方法ID表指针
            dexFile_.pMethodIds = methodIds_.data();
        }
    }

    const std::vector<DexMethodId>& DexContext::getMethodIds() const
    {
        return methodIds_;
    }

    uint32_t DexContext::getMethodIdsCount() const
    {
        return static_cast<uint32_t>(methodIds_.size());
    }

    MethodInfo DexContext::getMethodInfo(uint32_t idx) const
    {
        // 准备空的方法信息
        MethodInfo info = {};
        info.hasParameterList = false;

        // 检查索引是否有效
        if (idx >= methodIds_.size() || fileData_ == nullptr)
        {
            LOGE("方法索引无效: %u", idx);
            return info;
        }

        // 如果已加载所有方法信息，直接返回缓存
        if (methodsLoaded_ && idx < methodCache_.size())
        {
            return methodCache_[idx];
        }

        // 获取方法ID
        const DexMethodId& methodId = methodIds_[idx];

        // 填充基本信息
        info.classIdx = methodId.classIdx;
        info.protoIdx = methodId.protoIdx;
        info.nameIdx = methodId.nameIdx;

        // 解析类名、原型和方法名
        if (methodId.classIdx < typeIds_.size())
        {
            info.className = getType(methodId.classIdx);
        }

        if (methodId.protoIdx < protoIds_.size())
        {
            info.protoShorty = getProtoShorty(methodId.protoIdx);
            info.returnType = getProtoReturnType(methodId.protoIdx);

            // 解析参数列表
            const TypeListData* paramTypeList = getProtoParameters(methodId.protoIdx);
            if (paramTypeList != nullptr)
            {
                info.hasParameterList = true;
                for (uint32_t i = 0; i < paramTypeList->size; i++)
                {
                    const uint32_t typeIdx = paramTypeList->items[i].typeIdx;
                    if (typeIdx < typeIds_.size())
                    {
                        std::string paramType = getType(typeIdx);
                        info.parameterTypes.push_back(paramType);
                    }
                }
            }
        }

        if (methodId.nameIdx < stringIds_.size())
        {
            info.name = getString(methodId.nameIdx);
        }

        return info;
    }

    bool DexContext::loadAllMethods() const
    {
        // 如果已加载所有方法信息，直接返回成功
        if (methodsLoaded_)
        {
            return true;
        }

        // 检查方法ID表是否存在
        if (methodIds_.empty() || fileData_ == nullptr)
        {
            LOGE("方法ID表不存在或文件数据为空");
            return false;
        }

        // 重新调整缓存大小
        methodCache_.resize(methodIds_.size());

        // 逐个解析方法信息
        for (uint32_t i = 0; i < methodIds_.size(); i++)
        {
            // 获取方法ID
            const DexMethodId& methodId = methodIds_[i];

            // 填充基本信息
            methodCache_[i].classIdx = methodId.classIdx;
            methodCache_[i].protoIdx = methodId.protoIdx;
            methodCache_[i].nameIdx = methodId.nameIdx;
            methodCache_[i].hasParameterList = false;

            // 解析类名、原型和方法名
            if (methodId.classIdx < typeIds_.size())
            {
                methodCache_[i].className = getType(methodId.classIdx);
            }

            if (methodId.protoIdx < protoIds_.size())
            {
                methodCache_[i].protoShorty = getProtoShorty(methodId.protoIdx);
                methodCache_[i].returnType = getProtoReturnType(methodId.protoIdx);

                // 解析参数列表
                const TypeListData* paramTypeList = getProtoParameters(methodId.protoIdx);
                if (paramTypeList != nullptr)
                {
                    methodCache_[i].hasParameterList = true;
                    for (uint32_t j = 0; j < paramTypeList->size; j++)
                    {
                        const uint32_t typeIdx = paramTypeList->items[j].typeIdx;
                        if (typeIdx < typeIds_.size())
                        {
                            std::string paramType = getType(typeIdx);
                            methodCache_[i].parameterTypes.push_back(paramType);
                        }
                    }
                }
            }

            if (methodId.nameIdx < stringIds_.size())
            {
                methodCache_[i].name = getString(methodId.nameIdx);
            }
        }

        // 标记为已加载所有方法信息
        methodsLoaded_ = true;

        LOGI("加载了 %zu 个方法信息", methodCache_.size());
        return true;
    }

    // DexFile
    DexFile& DexContext::getDexFile()
    {
        return dexFile_;
    }

    void DexContext::setValid(bool isValid)
    {
        isValid_ = isValid;
    }

    bool DexContext::isValid() const
    {
        return isValid_;
    }

    void DexContext::reset()
    {
        fileData_ = nullptr;
        fileSize_ = 0;
        isValid_ = false;
        stringsLoaded_ = false;
        typeSLoad_ = false;
        protoLoad_ = false;
        fieldsLoaded_ = false;
        methodsLoaded_ = false;
        classDefsLoaded_ = false;

        // 清空头部结构和DexFile结构
        memset(&header_, 0, sizeof(DexHeader));
        memset(&dexFile_, 0, sizeof(DexFile));

        // 清空各种数据和缓存
        stringIds_.clear();
        stringCache_.clear();
        typeIds_.clear();
        typeCache_.clear();
        protoIds_.clear();
        protoCacheShort_.clear();
        protoCacheReturn_.clear();
        protoCacheParameter_.clear();
        fieldIds_.clear();
        fieldCache_.clear();
        methodIds_.clear();
        methodCache_.clear();
        classDefs_.clear();
        classDefCache_.clear();
        typeListCache_.clear();

        LOGI("DexContext重置完成");
    }

    std::string DexContext::decodeMUTF8(const uint8_t* data)
    {
        std::string result;

        // 首先读取ULEB128编码的字符串长度
        uint32_t length = 0;
        uint32_t shift = 0;
        uint8_t byte;

        do
        {
            byte = *data++;
            length |= (byte & 0x7F) << shift;
            shift += 7;
        }
        while (byte & 0x80);

        // 预分配结果字符串空间
        result.reserve(length);

        // 解析MUTF-8编码的字符串
        const uint8_t* end = data + length;
        while (data < end)
        {
            uint8_t c = *data++;
            if ((c & 0x80) == 0)
            {
                // 单字节字符
                result.push_back(static_cast<char>(c));
            }
            else if ((c & 0xE0) == 0xC0)
            {
                // 两字节字符
                if (data >= end) break;
                uint8_t c2 = *data++;
                char16_t ch = ((c & 0x1F) << 6) | (c2 & 0x3F);
                result.push_back(static_cast<char>(ch));
            }
            else if ((c & 0xF0) == 0xE0)
            {
                // 三字节字符
                if (data + 1 >= end) break;
                uint8_t c2 = *data++;
                uint8_t c3 = *data++;
                char16_t ch = ((c & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
                // 简单处理，可能导致非ASCII字符显示不正确
                if (ch < 256)
                {
                    result.push_back(static_cast<char>(ch));
                }
                else
                {
                    result.append("\\u");
                    char hex[5];
                    snprintf(hex, sizeof(hex), "%04X", ch);
                    result.append(hex);
                }
            }
        }

        return result;
    }

    // ClassDef相关方法
    void DexContext::setClassDefs(const DexClassDef* classDefs, uint32_t count)
    {
        // 清空现有ClassDef表和缓存
        classDefs_.clear();
        classDefCache_.clear();
        classDefsLoaded_ = false;

        // 复制ClassDef表
        if (classDefs != nullptr && count > 0)
        {
            classDefs_.assign(classDefs, classDefs + count);

            // 初始化类定义信息缓存
            classDefCache_.resize(count);

            // 更新全局DexFile中的类定义表指针
            dexFile_.pClassDefs = classDefs_.data();
        }
    }

    const std::vector<DexClassDef>& DexContext::getClassDefs() const
    {
        return classDefs_;
    }

    uint32_t DexContext::getClassDefsCount() const
    {
        return static_cast<uint32_t>(classDefs_.size());
    }

    ClassDefInfo DexContext::getClassDefInfo(uint32_t idx) const
    {
        // 准备空的类定义信息
        ClassDefInfo info = {};

        // 检查索引是否有效
        if (idx >= classDefs_.size() || fileData_ == nullptr)
        {
            LOGE("类定义索引无效: %u", idx);
            return info;
        }

        // 如果已加载所有类定义信息，直接返回缓存
        if (classDefsLoaded_ && idx < classDefCache_.size())
        {
            return classDefCache_[idx];
        }

        // 获取类定义
        const DexClassDef& classDef = classDefs_[idx];

        // 填充基本信息
        info.classIdx = classDef.classIdx;
        info.accessFlags = classDef.accessFlags;
        info.superclassIdx = classDef.superclassIdx;
        info.interfacesOff = classDef.interfacesOff;
        info.sourceFileIdx = classDef.sourceFileIdx;
        info.annotationsOff = classDef.annotationsOff;
        info.classDataOff = classDef.classDataOff;
        info.staticValuesOff = classDef.staticValuesOff;

        // 解析类名
        if (classDef.classIdx < typeIds_.size())
        {
            info.className = getType(classDef.classIdx);
        }

        // 解析父类名
        if (classDef.superclassIdx != 0xFFFFFFFF && classDef.superclassIdx < typeIds_.size())
        {
            info.superClassName = getType(classDef.superclassIdx);
        }

        // 解析源文件名
        if (classDef.sourceFileIdx != 0xFFFFFFFF && classDef.sourceFileIdx < stringIds_.size())
        {
            info.sourceFileName = getString(classDef.sourceFileIdx);
        }

        // 解析接口列表
        if (classDef.interfacesOff != 0)
        {
            const TypeListData* interfaces = parseTypeList(classDef.interfacesOff);
            if (interfaces != nullptr)
            {
                for (uint32_t i = 0; i < interfaces->size; i++)
                {
                    if (interfaces->items[i].typeIdx < typeIds_.size())
                    {
                        std::string interfaceName = getType(interfaces->items[i].typeIdx);
                        info.interfaces.push_back(interfaceName);
                    }
                }
            }
        }

        // 尝试解析类数据
        if (classDef.classDataOff != 0)
        {
            // 将临时信息存入缓存
            if (idx < classDefCache_.size())
            {
                classDefCache_[idx] = info;
                // 解析类数据
                parseClassData(idx);
                // 更新info
                info = classDefCache_[idx];
            }
        }

        return info;
    }

    bool DexContext::loadAllClassDefs() const
    {
        // 如果已加载所有类定义信息，直接返回成功
        if (classDefsLoaded_)
        {
            return true;
        }

        // 检查类定义表是否存在
        if (classDefs_.empty() || fileData_ == nullptr)
        {
            LOGE("类定义表不存在或文件数据为空");
            return false;
        }

        // 重新调整缓存大小
        classDefCache_.resize(classDefs_.size());

        // 逐个解析类定义信息
        for (uint32_t i = 0; i < classDefs_.size(); i++)
        {
            // 获取类定义
            const DexClassDef& classDef = classDefs_[i];

            // 填充基本信息
            classDefCache_[i].classIdx = classDef.classIdx;
            classDefCache_[i].accessFlags = classDef.accessFlags;
            classDefCache_[i].superclassIdx = classDef.superclassIdx;
            classDefCache_[i].interfacesOff = classDef.interfacesOff;
            classDefCache_[i].sourceFileIdx = classDef.sourceFileIdx;
            classDefCache_[i].annotationsOff = classDef.annotationsOff;
            classDefCache_[i].classDataOff = classDef.classDataOff;
            classDefCache_[i].staticValuesOff = classDef.staticValuesOff;

            // 解析类名
            if (classDef.classIdx < typeIds_.size())
            {
                classDefCache_[i].className = getType(classDef.classIdx);
            }

            // 解析父类名
            if (classDef.superclassIdx != 0xFFFFFFFF && classDef.superclassIdx < typeIds_.size())
            {
                classDefCache_[i].superClassName = getType(classDef.superclassIdx);
            }

            // 解析源文件名
            if (classDef.sourceFileIdx != 0xFFFFFFFF && classDef.sourceFileIdx < stringIds_.size())
            {
                classDefCache_[i].sourceFileName = getString(classDef.sourceFileIdx);
            }

            // 解析接口列表
            if (classDef.interfacesOff != 0)
            {
                const TypeListData* interfaces = parseTypeList(classDef.interfacesOff);
                if (interfaces != nullptr)
                {
                    for (uint32_t j = 0; j < interfaces->size; j++)
                    {
                        if (interfaces->items[j].typeIdx < typeIds_.size())
                        {
                            std::string interfaceName = getType(interfaces->items[j].typeIdx);
                            classDefCache_[i].interfaces.push_back(interfaceName);
                        }
                    }
                }
            }

            // 解析类数据
            parseClassData(i);
        }

        // 标记为已加载所有类定义信息
        classDefsLoaded_ = true;

        LOGI("加载了 %zu 个类定义信息", classDefCache_.size());
        return true;
    }

    std::string DexContext::getAccessFlagsString(uint32_t flags)
    {
        std::string result;

        // 访问权限标志（只选一个）
        if (flags & ACC_PUBLIC)
        {
            result += "public ";
        }
        else if (flags & ACC_PRIVATE)
        {
            result += "private ";
        }
        else if (flags & ACC_PROTECTED)
        {
            result += "protected ";
        }

        // 其他修饰符
        if (flags & ACC_STATIC)
        {
            result += "static ";
        }
        if (flags & ACC_FINAL)
        {
            result += "final ";
        }
        if (flags & ACC_SYNCHRONIZED)
        {
            result += "synchronized ";
        }
        if (flags & ACC_VOLATILE)
        {
            result += "volatile ";
        }
        if (flags & ACC_BRIDGE)
        {
            result += "bridge ";
        }
        if (flags & ACC_TRANSIENT)
        {
            result += "transient ";
        }
        if (flags & ACC_VARARGS)
        {
            result += "varargs ";
        }
        if (flags & ACC_NATIVE)
        {
            result += "native ";
        }
        if (flags & ACC_INTERFACE)
        {
            result += "interface ";
        }
        if (flags & ACC_ABSTRACT)
        {
            result += "abstract ";
        }
        if (flags & ACC_STRICT)
        {
            result += "strict ";
        }
        if (flags & ACC_SYNTHETIC)
        {
            result += "synthetic ";
        }
        if (flags & ACC_ANNOTATION)
        {
            result += "annotation ";
        }
        if (flags & ACC_ENUM)
        {
            result += "enum ";
        }
        if (flags & ACC_CONSTRUCTOR)
        {
            result += "constructor ";
        }
        if (flags & ACC_DECLARED_SYNCHRONIZED)
        {
            result += "declared-synchronized ";
        }

        // 移除末尾空格
        if (!result.empty())
        {
            result.pop_back();
        }

        return result;
    }

    // 读取ULEB128编码的数值
    uint32_t DexContext::readULEB128(const uint8_t** pData)
    {
        const uint8_t* data = *pData;
        uint32_t result = 0;
        uint32_t shift = 0;
        uint8_t byte;

        do
        {
            byte = *data++;
            result |= (byte & 0x7F) << shift;
            shift += 7;
        }
        while (byte & 0x80);

        *pData = data;
        return result;
    }

    // 解析类数据
    bool DexContext::parseClassData(uint32_t classDefIdx) const
    {
        // 检查索引是否有效
        if (classDefIdx >= classDefs_.size() || fileData_ == nullptr)
        {
            LOGE("类定义索引无效: %u", classDefIdx);
            return false;
        }

        // 获取类定义
        const DexClassDef& classDef = classDefs_[classDefIdx];

        // 检查是否已加载类数据
        if (classDefCache_[classDefIdx].classData.isLoaded)
        {
            return true;
        }

        // 检查类数据偏移量是否有效
        if (classDef.classDataOff == 0)
        {
            // 没有类数据是可以的，例如接口或空类
            classDefCache_[classDefIdx].classData.isLoaded = true;
            return true;
        }

        if (classDef.classDataOff >= fileSize_)
        {
            LOGE("类数据偏移量无效: %u", classDef.classDataOff);
            return false;
        }

        // 获取类数据指针
        const uint8_t* dataPtr = fileData_ + classDef.classDataOff;

        // 读取字段和方法的数量信息(ULEB128编码)
        classDefCache_[classDefIdx].classData.staticFieldsSize = readULEB128(&dataPtr);
        classDefCache_[classDefIdx].classData.instanceFieldsSize = readULEB128(&dataPtr);
        classDefCache_[classDefIdx].classData.directMethodsSize = readULEB128(&dataPtr);
        classDefCache_[classDefIdx].classData.virtualMethodsSize = readULEB128(&dataPtr);

        // 准备字段和方法列表
        auto& classData = classDefCache_[classDefIdx].classData;
        classData.staticFields.resize(classData.staticFieldsSize);
        classData.instanceFields.resize(classData.instanceFieldsSize);
        classData.directMethods.resize(classData.directMethodsSize);
        classData.virtualMethods.resize(classData.virtualMethodsSize);

        // 解析静态字段(ULEB128编码)
        uint32_t fieldIdx = 0;
        for (uint32_t i = 0; i < classData.staticFieldsSize; i++)
        {
            fieldIdx += readULEB128(&dataPtr);
            classData.staticFields[i].fieldIdx = fieldIdx;
            classData.staticFields[i].accessFlags = readULEB128(&dataPtr);

            // 获取字段详细信息
            if (fieldIdx < fieldIds_.size())
            {
                const DexFieldId& fieldId = fieldIds_[fieldIdx];
                if (fieldId.nameIdx < stringIds_.size())
                {
                    classData.staticFields[i].name = getString(fieldId.nameIdx);
                }
                if (fieldId.typeIdx < typeIds_.size())
                {
                    classData.staticFields[i].type = getType(fieldId.typeIdx);
                }
            }
        }

        // 解析实例字段(ULEB128编码)
        fieldIdx = 0;
        for (uint32_t i = 0; i < classData.instanceFieldsSize; i++)
        {
            fieldIdx += readULEB128(&dataPtr);
            classData.instanceFields[i].fieldIdx = fieldIdx;
            classData.instanceFields[i].accessFlags = readULEB128(&dataPtr);

            // 获取字段详细信息
            if (fieldIdx < fieldIds_.size())
            {
                const DexFieldId& fieldId = fieldIds_[fieldIdx];
                if (fieldId.nameIdx < stringIds_.size())
                {
                    classData.instanceFields[i].name = getString(fieldId.nameIdx);
                }
                if (fieldId.typeIdx < typeIds_.size())
                {
                    classData.instanceFields[i].type = getType(fieldId.typeIdx);
                }
            }
        }

        // 解析直接方法(ULEB128编码)
        uint32_t methodIdx = 0;
        for (uint32_t i = 0; i < classData.directMethodsSize; i++)
        {
            methodIdx += readULEB128(&dataPtr);
            classData.directMethods[i].methodIdx = methodIdx;
            classData.directMethods[i].accessFlags = readULEB128(&dataPtr);
            classData.directMethods[i].codeOff = readULEB128(&dataPtr);

            // 获取方法详细信息
            if (methodIdx < methodIds_.size())
            {
                const DexMethodId& methodId = methodIds_[methodIdx];
                if (methodId.nameIdx < stringIds_.size())
                {
                    classData.directMethods[i].name = getString(methodId.nameIdx);
                }

                if (methodId.protoIdx < protoIds_.size())
                {
                    // 获取方法的完整信息以构建更详细的原型字符串
                    MethodInfo methodInfo = getMethodInfo(methodIdx);

                    // 构建参数列表字符串
                    std::string paramStr = "";
                    if (methodInfo.hasParameterList && !methodInfo.parameterTypes.empty())
                    {
                        paramStr = "(";
                        for (size_t j = 0; j < methodInfo.parameterTypes.size(); j++)
                        {
                            if (j > 0)
                            {
                                paramStr += ", ";
                            }
                            paramStr += methodInfo.parameterTypes[j];
                        }
                        paramStr += ")";
                    }
                    else
                    {
                        paramStr = "()";
                    }

                    // 更新Proto字符串，包含返回类型和参数列表
                    classData.directMethods[i].proto = methodInfo.returnType + " " + paramStr;
                }
            }
        }

        // 解析虚拟方法(ULEB128编码)
        methodIdx = 0;
        for (uint32_t i = 0; i < classData.virtualMethodsSize; i++)
        {
            methodIdx += readULEB128(&dataPtr);
            classData.virtualMethods[i].methodIdx = methodIdx;
            classData.virtualMethods[i].accessFlags = readULEB128(&dataPtr);
            classData.virtualMethods[i].codeOff = readULEB128(&dataPtr);

            // 获取方法详细信息
            if (methodIdx < methodIds_.size())
            {
                const DexMethodId& methodId = methodIds_[methodIdx];
                if (methodId.nameIdx < stringIds_.size())
                {
                    classData.virtualMethods[i].name = getString(methodId.nameIdx);
                }

                if (methodId.protoIdx < protoIds_.size())
                {
                    // 获取方法的完整信息以构建更详细的原型字符串
                    MethodInfo methodInfo = getMethodInfo(methodIdx);

                    // 构建参数列表字符串
                    std::string paramStr = "";
                    if (methodInfo.hasParameterList && !methodInfo.parameterTypes.empty())
                    {
                        paramStr = "(";
                        for (size_t j = 0; j < methodInfo.parameterTypes.size(); j++)
                        {
                            if (j > 0)
                            {
                                paramStr += ", ";
                            }
                            paramStr += methodInfo.parameterTypes[j];
                        }
                        paramStr += ")";
                    }
                    else
                    {
                        paramStr = "()";
                    }

                    // 更新Proto字符串，包含返回类型和参数列表
                    classData.virtualMethods[i].proto = methodInfo.returnType + " " + paramStr;
                }
            }
        }

        // 标记类数据已加载
        classDefCache_[classDefIdx].classData.isLoaded = true;

        return true;
    }

    int32_t DexContext::readSLEB128(const uint8_t** pData)
    {
        const uint8_t* data = *pData;
        int32_t result = 0;
        int shift = 0;
        uint8_t byte;

        do {
            byte = *data++;
            result |= ((byte & 0x7F) << shift);
            shift += 7;
        } while (byte & 0x80);

        // 如果最高位是1，则进行符号扩展
        if (shift < 32 && (byte & 0x40)) {
            // 负数，进行符号扩展
            result |= ~0 << shift;
        }

        *pData = data;
        return result;
    }

    // 解析方法代码信息
    bool DexContext::parseMethodCode(uint32_t methodIdx) const
    {
        // 首先需要找到该方法对应的代码偏移量
        uint32_t codeOffset = 0;
        ClassDefInfo::ClassDataInfo::EncodedMethodInfo* pMethod = nullptr;

        // 遍历所有类定义
        for (uint32_t i = 0; i < getClassDefsCount(); i++)
        {
            ClassDefInfo& classInfo = classDefCache_[i];

            // 检查是否有类数据
            if (classInfo.classDataOff != 0 && classInfo.classData.isLoaded)
            {
                // 查找直接方法
                for (auto& method : classInfo.classData.directMethods)
                {
                    if (method.methodIdx == methodIdx && method.codeOff != 0)
                    {
                        codeOffset = method.codeOff;
                        pMethod = &method;
                        break;
                    }
                }

                // 如果在直接方法中没找到，查找虚拟方法
                if (pMethod == nullptr)
                {
                    for (auto& method : classInfo.classData.virtualMethods)
                    {
                        if (method.methodIdx == methodIdx && method.codeOff != 0)
                        {
                            codeOffset = method.codeOff;
                            pMethod = &method;
                            break;
                        }
                    }
                }

                if (pMethod != nullptr)
                {
                    break;
                }
            }
        }

        if (pMethod == nullptr || codeOffset == 0)
        {
            LOGE("未找到方法 %u 的代码段", methodIdx);
            return false;
        }

        // 检查偏移量是否有效
        if (codeOffset >= fileSize_)
        {
            LOGE("代码偏移量无效: 0x%08X", codeOffset);
            return false;
        }

        // 获取DexCode结构
        const DexCode* dexCode = reinterpret_cast<const DexCode*>(fileData_ + codeOffset);

        // 填充代码信息
        CodeInfo& codeInfo = pMethod->codeInfo;
        codeInfo.codeOff = codeOffset;
        codeInfo.registersSize = dexCode->registers_size;
        codeInfo.insSize = dexCode->ins_size;
        codeInfo.outsSize = dexCode->outs_size;
        codeInfo.triesSize = dexCode->tries_size;
        codeInfo.debugInfoOff = dexCode->debug_info_off;
        codeInfo.insnsSize = dexCode->insns_size;

        // 解析调试信息
        if (dexCode->debug_info_off != 0)
        {
            parseDebugInfo(dexCode->debug_info_off, codeInfo.debugInfo);
        }

        // 解析try/catch信息
        if (dexCode->tries_size > 0)
        {
            parseTryCatchInfo(codeOffset, codeInfo);
        }

        codeInfo.isLoaded = true;
        return true;
    }

    // 解析调试信息
    bool DexContext::parseDebugInfo(uint32_t debugInfoOff, DebugInfoData& debugInfo) const
    {
        // 检查缓存
        auto it = debugInfoCache_.find(debugInfoOff);
        if (it != debugInfoCache_.end())
        {
            debugInfo = it->second;
            return true;
        }

        // 检查偏移量是否有效
        if (debugInfoOff == 0 || debugInfoOff >= fileSize_)
        {
            LOGE("调试信息偏移量无效: 0x%08X", debugInfoOff);
            return false;
        }

        const uint8_t* pData = fileData_ + debugInfoOff;

        // 读取起始行号(ULEB128)
        debugInfo.debugInfoOff = debugInfoOff;
        debugInfo.lineStart = readULEB128(&pData);

        // 读取参数数量(ULEB128)
        debugInfo.parametersSize = readULEB128(&pData);

        // 读取参数名称索引(ULEB128)
        debugInfo.parameterNames.clear();
        for (uint32_t i = 0; i < debugInfo.parametersSize; i++)
        {
            uint32_t nameIdx = readULEB128(&pData);
            if (nameIdx != 0 && nameIdx < stringIds_.size())
            {
                debugInfo.parameterNames.push_back(getString(nameIdx));
            }
            else
            {
                // 无名称参数
                debugInfo.parameterNames.push_back("");
            }
        }

        // 状态机初始状态
        uint32_t address = 0;       // 当前地址
        uint32_t line = debugInfo.lineStart;  // 当前行号
        int32_t registerNum = -1;   // 当前寄存器编号
        uint32_t nameIdx = 0;       // 当前变量名称索引
        uint32_t typeIdx = 0;       // 当前变量类型索引
        uint32_t sigIdx = 0;        // 当前变量签名索引

        // 使用状态机处理调试指令
        while (true)
        {
            // 获取当前指令操作码
            uint8_t opcode = *pData++;

            // 处理操作码
            switch (opcode)
            {
                case DexDebugOpCode::DBG_END_SEQUENCE:
                    // 调试指令序列结束
                    goto done;

                case DexDebugOpCode::DBG_ADVANCE_PC:
                    // 推进PC地址
                    address += readULEB128(&pData);
                    break;

                case DexDebugOpCode::DBG_ADVANCE_LINE:
                    // 推进行号
                    line += readSLEB128(&pData);
                    break;

                case DexDebugOpCode::DBG_START_LOCAL:
                    // 局部变量作用域开始
                    {
                        registerNum = readULEB128(&pData);
                        nameIdx = readULEB128(&pData);
                        typeIdx = readULEB128(&pData);

                        LocalVarInfo var;
                        var.registerNum = registerNum;
                        var.nameIdx = nameIdx;
                        var.typeIdx = typeIdx;
                        var.sigIdx = 0;  // 无签名

                        // 获取变量名和类型
                        if (nameIdx != 0 && nameIdx < stringIds_.size())
                        {
                            var.name = getString(nameIdx);
                        }

                        if (typeIdx != 0 && typeIdx < typeIds_.size())
                        {
                            var.type = getType(typeIdx);
                        }

                        debugInfo.localVars.push_back(var);
                    }
                    break;

                case DexDebugOpCode::DBG_START_LOCAL_EXTENDED:
                    // 带签名的局部变量作用域开始
                    {
                        registerNum = readULEB128(&pData);
                        nameIdx = readULEB128(&pData);
                        typeIdx = readULEB128(&pData);
                        sigIdx = readULEB128(&pData);

                        LocalVarInfo var;
                        var.registerNum = registerNum;
                        var.nameIdx = nameIdx;
                        var.typeIdx = typeIdx;
                        var.sigIdx = sigIdx;

                        // 获取变量名、类型和签名
                        if (nameIdx != 0 && nameIdx < stringIds_.size())
                        {
                            var.name = getString(nameIdx);
                        }

                        if (typeIdx != 0 && typeIdx < typeIds_.size())
                        {
                            var.type = getType(typeIdx);
                        }

                        if (sigIdx != 0 && sigIdx < stringIds_.size())
                        {
                            var.signature = getString(sigIdx);
                        }

                        debugInfo.localVars.push_back(var);
                    }
                    break;

                case DexDebugOpCode::DBG_END_LOCAL:
                    // 局部变量作用域结束
                    registerNum = readULEB128(&pData);
                    break;

                case DexDebugOpCode::DBG_RESTART_LOCAL:
                    // 重新开始一个局部变量
                    registerNum = readULEB128(&pData);
                    break;

                case DexDebugOpCode::DBG_SET_PROLOGUE_END:
                    // 方法序言结束位置
                    break;

                case DexDebugOpCode::DBG_SET_EPILOGUE_BEGIN:
                    // 方法结语开始位置
                    break;

                case DexDebugOpCode::DBG_SET_FILE:
                    // 设置当前源文件
                    readULEB128(&pData);  // 文件名索引
                    break;

                default:
                    // 特殊操作码(0x0a-0xff)，同时推进行号和PC
                    if (opcode >= DexDebugOpCode::DBG_SPECIAL_OPCODE_BASE)
                    {
                        uint8_t adjustedOpcode = opcode - DexDebugOpCode::DBG_SPECIAL_OPCODE_BASE;

                        // 行号变化
                        int32_t lineAdj = (adjustedOpcode % 15) - 4;

                        // PC地址变化
                        uint32_t addrAdj = adjustedOpcode / 15;

                        // 更新状态
                        line += lineAdj;
                        address += addrAdj;

                        // 记录位置信息
                        PositionInfo pos;
                        pos.address = address;
                        pos.lineNum = line;
                        debugInfo.positions.push_back(pos);
                    }
                    break;
            }
        }

    done:
        // 将调试信息添加到缓存
        debugInfo.isLoaded = true;
        debugInfoCache_[debugInfoOff] = debugInfo;

        return true;
    }

    // 解析Try/Catch信息
    bool DexContext::parseTryCatchInfo(uint32_t codeOff, CodeInfo& codeInfo) const
    {
        // 检查偏移量是否有效
        if (codeOff == 0 || codeOff >= fileSize_)
        {
            LOGE("代码偏移量无效: 0x%08X", codeOff);
            return false;
        }

        // 获取DexCode结构
        const DexCode* dexCode = reinterpret_cast<const DexCode*>(fileData_ + codeOff);

        // 检查try块数量
        if (dexCode->tries_size == 0)
        {
            return true;  // 没有try块
        }

        // 获取tries数组的起始位置
        // tries数组在insns之后，但如果insns的大小是奇数，中间会有两个字节的padding
        const uint8_t* pData = reinterpret_cast<const uint8_t*>(dexCode->insns + dexCode->insns_size);
        if (dexCode->insns_size % 2 != 0) {
            pData += 2;  // 添加2字节的padding
        }

        // 获取DexTry数组
        const DexTry* tries = reinterpret_cast<const DexTry*>(pData);

        // 获取handlers区域的起始位置
        const uint8_t* handlersData = pData + (dexCode->tries_size * sizeof(DexTry));

        // 读取handlers区域的大小
        uint32_t handlersSize = readULEB128(&handlersData);

        // 解析每个try块
        codeInfo.tries.clear();
        for (uint32_t i = 0; i < dexCode->tries_size; i++)
        {
            TryBlockInfo tryInfo;
            tryInfo.startAddr = tries[i].start_addr;
            tryInfo.insnCount = tries[i].insn_count;
            tryInfo.handlerOff = tries[i].handler_off;
            tryInfo.hasCatchAll = false;

            // 找到对应的handler
            const uint8_t* handlerData = handlersData + tryInfo.handlerOff;

            // 读取handler的大小
            int32_t size = readSLEB128(&handlerData);
            bool hasCatchAll = size <= 0;
            uint32_t catchCount = hasCatchAll ? -size : size;

            // 解析每个catch类型
            for (uint32_t j = 0; j < catchCount; j++)
            {
                TryBlockInfo::CatchInfo catchInfo;

                // 读取捕获的异常类型
                uint32_t typeIdx = readULEB128(&handlerData);
                catchInfo.typeIdx = typeIdx;

                // 读取处理器地址
                uint32_t addr = readULEB128(&handlerData);
                catchInfo.address = addr;

                // 获取类型名称
                if (typeIdx < typeIds_.size())
                {
                    catchInfo.typeName = getType(typeIdx);
                }

                tryInfo.catches.push_back(catchInfo);
            }

            // 如果有catch-all处理器
            if (hasCatchAll)
            {
                tryInfo.hasCatchAll = true;
                tryInfo.catchAllAddr = readULEB128(&handlerData);
            }

            codeInfo.tries.push_back(tryInfo);
        }

        return true;
    }

}
