//
// Created by GaGa on 25-5-11.
//

#ifndef PROTOPRINT_H
#define PROTOPRINT_H

#include "BasePrint.h"

namespace dex::print
{
    /**
     * Proto表格式化输出类
     */
    class ProtoPrint final : public BasePrint
    {
    public:
        /**
         * 构造函数
         */
        ProtoPrint() = default;
        
        /**
         * 析构函数
         */
        ~ProtoPrint() override = default;
        
        /**
         * 打印Proto表
         */
        void print() override;
    };
}

#endif //PROTOPRINT_H 