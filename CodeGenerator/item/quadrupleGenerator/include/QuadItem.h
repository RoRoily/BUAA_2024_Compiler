//
// Created by 郑慕函 on 24-11-11.
//

#ifndef QUADITEM_H
#define QUADITEM_H
#include <string>
#include "QuadItemType.h"

/**
 * Intermediate Code Item
 */

enum class ReferenceType {
    Unset,
    Int,
    Char,
    IntArray,
    CharArray,
    IntArray_Var,
    CharArray_Var
};

inline std::map<ReferenceType, std::string> referenceTypeMap = {
    {ReferenceType::Unset, "Unset"},
    {ReferenceType::Int, "Int"},
    {ReferenceType::Char, "Char"},
    {ReferenceType::IntArray, "IntArray"},
    {ReferenceType::CharArray, "CharArray"},
    {ReferenceType::IntArray_Var, "IntArray_Var"},
    {ReferenceType::CharArray_Var, "CharArray_Var"}
};

class QuadItem {
public:
    std::string Name;
    QuadItemType type;
    const std::string newName;
    bool typeChanged;
    QuadItem *newQuadItem;

    // ----------------------------------------------- LVal 引用 -----------------------------------------
    QuadItem *reference;
    ReferenceType referenceType;
    // 该 LVal 是否是函数实参
    bool isFuncRParam;
    // for Array1_Var
    QuadItem *array1_var_index{nullptr};


    // explicit QuadItem(QuadItemType type = QuadItemType::Reference)
    //         : type(type),
    //           reference(nullptr),
    //           typeChanged(false),
    //           newQuadItem(nullptr) {}

    //    virtual std::string *toString() = 0;
    explicit QuadItem(QuadItemType type = QuadItemType::Reference);
    virtual ~QuadItem();

    //QuadItem(QuadItemType type);
};

#endif //QUADITEM_H
