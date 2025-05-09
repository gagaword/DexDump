//
// Created by DexDump on 2025-5-15.
//

#include "CodeParser.h"
#include "log/log.h"
#include <sstream>
#include <iomanip>

#include "core/DexContext.h"

namespace dex::parser
{
    // Dalvik指令集格式标志
    enum DalvikFormatFlag
    {
        kFmt00x = 0,    // 无操作数
        kFmt10x,        // 单字节操作码
        kFmt12x,        // 寄存器对
        kFmt11n,        // 寄存器和常量4位
        kFmt11x,        // 单寄存器
        kFmt10t,        // 10位偏移量
        kFmt20t,        // 20位偏移量
        kFmt20bc,       // 字段或方法引用
        kFmt22x,        // 16位寄存器引用
        kFmt21t,        // 带8位寄存器的21位偏移量
        kFmt21s,        // 带8位寄存器的16位常量
        kFmt21h,        // 带8位寄存器的高16位常量
        kFmt21c,        // 8位寄存器和索引
        kFmt23x,        // 三个8位寄存器
        kFmt22b,        // 两个8位寄存器和一个8位常量
        kFmt22t,        // 两个4位寄存器和一个16位偏移量
        kFmt22s,        // 两个4位寄存器和一个16位常量
        kFmt22c,        // 两个4位寄存器和一个16位常量索引
        kFmt22cs,       // 快速实例字段访问
        kFmt30t,        // 30位偏移量
        kFmt32x,        // 两个16位寄存器
        kFmt31i,        // 8位寄存器和32位常量
        kFmt31t,        // 8位寄存器和32位偏移量
        kFmt31c,        // 8位寄存器和运行时常量索引
        kFmt35c,        // 3-5个寄存器和类/方法/字段索引
        kFmt35ms,       // 类似35c但用于快速方法调用
        kFmt3rc,        // 范围调用 (N个连续寄存器)
        kFmt3rms,       // 类似3rc但用于快速方法调用
        kFmt51l,        // 8位寄存器和64位常量
        kFmtUnknown,    // 未知格式
    };

    // 操作码和助记符映射
    struct OpcodeMapEntry
    {
        uint16_t opcode;
        const char* mnemonic;
        DalvikFormatFlag format;
    };

    // Dalvik指令集操作码映射表
    static const OpcodeMapEntry gOpcodeMap[] = {
        {0x00, "nop", kFmt10x},
        {0x01, "move", kFmt12x},
        {0x02, "move/from16", kFmt22x},
        {0x03, "move/16", kFmt32x},
        {0x04, "move-wide", kFmt12x},
        {0x05, "move-wide/from16", kFmt22x},
        {0x06, "move-wide/16", kFmt32x},
        {0x07, "move-object", kFmt12x},
        {0x08, "move-object/from16", kFmt22x},
        {0x09, "move-object/16", kFmt32x},
        {0x0a, "move-result", kFmt11x},
        {0x0b, "move-result-wide", kFmt11x},
        {0x0c, "move-result-object", kFmt11x},
        {0x0d, "move-exception", kFmt11x},
        {0x0e, "return-void", kFmt10x},
        {0x0f, "return", kFmt11x},
        {0x10, "return-wide", kFmt11x},
        {0x11, "return-object", kFmt11x},
        {0x12, "const/4", kFmt11n},
        {0x13, "const/16", kFmt21s},
        {0x14, "const", kFmt31i},
        {0x15, "const/high16", kFmt21h},
        {0x16, "const-wide/16", kFmt21s},
        {0x17, "const-wide/32", kFmt31i},
        {0x18, "const-wide", kFmt51l},
        {0x19, "const-wide/high16", kFmt21h},
        {0x1a, "const-string", kFmt21c},
        {0x1b, "const-string/jumbo", kFmt31c},
        {0x1c, "const-class", kFmt21c},
        {0x1d, "monitor-enter", kFmt11x},
        {0x1e, "monitor-exit", kFmt11x},
        {0x1f, "check-cast", kFmt21c},
        {0x20, "instance-of", kFmt22c},
        {0x21, "array-length", kFmt12x},
        {0x22, "new-instance", kFmt21c},
        {0x23, "new-array", kFmt22c},
        {0x24, "filled-new-array", kFmt35c},
        {0x25, "filled-new-array/range", kFmt3rc},
        {0x26, "fill-array-data", kFmt31t},
        {0x27, "throw", kFmt11x},
        {0x28, "goto", kFmt10t},
        {0x29, "goto/16", kFmt20t},
        {0x2a, "goto/32", kFmt30t},
        {0x2b, "packed-switch", kFmt31t},
        {0x2c, "sparse-switch", kFmt31t},
        // add-int/sub-int/mul-int等基本算术操作已省略...
        {0xfe, "invoke-polymorphic", kFmt35c},
        {0xff, "invoke-polymorphic/range", kFmt3rc},
    };

    // 操作码到格式的映射
    static DalvikFormatFlag getOpcodeFormat(uint16_t opcode)
    {
        for (const auto& entry : gOpcodeMap)
        {
            if (entry.opcode == opcode)
            {
                return entry.format;
            }
        }
        return kFmtUnknown;
    }

    CodeParser::CodeParser(const uint8_t* fileData, size_t fileSize)
        : BaseParser(fileData, fileSize, &DexContext::getInstance().getHeader())
    {
        if (fileData == nullptr || fileSize == 0)
        {
            LOGE("文件数据为空");
            return;
        }
    }

    bool CodeParser::parse()
    {
        // 这个方法通常不会被直接调用，而是通过parseCode方法解析特定的代码段
        return true;
    }

    CodeSectionInfo CodeParser::parseCode(uint32_t codeOffset)
    {
        CodeSectionInfo codeInfo = {};
        codeInfo.codeOffset = codeOffset;
        
        // 检查偏移量是否有效
        if (codeOffset == 0 || codeOffset >= BaseFileSize_)
        {
            LOGE("代码偏移量无效: 0x%08X", codeOffset);
            return codeInfo;
        }
        
        // 获取DexCode结构
        const DexCode* dexCode = reinterpret_cast<const DexCode*>(BaseFileData_ + codeOffset);
        
        // 填充代码段信息
        codeInfo.registersSize = dexCode->registers_size;
        codeInfo.insSize = dexCode->ins_size;
        codeInfo.outsSize = dexCode->outs_size;
        codeInfo.triesSize = dexCode->tries_size;
        codeInfo.debugInfoOff = dexCode->debug_info_off;
        codeInfo.insnsSize = dexCode->insns_size;
        
        // 检查指令数组是否有效
        if (codeInfo.insnsSize > 0)
        {
            // 解析指令
            codeInfo.instructions = parseInstructions(codeOffset, dexCode->insns, codeInfo.insnsSize);
        }
        
        return codeInfo;
    }

    std::vector<InstructionInfo> CodeParser::parseInstructions(uint32_t codeOffset, const uint16_t* insns, uint32_t insnsSize)
    {
        std::vector<InstructionInfo> instructions;
        
        // 计算指令数组起始偏移量
        uint32_t insnsOffset = codeOffset + offsetof(DexCode, insns);
        
        // 遍历指令数组
        uint32_t offset = 0;
        while (offset < insnsSize)
        {
            // 获取操作码
            uint16_t opcode = insns[offset] & 0xFF;
            
            // 创建指令信息
            InstructionInfo insInfo = {};
            insInfo.opcode = opcode;
            insInfo.offset = offset;
            insInfo.mnemonic = getOpcodeMnemonic(opcode);
            
            // 获取指令长度
            insInfo.length = getInstructionLength(opcode);
            
            // 解析操作数
            insInfo.operands = parseOperands(opcode, insns, offset);
            
            // 添加到指令列表
            instructions.push_back(insInfo);
            
            // 更新偏移量
            offset += insInfo.length;
        }
        
        return instructions;
    }

    std::string CodeParser::getOpcodeMnemonic(uint16_t opcode)
    {
        for (const auto& entry : gOpcodeMap)
        {
            if (entry.opcode == opcode)
            {
                return entry.mnemonic;
            }
        }
        return "unknown";
    }

    std::string CodeParser::parseOperands(uint16_t opcode, const uint16_t* insns, uint32_t offset)
    {
        std::stringstream ss;
        DalvikFormatFlag format = getOpcodeFormat(opcode);
        
        uint16_t instr = insns[offset];
        
        switch (format)
        {
            case kFmt10x: // 单字节操作码，无操作数
                break;
                
            case kFmt12x: // 寄存器对
            {
                uint8_t vA = (instr >> 8) & 0xF;
                uint8_t vB = (instr >> 12) & 0xF;
                ss << "v" << static_cast<int>(vA) << ", v" << static_cast<int>(vB);
                break;
            }
                
            case kFmt11n: // 寄存器和常量4位
            {
                uint8_t vA = (instr >> 8) & 0xF;
                int8_t val = (instr >> 12) & 0xF;
                ss << "v" << static_cast<int>(vA) << ", #" << static_cast<int>(val);
                break;
            }
                
            case kFmt11x: // 单寄存器
            {
                uint8_t vA = (instr >> 8) & 0xFF;
                ss << "v" << static_cast<int>(vA);
                break;
            }
                
            case kFmt10t: // 10位偏移量
            {
                int8_t offset = static_cast<int8_t>((instr >> 8) & 0xFF);
                ss << "+" << static_cast<int>(offset);
                break;
            }
                
            // 支持更多指令格式... (为简洁起见这里省略了部分实现)
                
            default:
                ss << "格式" << format << "(未详细解析)";
                break;
        }
        
        return ss.str();
    }

    uint32_t CodeParser::getInstructionLength(uint16_t opcode)
    {
        DalvikFormatFlag format = getOpcodeFormat(opcode);
        
        switch (format)
        {
            case kFmt10x:
            case kFmt12x:
            case kFmt11n:
            case kFmt11x:
            case kFmt10t:
                return 1; // 1个16位字
                
            case kFmt20t:
            case kFmt22x:
            case kFmt21t:
            case kFmt21s:
            case kFmt21h:
            case kFmt21c:
            case kFmt23x:
            case kFmt22b:
            case kFmt22t:
            case kFmt22s:
            case kFmt22c:
                return 2; // 2个16位字
                
            case kFmt30t:
            case kFmt32x:
            case kFmt31i:
            case kFmt31t:
            case kFmt31c:
            case kFmt35c:
            case kFmt3rc:
                return 3; // 3个16位字
                
            case kFmt51l:
                return 5; // 5个16位字
                
            default:
                return 1; // 默认值
        }
    }
} 