//
// Created by GaGa on 25-5-8.
//

#ifndef HEADERPARSER_H
#define HEADERPARSER_H
#include "BaseParser.h"

namespace dex::parser
{
    /**
     * DEX文件头部解析器
     * 负责解析和验证DEX文件的头部信息
     */
    class HeaderParser final : public BaseParser
    {
    public:
        using BaseParser::BaseParser;

        /**
         * 构造函数
         * @param fileData DEX文件数据
         * @param fileSize DEX文件大小
        */
        HeaderParser(const uint8_t* fileData, size_t fileSize);

        /**
        * 析构函数
        */
        ~HeaderParser() override = default;

        /**
        * 解析DEX文件头部
        * @return 解析是否成功
        */
        bool parse() override;

        /**
         * 获取解析后的头部结构
         * @return DEX头部结构
         */
        const DexHeader& getHeader() const;

        /**
         * 验证头部信息是否有效
         * @return 是否有效
         */
        [[nodiscard]] static bool isValid(const uint8_t* fileData);

    private:

        /**
         * 验证魔数
         * @return 是否有效
         */
        bool validateMagic() const;

        /**
         * 验证文件大小是否与头部声明一致
         * @return 是否一致
         */
        bool validateFileSize() const;

        /**
         * 验证头部大小
         * @return 是否有效
         */
        bool validateHeaderSize() const;

        /**
         * 验证大端标记
         * @return 是否有效
         */
        bool validateEndianTag() const;

        /**
         * 验证各个段的偏移量和大小
         * @return 是否有效
         */
        bool validateSectionOffsets() const;

        // 解析后的头部结构
        DexHeader header_{};

        // 头部是否有效
        bool isValid_{};
    };
}


#endif //HEADERPARSER_H
