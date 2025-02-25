//
// Created by 郑慕函 on 24-10-17.
//
#include "../include/SymbolTableEntry.h"
#include <iostream>

#include "../../quadrupleGenerator/include/QuadItemType.h"

////常量 且不是函数形参
//intConst
SymbolTableEntry::SymbolTableEntry(int value) :
        isFuncFParam(false), type(SymbolTableEntryType::IntConst) {
     constInt = new ConstInt(value);
    // FIXME: 实际上用于提供函数调用时传入的常数
}

SymbolTableEntry::SymbolTableEntry(char value) :
        isFuncFParam(false), type(SymbolTableEntryType::CharConst) {
    constChar = new ConstChar(value);
}

//处理引用变量，对definEntry的引用
SymbolTableEntry::SymbolTableEntry(SymbolTableEntry *defineEntry, SymbolTableEntryType actualType)
        : type(SymbolTableEntryType::ReferencedEntry), defLineNum(-1), isFuncFParam(false) {
    tempEntry = new ReferencedEntry(actualType, defineEntry->type);
    definedEntry = defineEntry;
}

//处理数组的引用
SymbolTableEntry::SymbolTableEntry(SymbolTableEntry *defineEntry,
                                   SymbolTableEntryType actualType, int d1)
        : type(SymbolTableEntryType::ReferencedEntry), defLineNum(-1), isFuncFParam(false) {
    tempEntry = new ReferencedEntry(actualType, defineEntry->type, d1);
    definedEntry = defineEntry;
}

//

SymbolTableEntry::SymbolTableEntry(Node *node, Char *var,
                                   unsigned int defLineNum, bool isFuncFParam)
        : type(SymbolTableEntryType::Char),
          handle_char(var), node(node), defLineNum(defLineNum), isFuncFParam(isFuncFParam) {printf("1\n");}

SymbolTableEntry::SymbolTableEntry(Node *node, Int *var,
                                   unsigned int defLineNum, bool isFuncFParam)
        : type(SymbolTableEntryType::Int),
          handle_int(var), node(node), defLineNum(defLineNum), isFuncFParam(isFuncFParam) {printf("2\n");}

//创建常量符号表项
SymbolTableEntry::SymbolTableEntry(Node *node, ConstChar *varConst,
                                   unsigned int defLineNum, bool isFuncFParam)
        : type(SymbolTableEntryType::CharConst),
          constChar(varConst), node(node), defLineNum(defLineNum), isFuncFParam(isFuncFParam) {}

SymbolTableEntry::SymbolTableEntry(Node *node, ConstInt *varConst,
                                   unsigned int defLineNum, bool isFuncFParam)
        : type(SymbolTableEntryType::IntConst),
          constInt(varConst), node(node), defLineNum(defLineNum), isFuncFParam(isFuncFParam) {}


//一维数组

SymbolTableEntry::SymbolTableEntry(Node *node, IntArray *varConst,
                                   unsigned int defLineNum, bool isFuncFParam)
        : type(SymbolTableEntryType::IntArray),
          intArray(varConst), node(node), defLineNum(defLineNum), isFuncFParam(isFuncFParam) {printf("4\n");}

SymbolTableEntry::SymbolTableEntry(Node *node, CharArray *varConst,
                                   unsigned int defLineNum, bool isFuncFParam)
        : type(SymbolTableEntryType::CharArray),
          charArray(varConst), node(node), defLineNum(defLineNum), isFuncFParam(isFuncFParam) {printf("3\n");}

//一维常量数组

SymbolTableEntry::SymbolTableEntry(Node *node, ConstIntArray *array1Const,
                                   unsigned int defLineNum, bool isFuncFParam)
        : type(SymbolTableEntryType::ConstIntArray),
          constIntArray(array1Const), node(node), defLineNum(defLineNum), isFuncFParam(isFuncFParam) {}


SymbolTableEntry::SymbolTableEntry(Node *node, ConstCharArray *array1Const,
                                   unsigned int defLineNum, bool isFuncFParam)
        : type(SymbolTableEntryType::ConstCharArray),
          constCharArray(array1Const), node(node), defLineNum(defLineNum), isFuncFParam(isFuncFParam) {}

SymbolTableEntry::SymbolTableEntry(Node *node, SymbolTableEntryType type,
                                   unsigned int defLineNum, bool isFuncFParam)
        : type(type), node(node), defLineNum(defLineNum), isFuncFParam(isFuncFParam) {}



//int函数表项
SymbolTableEntry::SymbolTableEntry(Node *node, IntFunc *functionOfInt, unsigned int defLineNum)
        : type(SymbolTableEntryType::FunctionOfInt),
          intFunc(functionOfInt), node(node), defLineNum(defLineNum), isFuncFParam(false) {}

//char函数表项
SymbolTableEntry::SymbolTableEntry(Node *node, CharFunc *functionOfInt, unsigned int defLineNum)
        : type(SymbolTableEntryType::FunctionOfChar),
          charFunc(functionOfInt), node(node), defLineNum(defLineNum), isFuncFParam(false) {}

//void函数表项
SymbolTableEntry::SymbolTableEntry(Node *node, VoidFunc *functionOfVoid, unsigned int defLineNum)
        : type(SymbolTableEntryType::FunctionOfVoid),
          voidFunc(functionOfVoid), node(node), defLineNum(defLineNum), isFuncFParam(false) {}


SymbolTableEntry::SymbolTableEntry(Node *node, SymbolTableEntryType type, unsigned int defLineNum)
        : type(type), node(node), defLineNum(defLineNum), isFuncFParam(false) {}


std::string *SymbolTableEntry::getName() const {
    return &(node->token->word);
}

bool SymbolTableEntry::isReferencedEntry() const {
    return this->type == SymbolTableEntryType::ReferencedEntry;
}

int SymbolTableEntry::funcParamsNum() const {
    if (type == SymbolTableEntryType::FunctionOfInt) {
        return intFunc->params->size();
    }
    if(type == SymbolTableEntryType::FunctionOfChar) {
        return charFunc->params->size();
    }
    return voidFunc->params->size();
}

std::vector<FuncParam *> *SymbolTableEntry::getFuncParams() const {
    if (type == SymbolTableEntryType::FunctionOfInt) {
        return intFunc->params;
    }
    if (type == SymbolTableEntryType::FunctionOfChar) {
        return charFunc->params;
    }
    return voidFunc->params;
}

int SymbolTableEntry::intGet() const {
    if (type == SymbolTableEntryType::Int) {
        return handle_int->value;
    }
    if(type == SymbolTableEntryType::IntConst) {
        return constInt->value;
    }
    printf("Error in SymbolTableEntry::intGet : var not match any type\n");
    return 0;
}

char SymbolTableEntry::charGet() const {
    if(type == SymbolTableEntryType::Char) {
        return handle_char->value;
    }
    if(type == SymbolTableEntryType::CharConst) {
        return constChar->value;
    }
    printf("Error in SymbolTableEntry::charGet : var not match any type\n");
    return 0;
}

int SymbolTableEntry::getValueFromReferencedIntArray(int d1) const {
    return definedEntry->intArrayGet(d1);
}

char SymbolTableEntry::getValueFromReferencedCharArray(int d1) const {
    return definedEntry->charArrayGet(d1);
}



int SymbolTableEntry::intArrayGet(int index) const {
    if (type == SymbolTableEntryType::IntArray) {
        return intArray->values[index];
    }
    if (type == SymbolTableEntryType::ConstIntArray) {
        return constIntArray->values[index];
    }
    printf("Error in SymbolTableEntry::intArrayGet : var not match any type\n");
    return 0;
}

char SymbolTableEntry::charArrayGet(int index) const {
    if (type == SymbolTableEntryType::CharArray) {
        return charArray->values[index];
    }
    if (type == SymbolTableEntryType::ConstCharArray) {
        return (*constCharArray->values)[index];
    }
    printf("Error in SymbolTableEntry::intArrayGet : var not match any type\n");
    return 0;
}


bool SymbolTableEntry::hasSameType(SymbolTableEntry *realParam, FuncParam *funcParam) {
    //0:int 1:char 2:int[] 3：char[]、
    if (realParam->getActualType() == SymbolTableEntryType::Int || realParam->getActualType() == SymbolTableEntryType::IntConst ||realParam->type == SymbolTableEntryType::FunctionOfInt ) {  // 变量
        return (funcParam->type == 0||funcParam->type == 1);
    }
    if (realParam->getActualType() == SymbolTableEntryType::IntArray || realParam->getActualType() == SymbolTableEntryType::ConstIntArray ) {  // 一维数组
        return funcParam->type == 2;
    }
    if ( realParam->getActualType() == SymbolTableEntryType::Char ||realParam->getActualType() == SymbolTableEntryType::CharConst ||realParam->type == SymbolTableEntryType::FunctionOfChar) {  // 变量
        return (funcParam->type == 1||funcParam->type == 0);
        }
    if ( realParam->getActualType() == SymbolTableEntryType::CharArray || realParam->getActualType() == SymbolTableEntryType::ConstCharArray ) {  // 变量
        return funcParam->type == 3;
        }
    // 不合法情况
    return false;
}

//为函数条目添加参数
void SymbolTableEntry::addParamForFuncEntry(SymbolTableEntry *param) {
    //Function of int
    if (type == SymbolTableEntryType::FunctionOfInt) {
        if (param->type == SymbolTableEntryType::Int ||
            param->type == SymbolTableEntryType::IntConst) {  // 变量
            intFunc->addIntParam(param->getName());
        }
        if (param->type == SymbolTableEntryType::IntArray ||
            param->type == SymbolTableEntryType::ConstIntArray) {  // 一维数组
            intFunc->addIntArrayParam(param->getName());
        }
        if (param->type == SymbolTableEntryType::Char ||
           param->type == SymbolTableEntryType::CharConst) {  // 变量
            intFunc->addCharParam(param->getName());
           }
        if (param->type == SymbolTableEntryType::CharArray ||
            param->type == SymbolTableEntryType::ConstCharArray) {  // 一维数组
            intFunc->addCharArrayParam(param->getName());
            }
        //Function of Char
    } else  if (type == SymbolTableEntryType::FunctionOfChar) {
        if (param->type == SymbolTableEntryType::Int ||
                   param->type == SymbolTableEntryType::IntConst) {  // 变量
            charFunc->addIntParam(param->getName());
                   }
        if (param->type == SymbolTableEntryType::IntArray ||
            param->type == SymbolTableEntryType::ConstIntArray) {  // 一维数组
            charFunc->addIntArrayParam(param->getName());
            }
        if (param->type == SymbolTableEntryType::Char ||
            param->type == SymbolTableEntryType::CharConst) {  // 变量
            charFunc->addCharParam(param->getName());
            }
        if (param->type == SymbolTableEntryType::CharArray ||
            param->type == SymbolTableEntryType::ConstCharArray) {  // 一维数组
            charFunc->addCharArrayParam(param->getName());
            }
    }
    //Function of void
    else
        {
        if (param->type == SymbolTableEntryType::Int ||
             param->type == SymbolTableEntryType::IntConst) {  // 变量
            voidFunc->addIntParam(param->getName());
             }
        if (param->type == SymbolTableEntryType::IntArray ||
            param->type == SymbolTableEntryType::ConstIntArray) {  // 一维数组
            voidFunc->addIntArrayParam(param->getName());
            }
        if (param->type == SymbolTableEntryType::Char ||
            param->type == SymbolTableEntryType::CharConst) {  // 变量
            voidFunc->addCharParam(param->getName());
            }
        if (param->type == SymbolTableEntryType::CharArray ||
            param->type == SymbolTableEntryType::ConstCharArray) {  // 一维数组
            voidFunc->addCharArrayParam(param->getName());
            }
    }
}

\

bool SymbolTableEntry::isConst() const {
    return (type == SymbolTableEntryType::IntConst ||type == SymbolTableEntryType::CharConst ||
            type == SymbolTableEntryType::ConstIntArray ||  type == SymbolTableEntryType::ConstCharArray ||
             ( isReferencedEntry() && definedEntry->isConst()));
}

SymbolTableEntryType SymbolTableEntry::getActualType() const {
    if (isReferencedEntry()) {
        return tempEntry->actualType;
    }
    return type;
}

int *SymbolTableEntry::getAllIntArrayConst() const {
    if(type != SymbolTableEntryType::ConstIntArray) {
        printf("Error in getAllInteArrayConst : type not correct\n");
    }
        return constIntArray->values;

}

string SymbolTableEntry::getAllCharArrayConst() const {
    if(type != SymbolTableEntryType::ConstCharArray) {
        printf("Error in getAllChareArrayConst : type not correct\n");
    }
        return *constCharArray->values;
}
SymbolTableEntry::~SymbolTableEntry() {
    delete node;
    delete handle_char;
    delete handle_int;
    delete constChar;
    delete constInt;
    delete intArray;
    delete charArray;
    delete constCharArray;
    delete constIntArray;
    delete intFunc;
    delete charFunc;
    delete voidFunc;
    delete tempEntry;
    delete definedEntry;
}

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