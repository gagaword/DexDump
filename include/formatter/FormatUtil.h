//
// Created by DexDump on 2025-5-15.
//

#ifndef FORMATUTIL_H
#define FORMATUTIL_H

#include <string>
#include <sstream>
#include "core/DexContext.h"

namespace dex::print
{
    /**
     * 简化类型名称显示（只显示最后的部分）
     * @param typeName 原始类型名称
     * @return 简化后的类型名称
     */
    std::string simplifyTypeName(const std::string& typeName);
    
    /**
     * 生成方法签名字符串
     * @param methodInfo 方法信息
     * @return 格式化的方法签名
     */
    std::string formatMethodSignature(const dex::MethodInfo& methodInfo);
}

#endif // FORMATUTIL_H 