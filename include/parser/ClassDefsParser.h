//
// Created by GaGa on 25-5-14.
//

#ifndef CLASSDEFSPARSER_H
#define CLASSDEFSPARSER_H

#include <vector>
#include "BaseParser.h"

namespace dex::parser
{
    /**
     * ClassDefs解析器
     * 负责解析DEX文件中的类定义信息
     */
    class ClassDefsParser final : public BaseParser
    {
    public:
        using BaseParser::BaseParser;

        /**
         * 构造函数
         * @param fileData 文件数据
         * @param fileSize 文件大小
         */
        ClassDefsParser(const uint8_t* fileData, size_t fileSize);
        
        /**
         * 析构函数
         */
        ~ClassDefsParser() override = default;

        /**
         * 实现解析函数
         * @return 解析是否成功
         */
        bool parse() override;

    private:
        // ClassDef表指针
        const DexClassDef* classDefs_;
        
        // ClassDef表大小
        uint32_t classDefsSize_;
    };
}

#endif //CLASSDEFSPARSER_H 