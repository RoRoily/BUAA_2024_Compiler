//
// Created by 郑慕函 on 24-11-11.
//

#ifndef QUADENTRYTYPE_H
#define QUADENTRYTYPE_H
#include <string>
#include <map>
enum class QuadEntryType {
    // 第一次代码生成作业:
    IntDefine,
    CharDefine,
    ConstIntDefine,
    ConstCharDefine,

    IntArrayDefine,
    CharArrayDefine,
    ConstIntArrayDefine,
    ConstCharArrayDefine,

    FuncDefine,
    FuncCall,
    FuncReturnWithValue,
    FuncReturn,

    MainFuncStart,
    MainFuncEnd,

    Getint,
    Getchar,
    Print,
    Assign,
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    Not,
    Neg,
    ArrayGet,

    Bnez,
    Beqz,

    JumpLabel,
    InsertLabel,
    // 第二次代码生成作业:
    Or,
    And,
    Equal,
    NotEqual,
    LessEqual,
    LessThan,
    GreaterThan,
    GreaterEqual
};
// 构造映射表

#endif //QUADENTRYTYPE_H
