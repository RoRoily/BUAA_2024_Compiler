//
// Created by 郑慕函 on 24-11-11.
//


#include "../include/QuadEntry.h"
#include "../include/QuadItemInt.h"
#include "../include/QuadItemChar.h"
#include "../include/QuadItemCharArray.h"
#include "../include/QuadItemIntArray.h"
std::map<QuadEntryType, std::string> quadEntryTypeMap1 = {
        {QuadEntryType::IntDefine, "IntDefine"},
        {QuadEntryType::CharDefine, "CharDefine"},
        {QuadEntryType::ConstIntDefine, "ConstIntDefine"},
        {QuadEntryType::ConstCharDefine, "ConstCharDefine"},

        {QuadEntryType::IntArrayDefine, "IntArrayDefine"},
        {QuadEntryType::CharArrayDefine, "CharArrayDefine"},
        {QuadEntryType::ConstIntArrayDefine, "ConstIntArrayDefine"},
        {QuadEntryType::ConstCharArrayDefine, "ConstCharArrayDefine"},

        {QuadEntryType::FuncDefine, "FuncDefine"},
        {QuadEntryType::FuncCall, "FuncCall"},
        {QuadEntryType::FuncReturnWithValue, "FuncReturnWithValue"},
        {QuadEntryType::FuncReturn, "FuncReturn"},

        {QuadEntryType::MainFuncStart, "MainFuncStart"},
        {QuadEntryType::MainFuncEnd, "MainFuncEnd"},

        {QuadEntryType::Getint, "Getint"},
        {QuadEntryType::Getchar, "Getchar"},
        {QuadEntryType::Print, "Print"},
        {QuadEntryType::Assign, "Assign"},
        {QuadEntryType::Add, "Add"},
        {QuadEntryType::Sub, "Sub"},
        {QuadEntryType::Mul, "Mul"},
        {QuadEntryType::Div, "Div"},
        {QuadEntryType::Mod, "Mod"},
        {QuadEntryType::Not, "Not"},
        {QuadEntryType::Neg, "Neg"},
        {QuadEntryType::ArrayGet, "ArrayGet"}
};

QuadEntry::QuadEntry(QuadEntryType type, QuadItem *operator1) :
        entryType(type),
        calledFunc(nullptr),
        operator1(operator1),
        operator2(nullptr),
        operator3(nullptr),
        params(nullptr),
        opNum(1) {printf("Type of Entry is %s\n",quadEntryTypeMap1.find(type)->second.c_str());}

QuadEntry::QuadEntry(QuadEntryType type, QuadItem *operator1, QuadItem *operator2) :
        entryType(type),
        calledFunc(nullptr),
        operator1(operator1),
        operator2(operator2),
        operator3(nullptr),
        params(nullptr),
        opNum(2) {printf("this : Type of Entry is %s\n",quadEntryTypeMap1.find(type)->second.c_str());}

QuadEntry::QuadEntry(QuadEntryType type, QuadItem *operator1, QuadItem *operator2, QuadItem *operator3) :
        entryType(type),
        calledFunc(nullptr),
        operator1(operator1),
        operator2(operator2),
        operator3(operator3),
        params(nullptr),
        opNum(3) {printf("Type of Entry is %s\n",quadEntryTypeMap1.find(type)->second.c_str());}

QuadEntry::QuadEntry(QuadEntryType type) :
        entryType(type),
        calledFunc(nullptr),
        operator1(nullptr),
        operator2(nullptr),
        operator3(nullptr),
        params(nullptr),
        opNum(0) {printf("Type of Entry is %s\n",quadEntryTypeMap1.find(type)->second.c_str());}

QuadEntry::QuadEntry(QuadItem *calledFunc, std::vector<QuadItem *> *params) :
        entryType(QuadEntryType::FuncCall),
        calledFunc(calledFunc),
        operator1(nullptr),
        operator2(nullptr),
        operator3(nullptr),
        params(params),
        opNum(1) {printf("Type of Entry is Funcall\n");}

bool QuadEntry::isVarOrConstDef() const {
    return (entryType == QuadEntryType::IntDefine ||
            entryType == QuadEntryType::CharDefine ||
            entryType == QuadEntryType::ConstIntDefine ||
            entryType == QuadEntryType::ConstCharDefine ||
            entryType == QuadEntryType::IntArrayDefine ||
            entryType == QuadEntryType::CharArrayDefine ||
            entryType == QuadEntryType::ConstIntArrayDefine ||
            entryType == QuadEntryType::ConstCharArrayDefine);

}

bool QuadEntry::isGlobalVarOrConstDef() const {
        if (entryType == QuadEntryType::IntDefine ||
                entryType == QuadEntryType::CharDefine ||
                entryType == QuadEntryType::ConstIntDefine ||
                entryType == QuadEntryType::ConstCharDefine ) {
        auto *var = ((QuadItemInt *) operator1);
        return var->isGlobal;
    }
        if ( entryType == QuadEntryType::IntArrayDefine ||
                entryType == QuadEntryType::CharArrayDefine ||
                entryType == QuadEntryType::ConstIntArrayDefine ||
                entryType == QuadEntryType::ConstCharArrayDefine) {
        auto *array = ((QuadItemIntArray *) operator1);
        return array->isGlobal;
    }
    return false;
}
