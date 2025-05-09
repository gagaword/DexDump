//
// Created by DexDump on 2025-5-15.
//

#ifndef CODEPARSER_H
#define CODEPARSER_H

#include "BaseParser.h"
#include <vector>

namespace dex::parser
{
    /**
     * 指令信息结构
     */
    struct InstructionInfo
    {
        uint16_t opcode;         // 操作码
        uint32_t offset;         // 指令偏移量
        uint32_t length;         // 指令长度(16位字的数量)
        std::string mnemonic;    // 指令助记符
        std::string operands;    // 操作数字符串
    };
    
    /**
     * 代码区段信息结构
     */
    struct CodeSectionInfo
    {
        uint32_t codeOffset;        // 代码偏移量
        uint16_t registersSize;     // 寄存器数量
        uint16_t insSize;           // 输入参数数量
        uint16_t outsSize;          // 输出参数数量
        uint16_t triesSize;         // try/catch块数量
        uint32_t debugInfoOff;      // 调试信息偏移量
        uint32_t insnsSize;         // 指令数量(16位字的数量)
        std::vector<InstructionInfo> instructions; // 指令信息列表
    };

    /**
     * 代码解析器
     * 负责解析DEX文件中的方法代码
     */
    class CodeParser final : public BaseParser
    {
    public:
        /**
         * 构造函数
         * @param fileData 文件数据
         * @param fileSize 文件大小
         */
        CodeParser(const uint8_t* fileData, size_t fileSize);
        
        /**
         * 析构函数
         */
        ~CodeParser() override = default;
        
        /**
         * 解析代码段
         * @param codeOffset 代码偏移量
         * @return 代码段信息
         */
        CodeSectionInfo parseCode(uint32_t codeOffset);
        
        /**
         * 实现解析函数
         * @return 解析是否成功
         */
        bool parse() override;
        
    private:
        /**
         * 解析指令
         * @param codeOffset 代码偏移量
         * @param insns 指令数组
         * @param insnsSize 指令数量
         * @return 指令信息列表
         */
        std::vector<InstructionInfo> parseInstructions(uint32_t codeOffset, const uint16_t* insns, uint32_t insnsSize);
        
        /**
         * 获取操作码的助记符
         * @param opcode 操作码
         * @return 助记符
         */
        std::string getOpcodeMnemonic(uint16_t opcode);
        
        /**
         * 解析操作数
         * @param opcode 操作码
         * @param insns 指令数组
         * @param offset 当前偏移
         * @return 操作数字符串
         */
        std::string parseOperands(uint16_t opcode, const uint16_t* insns, uint32_t offset);
        
        /**
         * 获取指令长度
         * @param opcode 操作码
         * @return 指令长度(16位字的数量)
         */
        uint32_t getInstructionLength(uint16_t opcode);
    };
}

#endif // CODEPARSER_H 