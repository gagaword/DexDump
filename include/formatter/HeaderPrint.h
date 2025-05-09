//
// Created by GaGa on 25-5-9.
//

#ifndef HEDAERPRINT_H
#define HEDAERPRINT_H
#include "BasePrint.h"


namespace dex::print
{
    class HeaderPrint final : public BasePrint
    {
    public:
        void print() override;
    };
}


#endif //HEDAERPRINT_H
