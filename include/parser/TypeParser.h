//
// Created by GaGa on 25-5-9.
//

#ifndef TYPEPARSER_H
#define TYPEPARSER_H
#include <cstdint>

#include "BaseParser.h"


namespace dex::parser
{
    class TypeParser final : public BaseParser
    {
    public:
        TypeParser(const uint8_t* fileData, size_t fileSize);
        ~TypeParser() override = default;

        bool parse() override;

    private:
        const DexTypeId* typeIds_;
        uint32_t typeIdsSize_;
    };
}


#endif //TYPEPARSER_H
