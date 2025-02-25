//
// Created by 郑慕函 on 24-11-11.
//

#ifndef QUADENTRY_H
#define QUADENTRY_H
#include <vector>
#include "QuadEntryType.h"
#include "QuadItem.h"

/**
 * 一条中间代码
 */
class QuadEntry {
public:
    const QuadEntryType entryType;
    QuadItem *operator1;
    QuadItem *operator2;
    QuadItem *operator3;
    const QuadItem *calledFunc;  // 仅用于函数调用
    const int opNum;
    const std::vector<QuadItem *> *params;

    explicit QuadEntry(QuadEntryType type);

    QuadEntry(QuadEntryType type, QuadItem *operator1);

    QuadEntry(QuadItem *calledFunc, std::vector<QuadItem *> *params = nullptr);

    QuadEntry(QuadEntryType type, QuadItem *operator1, QuadItem *operator2);

    QuadEntry(QuadEntryType type, QuadItem *operator1, QuadItem *operator2, QuadItem *operator3);

    bool isVarOrConstDef() const;

    bool isGlobalVarOrConstDef() const;
};

#endif //QUADENTRY_H
