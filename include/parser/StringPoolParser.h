//
// Created by GaGa on 25-5-8.
//

#ifndef STRINGPOOLPARSER_H
#define STRINGPOOLPARSER_H
#include <vector>

#include "BaseParser.h"


namespace dex::parser
{
    class StringPoolParser final : public BaseParser
    {
    public:
        using BaseParser::BaseParser;

        /**
         * 构造函数
         * @param fileData 文件数据
         * @param fileSize 文件大小
         */
        StringPoolParser(const uint8_t* fileData, size_t fileSize);
        
        /**
         * 析构函数
         */
        ~StringPoolParser() override = default;

        /**
         * 实现解析函数
         * @return 解析是否成功
         */
        bool parse() override;

    private:
        // 字符串ID表指针
        const DexStringId* stringIds_;
        
        // 字符串ID表大小
        uint32_t stringIdsSize_;
    };
}


#endif //STRINGPOOLPARSER_H
