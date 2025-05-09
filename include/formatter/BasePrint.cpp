//
// Created by GaGa on 25-5-8.
//

#include "BasePrint.h"

namespace dex::print
{
    DexContext& BasePrint::getContext()
    {
        return DexContext::getInstance();
    }
}