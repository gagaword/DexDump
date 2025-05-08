//
// Created by GaGa on 25-5-5.
//

#include <cstdio>
#include<Windows.h>
#include "dex/DexFile.h"

/**
 * LEB128一种变长编码方式，ULEB128是无符号的LEB128
 * ULEB128的高位是标志位，表示是否还有下一个字节。
 * 如果最高位为0，表示没有下一个字节了
 * 如果最高位是1表示后续还有字节，直到遇到最高位为0就代表这个LEB128解析完毕。
 * 除去最高位0，剩下的7为就是需要的数据
 * @param data 需要解析的数据
 * @return 解析好的数据
 */
uint32_t parse_uleb128(const uint8_t** data)
{
    uint32_t result = 0;
    int shift = 0;
    while (1)
    {
        uint8_t byte = **data;
        (*data)++; // 关键：解析完一个字节后推进指针
        result |= (byte & 0x7F) << shift;
        if ((byte & 0x80) == 0) break;
        shift += 7;
    }
    return result;
}

int32_t parse_uleb128p1(const uint8_t** ptr)
{
    uint32_t raw = parse_uleb128(ptr);
    return (int32_t)(raw) - 1;
}

int32_t parse_sleb128(const uint8_t** ptr)
{
    int32_t result = 0;
    int shift = 0;
    uint8_t byte;

    do
    {
        byte = *(*ptr)++;
        result |= (byte & 0x7f) << shift;
        shift += 7;
    }
    while (byte & 0x80);

    // 如果符号位是1，说明是负数，进行符号扩展
    if ((byte & 0x40) && (shift < 32))
    {
        result |= -(1 << shift); // 符号扩展
    }

    return result;
}


enum
{
    ACC_PUBLIC = 0x00000001, // class, field, method, ic
    ACC_PRIVATE = 0x00000002, // field, method, ic
    ACC_PROTECTED = 0x00000004, // field, method, ic
    ACC_STATIC = 0x00000008, // field, method, ic
    ACC_FINAL = 0x00000010, // class, field, method, ic
    ACC_SYNCHRONIZED = 0x00000020, // method (only allowed on natives)
    ACC_SUPER = 0x00000020, // class (not used in Dalvik)
    ACC_VOLATILE = 0x00000040, // field
    ACC_BRIDGE = 0x00000040, // method (1.5)
    ACC_TRANSIENT = 0x00000080, // field
    ACC_VARARGS = 0x00000080, // method (1.5)
    ACC_NATIVE = 0x00000100, // method
    ACC_INTERFACE = 0x00000200, // class, ic
    ACC_ABSTRACT = 0x00000400, // class, method, ic
    ACC_STRICT = 0x00000800, // method
    ACC_SYNTHETIC = 0x00001000, // field, method, ic
    ACC_ANNOTATION = 0x00002000, // class, ic (1.5)
    ACC_ENUM = 0x00004000, // class, field, ic (1.5)
    ACC_CONSTRUCTOR = 0x00010000, // method (Dalvik only)
    ACC_DECLARED_SYNCHRONIZED =
    0x00020000, // method (Dalvik only)
    ACC_CLASS_MASK =
    (ACC_PUBLIC | ACC_FINAL | ACC_INTERFACE | ACC_ABSTRACT
        | ACC_SYNTHETIC | ACC_ANNOTATION | ACC_ENUM),
    ACC_INNER_CLASS_MASK =
    (ACC_CLASS_MASK | ACC_PRIVATE | ACC_PROTECTED | ACC_STATIC),
    ACC_FIELD_MASK =
    (ACC_PUBLIC | ACC_PRIVATE | ACC_PROTECTED | ACC_STATIC | ACC_FINAL
        | ACC_VOLATILE | ACC_TRANSIENT | ACC_SYNTHETIC | ACC_ENUM),
    ACC_METHOD_MASK =
    (ACC_PUBLIC | ACC_PRIVATE | ACC_PROTECTED | ACC_STATIC | ACC_FINAL
        | ACC_SYNCHRONIZED | ACC_BRIDGE | ACC_VARARGS | ACC_NATIVE
        | ACC_ABSTRACT | ACC_STRICT | ACC_SYNTHETIC | ACC_CONSTRUCTOR
        | ACC_DECLARED_SYNCHRONIZED),
};

enum
{
    DBG_END_SEQUENCE = 0x00,
    DBG_ADVANCE_PC = 0x01,
    DBG_ADVANCE_LINE = 0x02,
    DBG_START_LOCAL = 0x03,
    DBG_START_LOCAL_EXTENDED = 0x04,
    DBG_END_LOCAL = 0x05,
    DBG_RESTART_LOCAL = 0x06,
    DBG_SET_PROLOGUE_END = 0x07,
    DBG_SET_EPILOGUE_BEGIN = 0x08,
    DBG_SET_FILE = 0x09,
    DBG_FIRST_SPECIAL = 0x0a,
    DBG_LINE_BASE = -4,
    DBG_LINE_RANGE = 15,
};

typedef struct
{
    uint32_t flag;
    const char* name;
} AccessFlag;

// 按标志值从小到大排序（便于二分查找）
static const AccessFlag ACCESS_FLAGS[] = {
    {ACC_PUBLIC, "PUBLIC"},
    {ACC_PRIVATE, "PRIVATE"},
    {ACC_PROTECTED, "PROTECTED"},
    {ACC_STATIC, "STATIC"},
    {ACC_FINAL, "FINAL"},
    {ACC_SYNCHRONIZED, "SYNCHRONIZED"},
    {ACC_VOLATILE, "VOLATILE"},
    {ACC_BRIDGE, "BRIDGE"},
    {ACC_TRANSIENT, "TRANSIENT"},
    {ACC_VARARGS, "VARARGS"},
    {ACC_NATIVE, "NATIVE"},
    {ACC_INTERFACE, "INTERFACE"},
    {ACC_ABSTRACT, "ABSTRACT"},
    {ACC_STRICT, "STRICT"},
    {ACC_SYNTHETIC, "SYNTHETIC"},
    {ACC_ANNOTATION, "ANNOTATION"},
    {ACC_ENUM, "ENUM"},
    {ACC_CONSTRUCTOR, "CONSTRUCTOR"},
    {ACC_DECLARED_SYNCHRONIZED, "DECLARED_SYNCHRONIZED"}
};

void print_access_flags(uint32_t flags)
{
    printf("Access Flags (0x%08X): ", flags);

    if (flags == 0)
    {
        printf("(none)");
    }
    else
    {
        int count = 0;
        for (size_t i = 0; i < sizeof(ACCESS_FLAGS) / sizeof(AccessFlag); i++)
        {
            if (flags & ACCESS_FLAGS[i].flag)
            {
                if (count++ > 0) printf(" | ");
                printf("%s", ACCESS_FLAGS[i].name);
                flags &= ~ACCESS_FLAGS[i].flag; // 清除已处理的标志
            }
        }
        // 处理未知标志
        if (flags != 0)
        {
            if (count > 0) printf(" | ");
            printf("UNKNOWN(0x%X)", flags);
        }
    }
    printf("\n");
}


/**
 * 遍历输出Dex头信息
 * @param dex_header dex头信息
 */
void print_header(DexHeader dex_header)
{
    printf("/-----------------------------------------------\\\n");
    printf("|              DEX Header Info                |\n");
    printf("+-----------------------+-----------------------+\n");

    // Magic & Version
    char dex[4] = {0};
    char version[4] = {0};
    memcpy_s(dex, 3, &dex_header.magic, 3);
    memcpy_s(version, 3, &dex_header.magic[4], 3);
    printf("| %-22s | %-15s |\n", "Magic (dex):", dex);
    printf("| %-22s | %-15s |\n", "Magic (version):", version);
    printf("+-----------------------+-----------------------+\n");

    // Checksum & Signature
    printf("| %-22s | 0x%-13X |\n", "Checksum:", dex_header.checksum);
    printf("| %-22s | ", "Signature:");
    for (int i = 0; i < sizeof(dex_header.signature); i++)
    {
        printf("%02X", dex_header.signature[i]);
    }
    printf(" |\n");
    printf("+-----------------------+-----------------------+\n");

    // Sizes & Offsets - Column 1
    printf("| %-22s | %-15d |\n", "File Size:", dex_header.fileSize);
    printf("| %-22s | %-15d |\n", "Header Size:", dex_header.headerSize);
    printf("| %-22s | 0x%-13X |\n", "Endian Tag:", dex_header.endianTag);
    printf("| %-22s | %-15d |\n", "Link Size:", dex_header.linkSize);
    printf("| %-22s | %-15d |\n", "Link Offset:", dex_header.linkOff);
    printf("| %-22s | %-15d |\n", "Map Offset:", dex_header.mapOff);
    printf("+-----------------------+-----------------------+\n");

    // Sizes & Offsets - Column 2
    printf("| %-22s | %-15d |\n", "String IDs Size:", dex_header.stringIdsSize);
    printf("| %-22s | %-15d |\n", "String IDs Offset:", dex_header.stringIdsOff);
    printf("| %-22s | %-15d |\n", "Type IDs Size:", dex_header.typeIdsSize);
    printf("| %-22s | %-15d |\n", "Type IDs Offset:", dex_header.typeIdsOff);
    printf("| %-22s | %-15d |\n", "Proto IDs Size:", dex_header.protoIdsSize);
    printf("| %-22s | %-15d |\n", "Proto IDs Offset:", dex_header.protoIdsOff);
    printf("+-----------------------+-----------------------+\n");

    // Sizes & Offsets - Column 3
    printf("| %-22s | %-15d |\n", "Field IDs Size:", dex_header.fieldIdsSize);
    printf("| %-22s | %-15d |\n", "Field IDs Offset:", dex_header.fieldIdsOff);
    printf("| %-22s | %-15d |\n", "Method IDs Size:", dex_header.methodIdsSize);
    printf("| %-22s | %-15d |\n", "Method IDs Offset:", dex_header.methodIdsOff);
    printf("| %-22s | %-15d |\n", "Class Defs Size:", dex_header.classDefsSize);
    printf("| %-22s | %-15d |\n", "Class Defs Offset:", dex_header.classDefsOff);
    printf("+-----------------------+-----------------------+\n");

    // Data Section
    printf("| %-22s | %-15d |\n", "Data Size:", dex_header.dataSize);
    printf("| %-22s | %-15d |\n", "Data Offset:", dex_header.dataOff);
    printf("+-----------------------+-----------------------+\n");
}

/**
 * 遍历输出String信息
 * @param string_address 字符串地址
 */
void print_string(char* string_address)
{
    // 解析字符串长度
    const uint8_t* ptr = (const uint8_t*)string_address;
    uint32_t result = parse_uleb128(&ptr);
    // printf("string len: %d\n", result);

    // 准备一个缓冲区
    char* buffer = (char*)malloc(result + 1);
    memset(buffer, 0, result + 1);
    memcpy_s(buffer, result, string_address + 1, result);
    // printf("string:%s", buffer);
    printf("%-8s: %-4d | %-8s: %s\n",
           "Length", result,
           "Data", buffer);
}


void prit_proto()
{
}

/**
 * 遍历输出proto信息
 * @param buffer 文件缓冲区
 * @param dexHeader dex头信息
 * @param stringIDs_address 字符串地址
 * @param typeIDs_address 类型地址
 * @param protoIDs_address 原型地址
 */
void print_protoid(char* buffer, DexHeader dexHeader, uint32_t* stringIDs_address, uint32_t* typeIDs_address,
                   char*& protoIDs_address)
{
    for (int i = 0; i < dexHeader.protoIdsSize; i++)
    {
        DexprotoId dexproto_id{};
        memcpy_s(&dexproto_id, sizeof(dexproto_id), protoIDs_address, sizeof(DexprotoId));
        protoIDs_address += 12;

        char* shorty_idx = buffer + stringIDs_address[dexproto_id.shorty_idx];
        print_string(shorty_idx);

        char* return_type_idx = buffer + stringIDs_address[typeIDs_address[dexproto_id.return_type_idx]];
        print_string(return_type_idx);

        if (dexproto_id.parameters_off == 0)
        {
            printf("parameters_off == 0\n");
            continue;
        }
        uint32_t* parameters_off = (uint32_t*)(buffer + dexproto_id.parameters_off);
        DexTypeList dex_type_list{};
        memcpy_s(&dex_type_list, sizeof(dex_type_list), parameters_off, sizeof(DexTypeList));
        printf("size: %d ", dex_type_list.size);

        char* list_address = buffer + stringIDs_address[typeIDs_address[dex_type_list.list->typeIdx]];
        print_string(list_address);
    }
}


/**
 * 解析直接方法
 * @param dex_encode_method DexEncodeMethod结构
 */
void parse_method_info(char* dex_class_item, DexEncodeMethod* dex_encode_method)
{
    /**
    不能直接使用下标进行方法，因为这样会解析错误。
    char* dex_encode_item = dex_class_item;

    uint32_t method_idx_diff_result = parse_uleb128((const uint8_t*)(&dex_encode_item[0]));
    dex_encode_method->method_idx_diff = method_idx_diff_result;

    uint32_t access_flags_result = parse_uleb128((const uint8_t*)(&dex_encode_item[1]));
    dex_encode_method->access_flags = access_flags_result;

    uint32_t code_off_result = parse_uleb128((const uint8_t*)(&dex_encode_item[2]));
    dex_encode_method->code_off = code_off_result;
    */

    // 初始指针指向 EncodedMethod 起始位置
    const uint8_t* ptr = (const uint8_t*)dex_class_item;
    // 解析 method_idx_diff（自动推进指针）
    uint32_t method_idx_diff = parse_uleb128(&ptr);
    dex_encode_method->method_idx_diff = method_idx_diff;

    // 解析 access_flags（指针已由 parse_uleb128 自动推进）
    uint32_t access_flags = parse_uleb128(&ptr);
    dex_encode_method->access_flags = access_flags;

    // 解析 code_off（指针继续推进）
    uint32_t code_off = parse_uleb128(&ptr);
    dex_encode_method->code_off = code_off;
}

/**
 * 解析Code_item
 * @param dex_code_address Code_item的地址
 * @param dex_code Code结构体
 */
void parse_code_item(char* dex_code_address, DexCode* dex_code)
{
    char* temp = dex_code_address;
    dex_code->registers_size = *(uint8_t*)temp;
    dex_code->ins_size = *(uint8_t*)(temp + 2);
    dex_code->outs_size = *(uint8_t*)(temp + 4);
    dex_code->tries_size = *(uint8_t*)(temp + 6);
    dex_code->debug_info_off = *(uint32_t*)(temp + 8);
    dex_code->insns_size = *(uint32_t*)(temp + 12);

    // 开始解析insns
    auto* temp_insns = new uint16_t[dex_code->insns_size * sizeof(uint16_t)];
    memset(temp_insns, 0, dex_code->insns_size * sizeof(uint16_t));
    memcpy_s(temp_insns, dex_code->insns_size * sizeof(uint16_t), temp + 16, dex_code->insns_size * sizeof(uint16_t));

    printf("insns:");;
    for (int i = 0; i < dex_code->insns_size; ++i)
    {
        printf("%04x ", temp_insns[i]);
    }

    printf("\n");
}

/**
 * 输出Code_item信息
 * @param dex_code dex_code结构体
 */
VOID print_code_item(const DexCode* dex_code)
{
    if (dex_code == nullptr)
    {
        printf("DexCode is null\n");
        return;
    }

    printf("|-------------------------------|\n");
    printf("|          Code Info            |\n");
    printf("|-------------------------------|\n");

    printf("| Registers Size : %-13d|\n", dex_code->registers_size);
    printf("| Ins Size : %-19d|\n", dex_code->ins_size);
    printf("| Outs Size : %-18d|\n", dex_code->outs_size);
    printf("| Tries Size : %-17d|\n", dex_code->tries_size);
    printf("| DebugInfo OffSet : %-11d|\n", dex_code->debug_info_off);
    printf("| Insns Size : %-17d|\n", dex_code->insns_size);
    printf("|-------------------------------|\n");
}

/**
 * 解析Debug_Info
 * @param dex_code_address Debug_Info的地址
 * @param dex_debug_info DebugInfo结构体
 */
void parse_Debug_Info(char* dex_debug_info_address, DexDebugInfo* dex_debug_info)
{
    const uint8_t* ptr = (const uint8_t*)dex_debug_info_address;

    uint32_t temp_list_start = parse_uleb128(&ptr);
    dex_debug_info->line_start = temp_list_start;

    uint32_t temp_parameters_size = parse_uleb128(&ptr);
    dex_debug_info->parameters_size = temp_parameters_size;

    if (temp_parameters_size != 0)
    {
        uint32_t temp_parameter_names = parse_uleb128(&ptr);
        dex_debug_info->parameter_names = temp_parameter_names;
    }
    else
    {
        dex_debug_info->parameter_names = -1;
    }

    int32_t line = dex_debug_info->line_start; // 从 debug_info_item 中读取
    uint32_t address = 0;

    while (*ptr != 0x00)
    {
        uint8_t opcode = *ptr++;
        if (opcode == 0x00) break;

        switch (opcode)
        {
        case DBG_ADVANCE_PC:
            address += parse_uleb128(&ptr);
            break;
        case DBG_ADVANCE_LINE:
            line += parse_sleb128(&ptr);
            break;
        case DBG_START_LOCAL:
            // 解析局部变量信息...
            break;
        // 更多 case ...
        default:
            if (opcode >= DBG_FIRST_SPECIAL)
            {
                int adjusted_opcode = opcode - DBG_FIRST_SPECIAL;
                line += DBG_LINE_BASE + (adjusted_opcode % DBG_LINE_RANGE);
                address += adjusted_opcode / DBG_LINE_RANGE;
            }
            else
            {
                // 其他合法但未处理的 opcode
            }
            break;
        }
        printf("line: %d\n", line);
        printf("address: %d\n", address);
    }
}

VOID printf_Debug_Info(const DexDebugInfo* dex_debug_info)
{
    printf("|-------------------------------|\n");
    printf("|          Debug Info           |\n");
    printf("|-------------------------------|\n");
    printf("| Line Start : %-17d|\n", dex_debug_info->line_start);
    printf("| Parameters Size : %-12d|\n", dex_debug_info->parameters_size);
    printf("| Parameter Names : %-12d|\n", dex_debug_info->parameter_names);
    printf("|-------------------------------|\n");
}


/**
 * class_data_item 出现在 class_defs 中。
 * class_defs的 class_data_off 字段指向 class_data_item 结构。
 * class_data_item 结构体包含了静态字段、实例字段、直接方法和虚拟方法的数量。
 * 静态字段和实例字段都属于encoded_field格式。
 * 直接方法和虚拟方法都属于encoded_method格式。
 *
 * @param buffer DEX文件数据缓冲区
 * @param dex_class_address class_data_item的地址
 * @param dex_class_data ClassData结构体
 */
void class_data_item(char* buffer, char* dex_class_address, DexClassData* dex_class_data)
{
    // 解析类数据项
    const uint8_t* ptr = (const uint8_t*)dex_class_address;
    // 解析静态字段数量
    uint32_t static_fields_size = parse_uleb128(&ptr);
    dex_class_data->static_fields_size = static_fields_size;
    printf("static_field_size: %d\n", static_fields_size);

    // 解析实例字段数量
    uint32_t instance_fields_size = parse_uleb128(&ptr);
    dex_class_data->instance_fields_size = instance_fields_size;
    printf("instance_fields_size: %d\n", instance_fields_size);

    // 解析直接方法数量
    uint32_t direct_methods_size = parse_uleb128(&ptr);
    dex_class_data->direct_methods_size = direct_methods_size;
    printf("direct_methods_size: %d\n", direct_methods_size);

    // 解析直接方法，由 static 和 private 修饰的方法就是直接方法。
    if (direct_methods_size != 0)
    {
        dex_class_address += 4; // 前4个字节是各个字段的数量。
        for (int i = 0; i < direct_methods_size; ++i)
        {
            DexEncodeMethod dex_encode_method{};
            // 解析Code_item
            parse_method_info(dex_class_address, &dex_encode_method);
            printf("method_idx_diff: %d\n", dex_encode_method.method_idx_diff);
            print_access_flags(dex_encode_method.access_flags);
            printf("code_off: %x\n", dex_encode_method.code_off);

            // 解析Code_item
            DexCode dex_code{};
            char* dex_code_address = buffer + dex_encode_method.code_off;

            parse_code_item(dex_code_address, &dex_code);
            print_code_item(&dex_code);
            // 解析Debug_Info
            if (dex_code.debug_info_off != 0)
            {
                DexDebugInfo dex_debug_info{};
                char* dex_debug_info_address = buffer + dex_code.debug_info_off;
                parse_Debug_Info(dex_debug_info_address, &dex_debug_info);
                printf_Debug_Info(&dex_debug_info);
            }


            dex_class_address += 0x6;
        }
    }

    // 解析虚拟方法数量
    uint32_t virtual_methods_size = parse_uleb128(&ptr);
    dex_class_data->virtual_methods_size = virtual_methods_size;
    printf("virtual_methods_size: %d\n", virtual_methods_size);

    if (static_fields_size != 0)
    {
    }
    if (instance_fields_size != 0)
    {
    }


    if (virtual_methods_size != 0)
    {
    }
}

void parse_map_item(const char* map_list_address, DexMapList* dex_map_list, DexMapItem* dex_map_item)
{
    const char* ptr = map_list_address + sizeof(dex_map_list->size);
    for (int i = 0; i < dex_map_list->size; ++i)
    {
        dex_map_item[i].type = *(uint16_t*)ptr;
        dex_map_item[i].unused = *(uint16_t*)(ptr + sizeof(dex_map_item->unused));
        dex_map_item[i].size = *(uint32_t*)(ptr + sizeof(dex_map_item->size));
        dex_map_item[i].offset = *(uint32_t*)(ptr + sizeof(dex_map_item->offset) * 2);
        ptr += sizeof(DexMapItem);
    }
}

void printf_map_item(DexMapList* dex_map_list, DexMapItem* dex_map_item)
{
    for (int i = 0; i < dex_map_list->size; ++i)
    {
        printf("|----------------------map item---------------------|\n");
        printf("Map Item %d\n", i);
        printf("Type: %d\n", dex_map_item[i].type);
        printf("unused: %d\n", dex_map_item[i].unused);
        printf("size: %d\n", dex_map_item[i].size);
        printf("Offset: %d\n", dex_map_item[i].offset);
    }
    printf("|----------------------map item---------------------|\n");
}

void parse_map_list(const char* map_list_address, DexMapList* dex_map_list, DexMapItem* dex_map_item)
{
    if (map_list_address == nullptr)
    {
        printf("map_list_address is null\n");
        return;
    }
    dex_map_list->size = *(uint32_t*)map_list_address;
    auto* temp_dex_map_item = new DexMapItem[sizeof(dex_map_item) * dex_map_list->size];
    parse_map_item(map_list_address, dex_map_list, temp_dex_map_item);
    printf_map_item(dex_map_list, temp_dex_map_item);
}

int main()
{
    HANDLE hFile = CreateFile(
        R"(D:\ProjectALL\CLionProjects\DexDump\resource\classes.dex)",
        GENERIC_READ,
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        printf("CreateFile failed\n");
        return -1;
    }

    const DWORD file_size = GetFileSize(hFile, nullptr);
    printf("file_size=%d\n", file_size);

    // 读取文件信息到缓冲区
    char* buffer = (char*)(malloc(file_size));
    DWORD bytesRead = 0;
    if (!ReadFile(hFile, buffer, file_size, &bytesRead, nullptr))
    {
        printf("ReadFile failed\n");
        return -1;
    }
    printf("ReadFile size:%d\n", bytesRead);


    // Dex头信息
    DexHeader dexHeader{};
    printf("DexHeader size: %llu\n", sizeof(DexHeader));
    memcpy_s(&dexHeader, sizeof(dexHeader), buffer, sizeof(DexHeader));
    print_header(dexHeader);


    /**
     * Dex字符串信息，有两种方式可以定位到String IDS，
     * 一种是直接让文件头加上Header的大小就可以以定位到字符串IDS
     * 另一种方式是文件头加上String IDS Offset也可以定位到字符串IDS
     *
     * 而String IDS保存的是每个字符串相对与文件头的偏移string_data_off。指向string_data_item结构。
     * string_data_item有两个属性，分别是 utf16_size 和数据，utf16_size说明了该字符串的大小，是一个uleb128类型。
     */
    printf("********************String********************\n");
    uint32_t* stringIDs_address = (uint32_t*)(buffer + dexHeader.stringIdsOff);
    for (int i = 0; i < dexHeader.stringIdsSize; i++)
    {
        char* string_address = buffer + stringIDs_address[i];
        print_string(string_address);
    }
    printf("********************String********************\n");


    /**
     *
     * dex type 类型描述符
     * type_ids_off	类型描述符在文件中的偏移
     * type_ids_size 类型描述符大小
     * type_id_item 保存了每个类型描述符的字符串索引。
     */
    printf("********************Type********************\n");
    uint32_t* typeIDs_address = (uint32_t*)(buffer + dexHeader.typeIdsOff);
    for (int i = 0; i < dexHeader.typeIdsSize; i++)
    {
        char* type_address = buffer + stringIDs_address[typeIDs_address[i]];
        print_string(type_address);
    }
    printf("********************Type********************\n");

    /**
     * proto_ids 原型标识符保存了每个proto_ids_item的地址
     * proto_ids_off 在文件中的偏移
     * proto_ids_size 原型标识符数量
     * proto_ids_item 结构体包含了返回值类型、参数类型和参数个数
     *
     */
    char* protoIDs_address = (char*)(buffer + dexHeader.protoIdsOff);

    char** proto_id_table = (char**)malloc(dexHeader.protoIdsSize * sizeof(char*));
    for (int i = 0; i < dexHeader.protoIdsSize; i++)
    {
        proto_id_table[i] = protoIDs_address + i * sizeof(DexprotoId);
    }


    print_protoid(buffer, dexHeader, stringIDs_address, typeIDs_address, protoIDs_address);

    /**
     *
     * field ids
     * field ids offset 文件中的偏移
     * field ids szie 字段数量
     *
     * 其中 field_id_item 结构体包含了类索引、类型索引和名称索引，而field ids保存的就是field_id_item结构的地址
     */
    char* field_ids_address = (char*)(buffer + dexHeader.fieldIdsOff);
    for (int i = 0; i < dexHeader.fieldIdsSize; i++)
    {
        DexFieldId dex_field_id{};
        memcpy_s(&dex_field_id, sizeof(dex_field_id), field_ids_address, sizeof(DexFieldId));
        field_ids_address += 8;

        char* classIdx = buffer + stringIDs_address[typeIDs_address[dex_field_id.classIdx]];
        print_string(classIdx);

        char* typeIdx = buffer + stringIDs_address[typeIDs_address[dex_field_id.typeIdx]];
        print_string(typeIdx);

        char* nameIdx = buffer + stringIDs_address[dex_field_id.nameIdx];
        print_string(nameIdx);
    }


    /**
     *
     * dex method ids
     * method ids offset 文件中的偏移
     * method ids size  方法的数量
     */
    char* method_ids_address = (char*)(buffer + dexHeader.methodIdsOff);
    for (int i = 0; i < dexHeader.methodIdsSize; i++)
    {
        DexMethodId dex_method_id{};
        memcpy_s(&dex_method_id, sizeof(dex_method_id), method_ids_address, sizeof(DexMethodId));
        method_ids_address += 8;
        char* classIdx = buffer + stringIDs_address[typeIDs_address[dex_method_id.classIdx]];
        print_string(classIdx);

        char* protoIdx = buffer + stringIDs_address[protoIDs_address[dex_method_id.protoIdx]];
        // print_string(protoIdx);
        // printf("%d\n", dex_method_id.protoIdx);
        DexprotoId id{};
        memcpy_s(&id, sizeof(DexprotoId), proto_id_table[dex_method_id.protoIdx], sizeof(DexprotoId));
        // print_protoid(buffer, dexHeader, stringIDs_address, typeIDs_address, protoIDs_address);

        char* nameIdx = buffer + stringIDs_address[dex_method_id.nameIdx];
        print_string(nameIdx);
    }

    /**
     *
     *
     * class def
     *  class def offset 在文件中的偏移
     *  class  def size 类的数量
     *  class def 保存了class def item的地址，class def item是个结构体
     *
     */
    printf("********************DexClassDef********************\n");
    uint32_t* class_defs_address = (uint32_t*)(buffer + dexHeader.classDefsOff);
    for (int i = 0; i < dexHeader.classDefsSize; i++)
    {
        DexClassDef dex_class_def{};
        memcpy_s(&dex_class_def, sizeof(dex_class_def), class_defs_address, sizeof(DexClassDef));
        class_defs_address += sizeof(DexClassDef);

        char* classIdx = (char*)(buffer + stringIDs_address[typeIDs_address[dex_class_def.classIdx]]);
        print_string(classIdx);

        uint32_t accessFlags = dex_class_def.accessFlags;
        print_access_flags(accessFlags);

        char* superclassIdx = buffer + stringIDs_address[typeIDs_address[dex_class_def.superclassIdx]];
        print_string(superclassIdx);

        if (dex_class_def.interfacesOff == 0)
        {
            printf("interfacesOff is null\n");
        }
        char* interfacesOff = buffer + dex_class_def.interfacesOff;

        char* sourceFileIdx = buffer + stringIDs_address[dex_class_def.sourceFileIdx];
        print_string(sourceFileIdx);

        if (dex_class_def.annotationsOff == 0)
        {
            printf("annotationsOff is null\n");
        }

        char* annotationsOff = buffer + dex_class_def.annotationsOff;

        // 关联类数据
        if (dex_class_def.classDataOff == 0)
        {
            printf("classDataOff is null\n");
        }
        char* classDataOff = buffer + dex_class_def.classDataOff;
        DexClassData dex_class_data{};
        // 解析关联类数据
        class_data_item(buffer, classDataOff, &dex_class_data);

        if (dex_class_def.staticValuesOff == 0)
        {
            printf("static ValuesOff is null\n");
        }
        char* staticValuesOff = buffer + dex_class_def.staticValuesOff;
    }


    /**
     *
     *  map list
     *
     */
    char* map_list_address = (char*)(buffer + dexHeader.mapOff);
    DexMapList dex_map_list{};
    DexMapItem dex_map_item{};

    parse_map_list(map_list_address, &dex_map_list, &dex_map_item);


    free(proto_id_table);
    CloseHandle(hFile);

    return 0;
}
