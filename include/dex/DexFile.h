//
// Created by GaGa on 25-5-5.
//

#ifndef DEXFILE_H
#define DEXFILE_H

#include <cstdint>

typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;
typedef uint64_t u8;
typedef int8_t s1;
typedef int16_t s2;
typedef int32_t s4;
typedef int64_t s8;

enum { kSHA1DigestLen = 20,
       kSHA1DigestOutputLen = kSHA1DigestLen*2 +1 };

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


struct DexprotoId
{
    u4 shorty_idx;
    u4 return_type_idx;
    u4 parameters_off;
};

struct DexTypeItem {
    u2  typeIdx;            /* index into typeIds */
};

struct DexTypeList
{
    u4 size;
    DexTypeItem list[1];

};

struct DexFieldId
{
    u2 classIdx;           /* index into typeIds */
    u2 typeIdx;            /* index into typeIds */
    u4 nameIdx;            /* index into stringIds */
};
struct DexMethodId
{
    u2 classIdx;           /* index into typeIds */
    u2 protoIdx;           /* index into protoIds */
    u4 nameIdx;            /* index into stringIds */
};


struct DexClassDef {
    u4  classIdx;           /* index into typeIds for this class */
    u4  accessFlags;
    u4  superclassIdx;      /* index into typeIds for superclass */
    u4  interfacesOff;      /* file offset to DexTypeList */
    u4  sourceFileIdx;      /* index into stringIds for source file name */
    u4  annotationsOff;     /* file offset to annotations_directory_item */
    u4  classDataOff;       /* file offset to class_data_item */
    u4  staticValuesOff;    /* file offset to DexEncodedArray */
};

/**
 * class_data_item
 */
struct DexClassData
{
    u1 static_fields_size;
    u1 instance_fields_size;
    u1 direct_methods_size;
    u1 virtual_methods_size;
};

struct DexEncodeMethod
{
    u4 method_idx_diff;
    u4 access_flags;
    u4 code_off;
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
};

struct DexDebugInfo
{
    u4 line_start;
    u4 parameters_size;
    u4 parameter_names;
};

struct DexMapItem
{
    u2 type;               /* type of item */
    u2 unused;            /* reserved, must be zero */
    u4 size;              /* number of items in this section */
    u4 offset;            /* file offset to data */
};

struct DexMapList
{
    u4 size;
    DexMapItem dex_map_item[1];
};

#endif //DEXFILE_H
