//
// Created by 郑慕函 on 24-11-11.
//

#include "../include/QuadItemFunc.h"

#include "../include/QuadItemInt.h"
#include "../include/QuadItemIntArray.h"

QuadItemFunc::QuadItemFunc(SymbolTableEntry *funcEntry) :
        QuadItem(QuadItemType::Func),
        paramNum(funcEntry->funcParamsNum()),
        funcLabel(new QuadItemLabel()),
        originName(funcEntry->getName()),
        params(new std::vector<QuadItem *>),
        entries(new std::vector<QuadEntry *>){

    if (funcEntry->type == SymbolTableEntryType::FunctionOfInt) {
        returnType = 0;
    } else if(funcEntry->type == SymbolTableEntryType::FunctionOfChar) {
        returnType = 1;
    }else{
        returnType = 2;
    }
    // TODO: 假设没数组
    for (const auto *item: *funcEntry->getFuncParams()) {
       // 0:int 1:char 2:int[] 3：char[]
        if (item->type == 0) {
            auto *icItemVar = new QuadItemInt(item->name, nullptr, false, false);
            params->push_back(icItemVar);
        }
        else if(item->type==1) {
            auto *icItemVar = new QuadItemChar(item->name, nullptr, false, false);
            params->push_back(icItemVar);
        }
        else if(item->type ==2) {
            auto *icItemArray = new QuadItemIntArray(item->name, nullptr, false, false);
            icItemArray->originType.d = item->type;
            params->push_back(icItemArray);
        }
        else {
            auto *icItemArray = new QuadItemCharArray(item->name, nullptr, false, false);
            icItemArray->originType.d = item->type;
            params->push_back(icItemArray);
        }
    }
}
