//
// Created by GaGa on 25-5-8.
//

#ifndef BASE_PARSER_H
#define BASE_PARSER_H

#include <cstdint>
#include <string>
#include "core/DexFile.h"

namespace dex::parser
{
    /**
     * 解析器基类
     * 为所有类型的解析器提供基本功能和接口
     */
    class BaseParser
    {
    public:
        /**
         * 有参构造函数
         * @param fileData 文件数据指针
         * @param fileSize 文件大小
         * @param header DEX头部指针，可为空
         */
        BaseParser(const uint8_t* fileData, size_t fileSize, const DexHeader* header = nullptr);

        /**
         * 无参构造函数
         */
        BaseParser();

        /**
         * 虚析构函数
         */
        virtual ~BaseParser() = default;

        /**
         * 解析函数，子类必须实现
         * @return 解析是否成功
         */
        virtual bool parse() = 0;

        /**
         * 获取最后一次错误信息
         * @return 错误信息
         */
        const std::string& getLastError() const;

        /**
         * 设置错误信息
         * @param error 错误信息
         */
        void setError(const std::string& error) const;

        /**
        * 设置带格式的错误信息
        * @param fmt 格式字符串
        * @param ... 可变参数
        */
        void setError(const char* fmt, ...) const;

    protected:
        /**
         * 检查偏移量是否有效
         * @param offset 偏移量
         * @param size 大小
         * @return 是否有效
         */
        bool isValidOffset(uint32_t offset, uint32_t size = 0) const;

        // 文件数据指针
        const uint8_t* BaseFileData_;

        // 文件大小
        size_t BaseFileSize_;

        // DEX头部指针
        const DexHeader* BaseHeader_;

        // 最后一次错误信息
        mutable std::string lastError_;
    };
} // namespace dex::parser


#endif // BASE_PARSER_H
