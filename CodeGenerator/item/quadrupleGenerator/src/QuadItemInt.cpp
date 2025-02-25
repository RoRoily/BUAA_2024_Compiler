//
// Created by 郑慕函 on 24-11-11.
//
#include "../include/QuadItemInt.h"

/**
 *
 * @param originalName
 * @param symbolTableEntry
 * @param isConst
 * @param isGlobal
 * @param value
 */


QuadItemInt::QuadItemInt(const std::string *originalName, SymbolTableEntry *symbolTableEntry,
                     bool isConst, bool isGlobal, int value)
        : QuadItem(QuadItemType::Int),
          isGlobal(isGlobal),
          isConst(isConst),
          varId(generateVarId()),
          tempVarId(-1),
          value(value),
          symbolTableEntry(symbolTableEntry),
          originalName(originalName),
          isTemp(false) {}

/**
 *
 * @param isGlobal
 * @param value
 */
QuadItemInt::QuadItemInt(bool isGlobal, int value)
        : QuadItem(QuadItemType::Int),
          originalName(nullptr),
          symbolTableEntry(nullptr),
          isConst(false),
          isGlobal(isGlobal),
          value(value),
          varId(-1),
          tempVarId(generateTempVarId()),
          isTemp(true) {}

int QuadItemInt::generateVarId() {
    static int i = 0;
    return --Global_Count;
    return --i;
}

int QuadItemInt::generateTempVarId() {
    static int i = 0;
    return ++Temp_Global_Count;
    return ++i;
}

std::string QuadItemInt::toString() const{
    if (isTemp) {
        return "temp_int" + std::to_string(abs(tempVarId));
    } else {
        if (isGlobal) {
            if (isConst) {
                return "global_const_int" + std::to_string(abs(varId));
            } else {
                return "global_int" + std::to_string(abs(varId));
            }
        } else {
            if (isConst) {
                return "local_const_int" + std::to_string(abs(varId));
            } else {
                return "local_int" + std::to_string(abs(varId));
            }
        }
    }
}

QuadItemInt::~QuadItemInt() {
    delete symbolTableEntry;
}
