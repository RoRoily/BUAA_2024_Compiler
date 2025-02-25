//
// Created by 郑慕函 on 24-11-19.
//

#ifndef QUADITEMCHAR_H
#define QUADITEMCHAR_H
#include "QuadItem.h"
#include "../../symbolTable/include/SymbolTableEntry.h"

class QuadItemChar : public QuadItem {
    static int generateVarId();

    static int generateTempVarId(); //临时变量不在符号表中注册

public:
    const bool isTemp;
    const int tempVarId;

    const std::string *originalName;
    const SymbolTableEntry *symbolTableEntry; //变量对应的符号表条目
    const bool isGlobal;
    bool isConst;
    const int varId;
    char value;

    QuadItemChar(const std::string *originalName, SymbolTableEntry *symbolTableEntry,
              bool isConst, bool isGlobal, char value ='\0');

    explicit QuadItemChar(bool isGlobal, char value = '\0');

    [[nodiscard]] std::string toString() const;

    ~ QuadItemChar();

};
#endif //QUADITEMCHAR_H
