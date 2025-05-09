//
// Created by GaGa on 25-5-13.
//

#ifndef METHODPRINT_H
#define METHODPRINT_H

#include "BasePrint.h"

namespace dex::print
{
    /**
     * Method表格式化输出类
     */
    class MethodPrint final : public BasePrint
    {
    public:
        /**
         * 构造函数
         */
        MethodPrint() = default;
        
        /**
         * 析构函数
         */
        ~MethodPrint() override = default;
        
        /**
         * 打印Method表
         */
        void print() override;
    };
}

#endif //METHODPRINT_H 