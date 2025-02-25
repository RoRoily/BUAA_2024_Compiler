//
// Created by 郑慕函 on 24-10-17.
//

#ifndef THIS_PROJECT_SYMBOL_TABLE_ENTRY_H
#define THIS_PROJECT_SYMBOL_TABLE_ENTRY_H

#include <string>
#include <vector>
#include "SymbolTableEntryType.h"


#include "../../variableType/CharFunc.h"
#include "../../variableType/IntFunc.h"
#include "../../variableType/VoidFunc.h"
#include "../../tree/include/Node.h"
#include "../../variableType/Char.h"
#include "../../variableType/Int.h"
#include "../../variableType/CharArray.h"
#include "../../variableType/IntArray.h"
#include "../../variableType/ConstChar.h"
#include "../../variableType/ConstIntArray.h"
#include "../../variableType/ConstCharArray.h"
#include "../../variableType/ConstInt.h"
/**
 * 符号表对应的表项
 * 区别于Symbol.h!
 * 符号表条目
 */
#include "ReferencedEntry.h"

class SymbolTableEntry {


public:
    const SymbolTableEntryType type;
    explicit SymbolTableEntry(int value);

    SymbolTableEntry(char value);

    SymbolTableEntry(SymbolTableEntry *defineEntry, SymbolTableEntryType actualType, int d1);

    SymbolTableEntry(SymbolTableEntry *defineEntry, SymbolTableEntryType actualType);

    SymbolTableEntry(SymbolTableEntry *defineEntry, SymbolTableEntryType actualType, int d1, int d2);

    SymbolTableEntry(Node *node, Char *charCon, unsigned int defLineNum, bool isFuncFParam);

    SymbolTableEntry(Node *node, Int *intCon, unsigned int defLineNum, bool isFuncFParam);

    SymbolTableEntry(Node *node, ConstChar *constCharCon, unsigned int defLineNum, bool isFuncFParam);

    SymbolTableEntry(Node *node, ConstInt *constIntCon, unsigned int defLineNum, bool isFuncFParam);

    SymbolTableEntry(Node *node, CharArray *charArrayCon, unsigned int defLineNum, bool isFuncFParam);

    SymbolTableEntry(Node *node, IntArray *intArrayCon, unsigned int defLineNum, bool isFuncFParam);

    SymbolTableEntry(Node *node, ConstCharArray *consrCharArrayCon, unsigned int defLineNum, bool isFuncFParam);

    SymbolTableEntry(Node *node, SymbolTableEntryType type, unsigned int defLineNum, bool isFuncFParam);

    SymbolTableEntry(Node *node, ConstIntArray *constIntArrayCon, unsigned int defLineNum, bool isFuncFParam);


    SymbolTableEntry(Node *node, IntFunc *functionOfInt, unsigned int defLineNum);

    SymbolTableEntry(Node *node, VoidFunc *functionOfVoid, unsigned int defLineNum);

    SymbolTableEntry(Node *node, SymbolTableEntryType type, unsigned int defLineNum);

    SymbolTableEntry(Node *node, CharFunc *functionOfVoid, unsigned int defLineNum);

    Node *node;
    const bool isFuncFParam;  // 函数的形参
    unsigned int defLineNum;

    Int *handle_int{nullptr};
    Char *handle_char{nullptr};
    ConstInt *constInt{nullptr};
    ConstChar *constChar{nullptr};

    IntArray *intArray{nullptr};
    CharArray *charArray{nullptr};
    ConstIntArray *constIntArray{nullptr};
    ConstCharArray *constCharArray{nullptr};

    IntFunc * intFunc{nullptr};
    CharFunc *charFunc{nullptr};
    VoidFunc *voidFunc{nullptr};

    // 引用表项
    ReferencedEntry *tempEntry{nullptr};  // 该类型不应该保存到符号表 !!!
    // 其对应的真正定义的表项
    SymbolTableEntry *definedEntry{nullptr};

    SymbolTableEntryType getActualType() const;

    int *getAllIntArrayConst() const;

    string getAllCharArrayConst() const;

    ~SymbolTableEntry();

    std::string *getName() const;

    bool isReferencedEntry() const;


    int funcParamsNum() const;

    std::vector<FuncParam *> *getFuncParams() const;

    int intGet() const;

    char charGet() const;

    int getValueFromReferencedIntArray(int d1) const;

    char getValueFromReferencedCharArray(int d1) const;

    int intArrayGet(int index) const;

    [[nodiscard]] char charArrayGet(int index) const;

    static bool hasSameType(SymbolTableEntry *realParam, FuncParam *funcParam);


    void addParamForFuncEntry(SymbolTableEntry *param);



    bool isConst() const;
    const std::map<SymbolTableEntryType, std::string> symbolTableEntryType2string = {
        {SymbolTableEntryType::Int,              "Int"},
        {SymbolTableEntryType::Char,             "Char"},
        {SymbolTableEntryType::IntConst,         "ConstInt"},
        {SymbolTableEntryType::CharConst,        "ConstChar"},

        {SymbolTableEntryType::IntArray,         "IntArray"},
        {SymbolTableEntryType::CharArray,        "CharArray"},
        {SymbolTableEntryType::ConstIntArray,    "ConstIntArray"},
        {SymbolTableEntryType::ConstCharArray,   "ConstCharArray"},

        {SymbolTableEntryType::FunctionOfVoid,   "VoidFunc"},
        {SymbolTableEntryType::FunctionOfInt,    "IntFunc"},
        {SymbolTableEntryType::FunctionOfChar,   "CharFunc"},

        {SymbolTableEntryType::ReferencedEntry,  "ReferencedEntry"}
    };
};

#endif //THIS_PROJECT_SYMBOL_TABLE_ENTRY_H

