//
// Created by GaGa on 25-5-13.
//

#ifndef METHODPARSER_H
#define METHODPARSER_H

#include <vector>
#include "BaseParser.h"

namespace dex::parser
{
    /**
     * Method解析器
     * 负责解析DEX文件中的Method信息
     */
    class MethodParser final : public BaseParser
    {
    public:
        using BaseParser::BaseParser;

        /**
         * 构造函数
         * @param fileData 文件数据
         * @param fileSize 文件大小
         */
        MethodParser(const uint8_t* fileData, size_t fileSize);
        
        /**
         * 析构函数
         */
        ~MethodParser() override = default;

        /**
         * 实现解析函数
         * @return 解析是否成功
         */
        bool parse() override;

    private:
        // MethodId表指针
        const DexMethodId* methodIds_;
        
        // MethodId表大小
        uint32_t methodIdsSize_;
    };
}

#endif //METHODPARSER_H 