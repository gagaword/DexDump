//
// Created by DexDump on 2025-5-15.
//

#include "FormatUtil.h"

namespace dex::print
{
    // 简化类型名称显示（只显示最后的部分）
    std::string simplifyTypeName(const std::string& typeName)
    {
        std::string result = typeName;
        
        // 处理数组类型
        bool isArray = false;
        int arrayDimensions = 0;
        while (result.length() > 0 && result[0] == '[')
        {
            isArray = true;
            arrayDimensions++;
            result = result.substr(1);
        }
        
        // 处理基本类型
        if (result == "V") return isArray ? std::string(arrayDimensions, '[') + "void" : "void";
        if (result == "Z") return isArray ? std::string(arrayDimensions, '[') + "boolean" : "boolean";
        if (result == "B") return isArray ? std::string(arrayDimensions, '[') + "byte" : "byte";
        if (result == "S") return isArray ? std::string(arrayDimensions, '[') + "short" : "short";
        if (result == "C") return isArray ? std::string(arrayDimensions, '[') + "char" : "char";
        if (result == "I") return isArray ? std::string(arrayDimensions, '[') + "int" : "int";
        if (result == "J") return isArray ? std::string(arrayDimensions, '[') + "long" : "long";
        if (result == "F") return isArray ? std::string(arrayDimensions, '[') + "float" : "float";
        if (result == "D") return isArray ? std::string(arrayDimensions, '[') + "double" : "double";
        
        // 处理对象类型
        size_t pos = result.find_last_of('/');
        if (pos != std::string::npos && pos < result.length() - 1)
        {
            result = result.substr(pos + 1);
        }
        
        // 移除前缀L和后缀;（如果存在）
        if (result.length() > 2 && result[0] == 'L' && result[result.length() - 1] == ';')
        {
            result = result.substr(1, result.length() - 2);
        }
        
        // 添加数组符号
        if (isArray)
        {
            result = std::string(arrayDimensions, '[') + result;
        }
        
        return result;
    }
    
    // 生成方法签名字符串
    std::string formatMethodSignature(const dex::MethodInfo& methodInfo)
    {
        std::stringstream ss;
        std::string simplifiedReturnType = simplifyTypeName(methodInfo.returnType);
        
        ss << methodInfo.name << "(";
        
        if (methodInfo.hasParameterList && !methodInfo.parameterTypes.empty())
        {
            for (size_t i = 0; i < methodInfo.parameterTypes.size(); i++)
            {
                if (i > 0)
                {
                    ss << ", ";
                }
                ss << simplifyTypeName(methodInfo.parameterTypes[i]);
            }
        }
        
        ss << ")";
        return ss.str();
    }
} 