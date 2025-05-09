//
// Created by GaGa on 25-5-14.
//

#ifndef CLASSPRINT_H
#define CLASSPRINT_H

#include "BasePrint.h"

namespace dex::print
{
    /**
     * Class表格式化输出类
     */
    class ClassPrint final : public BasePrint
    {
    public:
        /**
         * 构造函数
         */
        ClassPrint() = default;
        
        /**
         * 析构函数
         */
        ~ClassPrint() override = default;
        
        /**
         * 打印Class表
         */
        void print() override;
    };
}

#endif //CLASSPRINT_H 