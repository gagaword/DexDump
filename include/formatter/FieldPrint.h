//
// Created by GaGa on 25-5-12.
//

#ifndef FIELDPRINT_H
#define FIELDPRINT_H

#include "BasePrint.h"

namespace dex::print
{
    /**
     * Field表格式化输出类
     */
    class FieldPrint final : public BasePrint
    {
    public:
        /**
         * 构造函数
         */
        FieldPrint() = default;
        
        /**
         * 析构函数
         */
        ~FieldPrint() override = default;
        
        /**
         * 打印Field表
         */
        void print() override;
    };
}

#endif //FIELDPRINT_H 