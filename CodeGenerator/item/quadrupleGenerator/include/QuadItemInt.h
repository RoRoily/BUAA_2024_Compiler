//
// Created by 郑慕函 on 24-11-11.
//

#ifndef QUADITEMVAR_H
#define QUADITEMVAR_H
#include "QuadItem.h"
#include "../../symbolTable/include/SymbolTableEntry.h"
inline extern int Global_Count = -1;
inline extern int Temp_Global_Count = 1;
class QuadItemInt : public QuadItem {
    static int generateVarId();

    static int generateTempVarId();

public:
    const bool isTemp;
    const int tempVarId;

    const std::string *originalName;
    const SymbolTableEntry *symbolTableEntry;
    const bool isGlobal;
    bool isConst;
    const int varId;
    int value;

    QuadItemInt(const std::string *originalName, SymbolTableEntry *symbolTableEntry,
              bool isConst, bool isGlobal, int value = 0);

    explicit QuadItemInt(bool isGlobal, int value = 0);

    [[nodiscard]] std::string toString() const;

    ~QuadItemInt();

};
#endif //QUADITEMVAR_H
