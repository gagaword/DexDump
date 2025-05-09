//
// Created by GaGa on 25-5-12.
//

#ifndef FIELDPARSER_H
#define FIELDPARSER_H

#include <vector>
#include "BaseParser.h"

namespace dex::parser
{
    /**
     * Field解析器
     * 负责解析DEX文件中的Field信息
     */
    class FieldParser final : public BaseParser
    {
    public:
        using BaseParser::BaseParser;

        /**
         * 构造函数
         * @param fileData 文件数据
         * @param fileSize 文件大小
         */
        FieldParser(const uint8_t* fileData, size_t fileSize);
        
        /**
         * 析构函数
         */
        ~FieldParser() override = default;

        /**
         * 实现解析函数
         * @return 解析是否成功
         */
        bool parse() override;

    private:
        // FieldId表指针
        const DexFieldId* fieldIds_;
        
        // FieldId表大小
        uint32_t fieldIdsSize_;
    };
}

#endif //FIELDPARSER_H 