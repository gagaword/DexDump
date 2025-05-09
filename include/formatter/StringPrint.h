//
// Created by GaGa on 25-5-10.
//

#ifndef STRINGPRINT_H
#define STRINGPRINT_H

#include "BasePrint.h"

namespace dex::print
{
    /**
     * 字符串表格式化输出类
     */
    class StringPrint final : public BasePrint
    {
    public:
        /**
         * 构造函数
         */
        StringPrint() = default;
        
        /**
         * 析构函数
         */
        ~StringPrint() override = default;
        
        /**
         * 打印字符串表
         */
        void print() override;
    };
}

#endif //STRINGPRINT_H 