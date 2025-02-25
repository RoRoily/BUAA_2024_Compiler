//
// Created by 郑慕函 on 24-11-19.
//
//
// Created by 郑慕函 on 24-11-11.
//
#include "../include/QuadItemChar.h"

#include "../include/QuadItemInt.h"

/**
 *
 * @param originalName
 * @param symbolTableEntry
 * @param isConst
 * @param isGlobal
 * @param value
 */
QuadItemChar::QuadItemChar(const std::string *originalName, SymbolTableEntry *symbolTableEntry,
                     bool isConst, bool isGlobal, char value)
        : QuadItem(QuadItemType::Char),
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
QuadItemChar::QuadItemChar(bool isGlobal, char value)
        : QuadItem(QuadItemType::Char),
          originalName(nullptr),
          symbolTableEntry(nullptr),
          isConst(false),
          isGlobal(isGlobal),
          value(value),
          varId(-1),
          tempVarId(generateTempVarId()),
          isTemp(true) {}

int QuadItemChar::generateVarId() {
    static int i = 0;
    return --Global_Count;
    return --i;
}

int QuadItemChar::generateTempVarId() {
    static int i = 0;
    return ++Temp_Global_Count;
    return ++i;
}

std::string QuadItemChar::toString() const{
    if (isTemp) {
        return "temp_char" + std::to_string(abs(tempVarId));
    } else {
        if (isGlobal) {
            if (isConst) {
                return "global_const_char" + std::to_string(abs(varId));
            } else {
                return "global_char" + std::to_string(abs(varId));
            }
        } else {
            if (isConst) {
                return "local_const_char" + std::to_string(abs(varId));
            } else {
                return "local_char" + std::to_string(abs(varId));
            }
        }
    }
}

QuadItemChar::~QuadItemChar() {
    delete symbolTableEntry;
}
