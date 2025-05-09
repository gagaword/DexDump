//
// Created by GaGa on 25-5-11.
//

#ifndef PROTOPARSER_H
#define PROTOPARSER_H

#include <vector>
#include "BaseParser.h"

namespace dex::parser
{
    /**
     * Proto解析器
     * 负责解析DEX文件中的Proto信息
     */
    class ProtoParser final : public BaseParser
    {
    public:
        using BaseParser::BaseParser;

        /**
         * 构造函数
         * @param fileData 文件数据
         * @param fileSize 文件大小
         */
        ProtoParser(const uint8_t* fileData, size_t fileSize);
        
        /**
         * 析构函数
         */
        ~ProtoParser() override = default;

        /**
         * 实现解析函数
         * @return 解析是否成功
         */
        bool parse() override;

    private:
        // ProtoId表指针
        const DexProtoId* protoIds_;
        
        // ProtoId表大小
        uint32_t protoIdsSize_;
    };
}

#endif //PROTOPARSER_H
