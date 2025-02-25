//
// Created by 郑慕函 on 24-11-11.
//

#ifndef QUADITEMTYPE_H
#define QUADITEMTYPE_H
#include <map>
enum class QuadItemType {
    String,  // 字符串，包括若干个纯字符串和引用的数值 (字符和数字输出方式不同)
    Imm,  // 常量数字 (立即数)
    Imc,  //常量字符
    Label,  // 函数、数组、循环跳转、字符串存储等用到的标签 (用于挑战到函数入口)
    IntArray,
    CharArray,
    Int,
    Char,
    Func,
    Reference
};

// 创建一个函数来初始化映射

#endif //QUADITEMTYPE_H
