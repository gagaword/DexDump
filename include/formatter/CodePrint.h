//
// Created by DexDump on 2025-5-15.
//

#ifndef CODEPRINT_H
#define CODEPRINT_H

#include "BasePrint.h"
#include "parser/CodeParser.h"

namespace dex::print
{
    /**
     * 代码格式化输出类
     * 负责解析和显示DEX文件中的方法代码
     */
    class CodePrint final : public BasePrint
    {
    public:
        /**
         * 构造函数
         */
        CodePrint() = default;
        
        /**
         * 析构函数
         */
        ~CodePrint() override = default;
        
        /**
         * 打印指定方法的代码
         * @param methodIdx 方法索引
         */
        void printMethodCode(uint32_t methodIdx);
        
        /**
         * 打印指定代码偏移量的代码
         * @param codeOffset 代码偏移量
         */
        void printCode(uint32_t codeOffset);
        
        /**
         * 实现基类的打印接口（打印所有方法的代码）
         */
        void print() override;
        
    private:
        /**
         * 打印特定指令的详细信息
         * @param instruction 指令信息
         * @param baseOffset 基础偏移量（用于计算绝对地址）
         */
        void printInstruction(const dex::parser::InstructionInfo& instruction, uint32_t baseOffset);
        
        /**
         * 获取指令的操作数描述
         * @param opcode 操作码
         * @param operands 操作数原始字符串
         * @return 格式化后的操作数描述
         */
        std::string getOperandDescription(uint16_t opcode, const std::string& operands);
    };
}

#endif // CODEPRINT_H 