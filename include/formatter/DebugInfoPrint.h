 //
// Created by DexDump on 2025-5-16.
//

#ifndef DEBUGINFOPRINT_H
#define DEBUGINFOPRINT_H

#include "BasePrint.h"

namespace dex::print
{
    /**
     * 调试信息格式化输出类
     * 负责显示DEX文件中的方法调试信息
     */
    class DebugInfoPrint final : public BasePrint
    {
    public:
        /**
         * 构造函数
         */
        DebugInfoPrint() = default;

        /**
         * 析构函数
         */
        ~DebugInfoPrint() override = default;

        /**
         * 打印指定方法的调试信息
         * @param methodIdx 方法索引
         */
        void printMethodDebugInfo(uint32_t methodIdx);

        /**
         * 打印指定调试信息偏移量的调试信息
         * @param debugInfoOff 调试信息偏移量
         */
        void printDebugInfo(uint32_t debugInfoOff);

        /**
         * 实现基类的打印接口（打印所有方法的调试信息）
         */
        void print() override;

    private:
        /**
         * 打印局部变量信息
         * @param localVars 局部变量列表
         */
        void printLocalVariables(const std::vector<dex::LocalVarInfo>& localVars);

        /**
         * 打印位置信息
         * @param positions 位置列表
         */
        void printPositions(const std::vector<dex::PositionInfo>& positions);

        /**
         * 打印Try/Catch信息
         * @param tries Try块列表
         */
        void printTryCatchBlocks(const std::vector<dex::TryBlockInfo>& tries);
    };
}

#endif // DEBUGINFOPRINT_H