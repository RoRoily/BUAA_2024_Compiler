//
// Created by 郑慕函 on 24-10-17.
//

#ifndef REFERENCEDENTRY_H
#define REFERENCEDENTRY_H
#include "SymbolTableEntryType.h"

class ReferencedEntry {
public:
    const SymbolTableEntryType actualType; //实际符号表条目类型
    const SymbolTableEntryType referencedType; //被引用的符号表条目类型

    int index;  // ident[index1] //访问一维数组

    bool valueUncertained; //引用的值是否不确定

    //引用的值确定
    ReferencedEntry(SymbolTableEntryType actualType,
                    SymbolTableEntryType referencedType,
                    int index) : actualType(actualType),
                                 referencedType(referencedType),
                                 index(index),
                                 valueUncertained(false) {}

    //引用的值不确定
    ReferencedEntry(SymbolTableEntryType actualType,
                    SymbolTableEntryType referencedType) : actualType(actualType),
                                                     referencedType(referencedType),
                                                     valueUncertained(true) {}
};


#endif //REFERENCEDENTRY_H
