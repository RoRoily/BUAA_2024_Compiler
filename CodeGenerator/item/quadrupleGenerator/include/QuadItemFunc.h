//
// Created by 郑慕函 on 24-11-12.
//

#ifndef QUADITEMFUNC_H
#define QUADITEMFUNC_H

#include "QuadItem.h"
#include "QuadItemLabel.h"
#include "QuadEntry.h"
#include "../../symbolTable/include/SymbolTableEntry.h"
#include <vector>

class QuadItemFunc : public QuadItem{
public:
    QuadItemLabel *funcLabel;
    std::string *originName;
    int returnType; //int 0 char 1 void 2
    std::vector<QuadItem *> *params;
    std::vector<QuadEntry *> *entries;
    const int paramNum;

    explicit QuadItemFunc(SymbolTableEntry *funcEntry);
};
#endif //QUADITEMFUNC_H
