//
// Created by GaGa on 25-5-5.
//

#ifndef DEXFILE_H
#define DEXFILE_H

#include <cstdint>
#include <semaphore>

typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;
typedef uint64_t u8;
typedef int8_t s1;
typedef int16_t s2;
typedef int32_t s4;
typedef int64_t s8;

enum
{
    kSHA1DigestLen = 20,
    kSHA1DigestOutputLen = kSHA1DigestLen * 2 + 1
};


/**
 * 优化后的Dex文件结构
 */
struct DexOptHeader
{
    u1 magic[8]; /* includes version number */

    u4 dexOffset; /* file offset of DEX header */
    u4 dexLength;
    u4 depsOffset; /* offset of optimized DEX dependency table */
    u4 depsLength;
    u4 optOffset; /* file offset of optimized data tables */
    u4 optLength;

    u4 flags; /* some info flags */
    u4 checksum; /* adler32 checksum covering deps/opt */

    /* pad for 64-bit alignment if necessary */
};


/**
 * 暂时未使用，保留
 */
struct DexLink
{
    u1 bleargh;
};

struct DexHeader
{
    u1 magic[8]; /* includes version number */
    u4 checksum; /* adler32 checksum */
    u1 signature[kSHA1DigestLen]; /* SHA-1 hash */
    u4 fileSize; /* length of entire file */
    u4 headerSize; /* offset to start of next section */
    u4 endianTag;
    u4 linkSize;
    u4 linkOff;
    u4 mapOff;
    u4 stringIdsSize;
    u4 stringIdsOff;
    u4 typeIdsSize;
    u4 typeIdsOff;
    u4 protoIdsSize;
    u4 protoIdsOff;
    u4 fieldIdsSize;
    u4 fieldIdsOff;
    u4 methodIdsSize;
    u4 methodIdsOff;
    u4 classDefsSize;
    u4 classDefsOff;
    u4 dataSize;
    u4 dataOff;
};

struct DexStringId
{
    u4 stringDataOff;
};

struct DexTypeId
{
    u4 descriptor_idx;
};

struct DexProtoId
{
    u4 shorty_idx; /*方法简短描述符，是StringIds索引 */
    u4 return_type_idx; /* 方法返回值类型，是TypeIds索引 */
    u4 parameters_off; /*方法原型参数类型，指向Type List*/
};

struct DexTypeItem
{
    u2 typeIdx; /* 方法参数类型，是TypeIds索引 */
};

struct DexTypeList
{
    u4 size;
    DexTypeItem list[1]; /* 这里的DexTypeItem大小由size决定*/
};

struct DexFieldId
{
    u2 classIdx; /* index into typeIds */
    u2 typeIdx; /* index into typeIds */
    u4 nameIdx; /* index into stringIds */
};

struct DexMethodId
{
    u2 classIdx; /* index into typeIds */
    u2 protoIdx; /* index into protoIds */
    u4 nameIdx; /* index into stringIds */
};

struct DexClassDef
{
    u4 classIdx; /* index into typeIds for this class */
    u4 accessFlags;
    u4 superclassIdx; /* index into typeIds for superclass */
    u4 interfacesOff; /* file offset to DexTypeList */
    u4 sourceFileIdx; /* index into stringIds for source file name */
    u4 annotationsOff; /* file offset to annotations_directory_item */
    u4 classDataOff; /* file offset to class_data_item */
    u4 staticValuesOff; /* file offset to DexEncodedArray */
};

/**
 * 类数据结构，表示class_data_item
 * 所有字段采用ULEB128编码
 */
struct DexClassData
{
    u4 static_fields_size;       // 静态字段数量
    u4 instance_fields_size;     // 实例字段数量
    u4 direct_methods_size;      // 直接方法数量
    u4 virtual_methods_size;     // 虚拟方法数量
};

/**
 * 编码后的字段，表示encoded_field
 */
struct DexEncodedField
{
    u4 field_idx_diff;           // 字段索引差值(ULEB128)
    u4 access_flags;             // 访问标志(ULEB128)
};

/**
 * 编码后的方法，表示encoded_method
 */
struct DexEncodedMethod
{
    u4 method_idx_diff;          // 方法索引差值(ULEB128)
    u4 access_flags;             // 访问标志(ULEB128)
    u4 code_off;                 // 代码偏移量(ULEB128)
};

struct DexCode
{
    u2 registers_size;
    u2 ins_size;
    u2 outs_size;
    u2 tries_size;
    u4 debug_info_off;
    u4 insns_size;
    u2 insns[1];
    // 可选: tries, handlers
};

/**
 * 调试信息头部
 * 采用ULEB128编码
 */
struct DexDebugInfo
{
    u4 line_start;           // 起始行号(ULEB128)
    u4 parameters_size;      // 参数名称数量(ULEB128)
    u4* parameter_names;     // 参数名称索引列表(ULEB128)，索引到字符串ID表，0表示无名称
    // 后面跟随调试信息状态机指令
};

/**
 * 调试信息状态机指令操作码定义
 */
enum DexDebugOpCode
{
    DBG_END_SEQUENCE      = 0x00,  // 结束标记
    DBG_ADVANCE_PC        = 0x01,  // 推进程序计数器
    DBG_ADVANCE_LINE      = 0x02,  // 推进源代码行号
    DBG_START_LOCAL       = 0x03,  // 开始一个局部变量作用域
    DBG_START_LOCAL_EXTENDED = 0x04, // 带类型签名的局部变量作用域开始
    DBG_END_LOCAL         = 0x05,  // 局部变量作用域结束
    DBG_RESTART_LOCAL     = 0x06,  // 重新开始一个局部变量
    DBG_SET_PROLOGUE_END  = 0x07,  // 方法序言结束位置
    DBG_SET_EPILOGUE_BEGIN = 0x08, // 方法结语开始位置
    DBG_SET_FILE          = 0x09,  // 设置当前源文件
    DBG_SPECIAL_OPCODE_BASE = 0x0a, // 特殊操作码起始
    // 特殊操作码范围：0x0a-0xff，用于同时推进行号和PC
};

/**
 * Try/Catch块定义
 */
struct DexTry
{
    u4 start_addr;          // 尝试块开始地址（以16位指令为单位）
    u2 insn_count;          // 尝试块指令数量
    u2 handler_off;         // 异常处理器的偏移（相对于handler区域起始）
};

/**
 * 异常处理器类型编码，采用SLEB128/ULEB128编码
 */
struct DexCatchHandler
{
    s4 size;               // 捕获的异常类型数量，负值表示有finally块
    // 以下字段数量由size决定
    // u4* type_idx;          // 捕获的异常类型索引列表(ULEB128)
    // u4* addr;              // 处理器地址列表(ULEB128)
    // u4 catch_all_addr;     // 如果size为负，catch-all处理器地址(ULEB128)
};

struct DexMapItem
{
    u2 type; /* type of item */
    u2 unused; /* reserved, must be zero */
    u4 size; /* number of items in this section */
    u4 offset; /* file offset to data */
};

struct DexMapList
{
    u4 size;
    DexMapItem dex_map_item[1];
};


/**
 * 表示Dex文件结构。提供操作Dex文件的API
 */
struct DexFile
{
    const DexOptHeader* pOptHeader;

    const DexHeader* pHeader;
    const DexLink* pLink;
    const DexStringId* pStringIds;
    const DexTypeId* pTypeIds;
    const DexProtoId* pProtoIds;
    const DexFieldId* pFieldIds;
    const DexMethodId* pMethodIds;
    const DexClassDef* pClassDefs;
    const DexMapList* pMapList;
};

#endif //DEXFILE_H
