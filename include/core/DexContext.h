//
// Created by DexDump on 2025-5-10.
//

#ifndef DEXCONTEXT_H
#define DEXCONTEXT_H

#include <cstdint>
#include <vector>
#include <string>
#include <map>
#include "DexFile.h"
#include "parser/ProtoParser.h"

namespace dex
{
    // 用于存储TypeList及其相关TypeItem的结构
    struct TypeListData {
        uint32_t size;                      // 列表中项的数量
        std::vector<DexTypeItem> items;     // TypeItem数组
    };

    // 字段信息结构体
    struct FieldInfo {
        uint32_t classIdx;        // 所属类索引
        uint32_t typeIdx;         // 字段类型索引
        uint32_t nameIdx;         // 字段名称索引
        std::string className;    // 所属类名
        std::string typeName;     // 字段类型名
        std::string name;         // 字段名
    };

    // 方法信息结构体
    struct MethodInfo {
        uint32_t classIdx;        // 所属类索引
        uint32_t protoIdx;        // 原型索引
        uint32_t nameIdx;         // 方法名称索引
        std::string className;    // 所属类名
        std::string protoShorty;  // 原型简短描述
        std::string returnType;   // 返回类型
        std::string name;         // 方法名
        std::vector<std::string> parameterTypes;  // 参数类型列表
        bool hasParameterList;    // 是否有参数列表
    };
    
    // 访问标志定义（用于类、方法和字段）
    enum AccessFlags {
        ACC_PUBLIC = 0x00000001,       // 公共
        ACC_PRIVATE = 0x00000002,      // 私有
        ACC_PROTECTED = 0x00000004,    // 保护
        ACC_STATIC = 0x00000008,       // 静态
        ACC_FINAL = 0x00000010,        // 最终
        ACC_SYNCHRONIZED = 0x00000020, // 同步
        ACC_VOLATILE = 0x00000040,     // 易变
        ACC_BRIDGE = 0x00000040,       // 桥接方法
        ACC_TRANSIENT = 0x00000080,    // 瞬态
        ACC_VARARGS = 0x00000080,      // 可变参数数量
        ACC_NATIVE = 0x00000100,       // 本地
        ACC_INTERFACE = 0x00000200,    // 接口
        ACC_ABSTRACT = 0x00000400,     // 抽象
        ACC_STRICT = 0x00000800,       // 严格
        ACC_SYNTHETIC = 0x00001000,    // 合成
        ACC_ANNOTATION = 0x00002000,   // 注解
        ACC_ENUM = 0x00004000,         // 枚举
        ACC_CONSTRUCTOR = 0x00010000,  // 构造函数
        ACC_DECLARED_SYNCHRONIZED = 0x00020000, // 同步声明
    };
    
    // 局部变量信息结构体
    struct LocalVarInfo {
        uint32_t registerNum;         // 寄存器编号
        uint32_t nameIdx;             // 变量名索引(字符串表)
        uint32_t typeIdx;             // 变量类型索引(类型表)
        uint32_t sigIdx;              // 类型签名索引(字符串表)，仅用于泛型
        std::string name;             // 变量名
        std::string type;             // 变量类型
        std::string signature;        // 类型签名
    };
    
    // 位置信息结构体
    struct PositionInfo {
        uint32_t address;             // 指令地址
        uint32_t lineNum;             // 源代码行号
    };
    
    // 调试信息结构体
    struct DebugInfoData {
        uint32_t debugInfoOff;        // 调试信息在文件中的偏移量
        uint32_t lineStart;           // 起始行号
        uint32_t parametersSize;      // 参数数量
        std::vector<std::string> parameterNames; // 参数名称列表
        std::vector<LocalVarInfo> localVars;     // 局部变量列表
        std::vector<PositionInfo> positions;     // 位置列表
        bool isLoaded;                // 是否已加载
        
        // 构造函数
        DebugInfoData() :
            debugInfoOff(0),
            lineStart(0),
            parametersSize(0),
            isLoaded(false) {}
    };
    
    // Try块信息结构体
    struct TryBlockInfo {
        uint32_t startAddr;           // 开始地址
        uint32_t insnCount;           // 指令数量
        uint32_t handlerOff;          // 处理器偏移量
        
        // 异常处理器信息
        struct CatchInfo {
            int32_t typeIdx;          // 捕获的异常类型索引，-1表示catch-all
            uint32_t address;         // 处理器地址
            std::string typeName;     // 异常类型名
        };
        
        std::vector<CatchInfo> catches; // 捕获列表
        bool hasCatchAll;             // 是否有catch-all处理器
        uint32_t catchAllAddr;        // catch-all处理器地址
    };
    
    // 代码信息结构体
    struct CodeInfo {
        uint32_t codeOff;             // 代码偏移量
        uint16_t registersSize;       // 寄存器数量
        uint16_t insSize;             // 输入参数数量
        uint16_t outsSize;            // 输出参数数量
        uint16_t triesSize;           // try块数量
        uint32_t debugInfoOff;        // 调试信息偏移量
        uint32_t insnsSize;           // 指令数量
        std::vector<TryBlockInfo> tries; // try块列表
        DebugInfoData debugInfo;      // 调试信息
        bool isLoaded;                // 是否已加载
        
        // 构造函数
        CodeInfo() :
            codeOff(0),
            registersSize(0),
            insSize(0),
            outsSize(0),
            triesSize(0),
            debugInfoOff(0),
            insnsSize(0),
            isLoaded(false) {}
    };

    // 类定义信息结构体
    struct ClassDefInfo {
        uint32_t classIdx;          // 类索引
        uint32_t accessFlags;       // 访问标志
        uint32_t superclassIdx;     // 父类索引
        uint32_t interfacesOff;     // 接口偏移量
        uint32_t sourceFileIdx;     // 源文件索引
        uint32_t annotationsOff;    // 注解偏移量
        uint32_t classDataOff;      // 类数据偏移量
        uint32_t staticValuesOff;   // 静态值偏移量
        
        std::string className;      // 类名
        std::string superClassName; // 父类名
        std::string sourceFileName; // 源文件名
        
        std::vector<std::string> interfaces; // 接口名称列表
        
        // 类数据相关信息
        struct ClassDataInfo {
            uint32_t staticFieldsSize;     // 静态字段数量
            uint32_t instanceFieldsSize;   // 实例字段数量
            uint32_t directMethodsSize;    // 直接方法数量
            uint32_t virtualMethodsSize;   // 虚拟方法数量
            
            // 编码字段信息
            struct EncodedFieldInfo {
                uint32_t fieldIdx;         // 字段索引
                uint32_t accessFlags;      // 访问标志
                std::string name;          // 字段名称
                std::string type;          // 字段类型
            };
            
            // 编码方法信息
            struct EncodedMethodInfo {
                uint32_t methodIdx;        // 方法索引
                uint32_t accessFlags;      // 访问标志
                uint32_t codeOff;          // 代码偏移量
                std::string name;          // 方法名称
                std::string proto;         // 方法原型
                CodeInfo codeInfo;         // 代码信息
            };
            
            std::vector<EncodedFieldInfo> staticFields;      // 静态字段列表
            std::vector<EncodedFieldInfo> instanceFields;    // 实例字段列表
            std::vector<EncodedMethodInfo> directMethods;    // 直接方法列表
            std::vector<EncodedMethodInfo> virtualMethods;   // 虚拟方法列表
            
            bool isLoaded;                 // 是否已加载类数据
            
            // 构造函数
            ClassDataInfo() : 
                staticFieldsSize(0), 
                instanceFieldsSize(0), 
                directMethodsSize(0), 
                virtualMethodsSize(0),
                isLoaded(false) {}
        };
        
        ClassDataInfo classData;          // 类数据信息
        
        // 构造函数
        ClassDefInfo() : 
            classIdx(0), 
            accessFlags(0), 
            superclassIdx(0), 
            interfacesOff(0), 
            sourceFileIdx(0), 
            annotationsOff(0), 
            classDataOff(0), 
            staticValuesOff(0) {}
    };

    /**
     * DexContext - 全局上下文单例类
     * 管理全局的DEX文件数据结构，作为解析器和格式化器之间的桥梁
     */
    class DexContext
    {
    public:
        // 获取单例实例
        static DexContext& getInstance();

        // 禁止拷贝和赋值
        DexContext(const DexContext&) = delete;
        DexContext& operator=(const DexContext&) = delete;

        // 设置DEX文件数据
        void setFileData(const uint8_t* fileData, size_t fileSize);

        // 获取DEX文件数据
        const uint8_t* getFileData() const;

        // 获取DEX文件大小
        size_t getFileSize() const;

        // 设置DEX头部结构
        void setHeader(const DexHeader& header);

        // 获取DEX头部结构
        const DexHeader& getHeader() const;

        // 设置字符串ID表
        void setStringIds(const DexStringId* stringIds, uint32_t count);

        // 获取字符串ID表
        const std::vector<DexStringId>& getStringIds() const;

        // 获取字符串ID表大小
        uint32_t getStringIdsCount() const;

        // 获取字符串内容
        std::string getString(uint32_t idx) const;

        // 加载所有字符串内容到内存
        bool loadAllStrings() const;

        // 加载字符到Type中
        bool loadStringType() const;


        // 设置TypeID表
        void setTypeIds(const DexTypeId* typeIds, uint32_t count);

        // 获取TypeID表
        const std::vector<DexTypeId>& getTypeIds() const;

        // 获取TypeID表大小
        uint32_t getTypeIdsCount() const;

        // 获取Type类型内存
        std::string getType(uint32_t idx) const;

        // 设置ProtoId表
        void setProtoIds(const DexProtoId* proto_id, uint32_t count);
        
        // 获取Proto表
        const std::vector<DexProtoId>& getProtoIds() const;
        
        // 获取Proto表大小
        uint32_t getProtoIdsCount() const;
        
        // 获取Proto的shorty字符串
        std::string getProtoShorty(uint32_t idx) const;
        
        // 获取Proto的返回类型
        std::string getProtoReturnType(uint32_t idx) const;
        
        // 获取Proto的参数列表
        const TypeListData* getProtoParameters(uint32_t idx) const;
        
        // 加载所有Proto信息
        bool loadAllProtos() const;
        
        // 解析并缓存TypeList
        const TypeListData* parseTypeList(uint32_t offset) const;

        // 设置FieldId表
        void setFieldIds(const DexFieldId* fieldIds, uint32_t count);
        
        // 获取Field表
        const std::vector<DexFieldId>& getFieldIds() const;
        
        // 获取Field表大小
        uint32_t getFieldIdsCount() const;
        
        // 获取Field信息
        FieldInfo getFieldInfo(uint32_t idx) const;
        
        // 加载所有Field信息
        bool loadAllFields() const;

        // 设置MethodId表
        void setMethodIds(const DexMethodId* methodIds, uint32_t count);
        
        // 获取Method表
        const std::vector<DexMethodId>& getMethodIds() const;
        
        // 获取Method表大小
        uint32_t getMethodIdsCount() const;
        
        // 获取Method信息
        MethodInfo getMethodInfo(uint32_t idx) const;
        
        // 加载所有Method信息
        bool loadAllMethods() const;
        
        // 设置ClassDef表
        void setClassDefs(const DexClassDef* classDefs, uint32_t count);
        
        // 获取ClassDef表
        const std::vector<DexClassDef>& getClassDefs() const;
        
        // 获取ClassDef表大小
        uint32_t getClassDefsCount() const;
        
        // 获取ClassDef信息
        ClassDefInfo getClassDefInfo(uint32_t idx) const;
        
        // 加载所有ClassDef信息
        bool loadAllClassDefs() const;
        
        // 解析方法代码信息
        bool parseMethodCode(uint32_t methodIdx) const;
        
        // 解析调试信息
        bool parseDebugInfo(uint32_t debugInfoOff, DebugInfoData& debugInfo) const;
        
        // 解析Try/Catch信息
        bool parseTryCatchInfo(uint32_t codeOff, CodeInfo& codeInfo) const;
        
        // 获取AccessFlags的字符串表示
        static std::string getAccessFlagsString(uint32_t flags);

        // 获取全局DexFile结构
        DexFile& getDexFile();

        // 设置解析状态
        void setValid(bool isValid);

        // 获取解析状态
        bool isValid() const;

        // 重置上下文（清除所有数据）
        void reset();

        // 解析类数据
        bool parseClassData(uint32_t classDefIdx) const;

        // 读取ULEB128编码的数值
        static uint32_t readULEB128(const uint8_t** pData);
        
        // 读取有符号SLEB128编码的数值
        static int32_t readSLEB128(const uint8_t** pData);

    private:
        // 私有构造函数
        DexContext();
        
        // 解析MUTF-8字符串内容
        static std::string decodeMUTF8(const uint8_t* data);
        
        // 文件数据指针
        const uint8_t* fileData_;
        
        // 文件大小
        size_t fileSize_;
        
        // DEX头部结构
        DexHeader header_;
        
        // 字符串ID表（动态数组）
        std::vector<DexStringId> stringIds_;
        
        // TypeID表
        std::vector<DexTypeId> typeIds_;

        // ProtoId表
        std::vector<DexProtoId> protoIds_;
        
        // FieldId表
        std::vector<DexFieldId> fieldIds_;
        
        // MethodId表
        std::vector<DexMethodId> methodIds_;
        
        // ClassDef表
        std::vector<DexClassDef> classDefs_;
        
        // TypeList缓存，使用偏移量作为键
        mutable std::map<uint32_t, TypeListData> typeListCache_;
        
        // DebugInfo缓存，使用偏移量作为键
        mutable std::map<uint32_t, DebugInfoData> debugInfoCache_;

        // 已解析的字符串内容缓存（mutable允许在const方法中修改）
        mutable std::vector<std::string> stringCache_;
        mutable std::vector<std::string> typeCache_;

        mutable std::vector<std::string> protoCacheShort_;
        mutable std::vector<std::string> protoCacheReturn_;
        mutable std::vector<std::string> protoCacheParameter_;
        
        // 字段信息缓存
        mutable std::vector<FieldInfo> fieldCache_;
        
        // 方法信息缓存
        mutable std::vector<MethodInfo> methodCache_;
        
        // 类定义信息缓存
        mutable std::vector<ClassDefInfo> classDefCache_;

        // 是否已加载各类数据
        mutable bool stringsLoaded_;
        mutable bool typeSLoad_;
        mutable bool protoLoad_;
        mutable bool fieldsLoaded_;
        mutable bool methodsLoaded_;
        mutable bool classDefsLoaded_;

        // 总体结构
        DexFile dexFile_;
        
        // 解析状态
        bool isValid_;
    };
}

#endif // DEXCONTEXT_H
