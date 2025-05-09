//
// Created by GaGa on 25-5-8.
//

#ifndef BASEPRINT_H
#define BASEPRINT_H

#include "core/DexContext.h"

namespace dex::print
{
    /**
     * 格式化器基类
     * 提供基本的格式化功能和接口
     */
    class BasePrint
    {
    protected:
        // 获取全局上下文
        static DexContext& getContext();
        
    public:
        // 构造函数
        BasePrint() = default;

        // 析构函数
        virtual ~BasePrint() = default;

        // 打印函数，子类必须实现
        virtual void print() = 0;
    };
}


#endif //BASEPRINT_H
