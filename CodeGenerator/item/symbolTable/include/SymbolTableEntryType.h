//
// Created by 郑慕函 on 24-10-17.
//

#ifndef THIS_PROJECT_SYMBOL_TABLE_ENTRY_TYPE_H
#define THIS_PROJECT_SYMBOL_TABLE_ENTRY_TYPE_H

#include <map>
enum class SymbolTableEntryType {
    Int,
    Char,
    IntConst,
    CharConst,

    IntArray,
    CharArray,
    ConstIntArray,
    ConstCharArray,

    FunctionOfVoid,  // 返回值是void的函数
    FunctionOfInt,   // 返回值是int的函数
    FunctionOfChar, //返回值是char的函数

    ReferencedEntry,  // 临时表项，用于引用数组，如：记录函数调用中 func(arr[1][2]) 中的 arr[1][2]

};

// 创建枚举到字符串的映射
const std::map<SymbolTableEntryType, std::string> SymbolTableEntryType2String = {
    {SymbolTableEntryType::Int, "Int"},
    {SymbolTableEntryType::Char, "Char"},
    {SymbolTableEntryType::IntConst, "IntConst"},
    {SymbolTableEntryType::CharConst, "CharConst"},
    {SymbolTableEntryType::IntArray, "IntArray"},
    {SymbolTableEntryType::CharArray, "CharArray"},
    {SymbolTableEntryType::ConstIntArray, "ConstIntArray"},
    {SymbolTableEntryType::ConstCharArray, "ConstCharArray"},
    {SymbolTableEntryType::FunctionOfVoid, "FunctionOfVoid"},
    {SymbolTableEntryType::FunctionOfInt, "FunctionOfInt"},
    {SymbolTableEntryType::FunctionOfChar, "FunctionOfChar"},
    {SymbolTableEntryType::ReferencedEntry, "ReferencedEntry"}
};

#endif //THIS_PROJECT_SYMBOL_TABLE_ENTRY_TYPE_H


