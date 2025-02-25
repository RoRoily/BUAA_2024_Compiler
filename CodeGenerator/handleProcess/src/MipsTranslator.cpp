//
// Created by 郑慕函 on 24-11-27.
//
#include "../include/MipsTranslator.h"
#include <fstream>
#include <string>
#include <cassert>
#include <iostream>

extern std::ofstream mipsOutput;
static bool inSelfDefinedFunc = false;

MipsTranslator::MipsTranslator(ICGenerator *icTranslator)
        : icTranslator(icTranslator) {
    regUsage = {{Reg::$zero, false},
                {Reg::$at,   false},
                {Reg::$v0,   false},
                {Reg::$v1,   false},
                {Reg::$a0,   false},
                {Reg::$a1,   false},
                {Reg::$a2,   false},
                {Reg::$a3,   false},
                {Reg::$t0,   false},
                {Reg::$t1,   false},
                {Reg::$t2,   false},
                {Reg::$t3,   false},
                {Reg::$t4,   false},
                {Reg::$t5,   false},
                {Reg::$t6,   false},
                {Reg::$t7,   false},
                {Reg::$t8,   false},
                {Reg::$t9,   false},
                {Reg::$s0,   false},
                {Reg::$s1,   false},
                {Reg::$s2,   false},
                {Reg::$s3,   false},
                {Reg::$s4,   false},
                {Reg::$s5,   false},
                {Reg::$s6,   false},
                {Reg::$s7,   false},
                {Reg::$k0,   false},
                {Reg::$k1,   false},
                {Reg::$gp,   false},
                {Reg::$sp,   false},
                {Reg::$fp,   false},
                {Reg::$ra,   false}
    };
}
std::map<QuadItemType, std::string> quadItemTypeMap2 = {
    {QuadItemType::String, "String"},
    {QuadItemType::Imm, "Imm"},
    {QuadItemType::Imc, "Imc"},
    {QuadItemType::Label, "Label"},
    {QuadItemType::IntArray, "IntArray"},
    {QuadItemType::CharArray, "CharArray"},
    {QuadItemType::Int, "Int"},
    {QuadItemType::Char, "Char"},
    {QuadItemType::Func, "Func"},
    {QuadItemType::Reference, "Reference"}
};
void MipsTranslator::translate() {
    printf("MipsTranslator : translate\n");
    std::vector<QuadEntry *> *mainStream = icTranslator->mainEntries;
    std::map<int, std::string *> *id2allPureString = icTranslator->id2allPureString;
    std::map<std::string *, QuadItemFunc *> *name2icItemFunc = icTranslator->name2quadItemFunc;
    std::list<std::string *> * quadItemFuncOrder = icTranslator->quadItemFuncOrder;
    printf("init member\n");

    int i = 0;
    const auto mainEntryNum = mainStream->size();

    // .data起始地址 0x10010000 (16进制)     268500992 (10进制)
    mipsOutput << ".data 0x10010000\n";

    mipsOutput << "temp:  .space  160000\n\n";  // 临时内存区，起始地址为0x10010000 (16) or 268500992 (10)

    // 全局变量、常量
    printf("Start analyse global decl\n");
    while (mainStream->at(i)->entryType != QuadEntryType::MainFuncStart) {
        printf("Global Decl\n");
        QuadEntry *defEntry = mainStream->at(i);
//        assert(defEntry->isGlobalVarOrConstDef());
        translate_GlobalVarOrArrayDef(defEntry);
        i++;
    }
    // 字符串片段(纯字符串部分)定义
    printf("Start analyse pure string\n");
    mipsOutput << "\n# string tokens: \n";
    for (const auto &item: *id2allPureString) {
        const int id = item.first;
        const std::string *str = item.second;
        mipsOutput << strId2label(id) << ":  .asciiz   \"" << *str << "\"\n";
    }
    // 主函数部分
    //assert(mainStream->at(i)->entryType == QuadEntryType::MainFuncStart);
    i++;
    mipsOutput << "\n\n.text 0x00400000\n\n# main function\n";
    printf("Mips MainFuncStart\n");
    while (i < mainEntryNum) {
#ifdef MIPS_DEBUG
        mipsOutput << std::flush;
#endif
        QuadEntry *entry = mainStream->at(i);
        QuadItem *op1 = entry->operator1, *op2 = entry->operator2, *op3 = entry->operator3;
        const int opNum = entry->opNum;
        switch (entry->entryType) {
            case QuadEntryType::CharDefine:{  // 局部变量
                printf("QuadEntryType : VarDefine\n");
                mipsOutput << "# local char var definition:\n";
                const bool hasInitValue = op2 != nullptr;
                auto *var = (QuadItemChar *) op1;
                localVarId2mem.insert({var->varId, tempStackAddressTop});
                tempStackAddressTop += 4;
                if (hasInitValue) {
                    auto *rightValue = (QuadItemChar *) op2;
                    lw(Reg::$t0, rightValue);
                    sw(Reg::$t0, var);
                }
                break;
            }
            case QuadEntryType::IntDefine: {  // 局部变量
                printf("QuadEntryType : VarDefine\n");
                mipsOutput << "# local int var definition:\n";
                const bool hasInitValue = op2 != nullptr;
                auto *var = (QuadItemInt *) op1;
                localVarId2mem.insert({var->varId, tempStackAddressTop});
                tempStackAddressTop += 4;
                if (hasInitValue) {
                    auto *rightValue = (QuadItemInt *) op2;
                    lw(Reg::$t0, rightValue);
                    sw(Reg::$t0, var);
                }
                break;
            }
            case QuadEntryType::ConstCharDefine:
            {  // 局部常量
                printf("QuadEntryType : ConstVarDefine\n");
                mipsOutput << "# local const char var definition:\n";
                auto *constVar = (QuadItemChar *) op1;
                localVarId2mem.insert({constVar->varId, tempStackAddressTop});
                tempStackAddressTop += 4;
                const char initValue = constVar->value;
                li(Reg::$t0, initValue);
                sw(Reg::$t0, constVar);
                break;
            }
            case QuadEntryType::ConstIntDefine: {  // 局部常量
                printf("QuadEntryType : ConstVarDefine\n");
                mipsOutput << "# local const int var definition:\n";
                auto *constVar = (QuadItemInt *) op1;
                localVarId2mem.insert({constVar->varId, tempStackAddressTop});
                tempStackAddressTop += 4;
                const int initValue = constVar->value;
                li(Reg::$t0, initValue);
                sw(Reg::$t0, constVar);
                break;
            }
            case QuadEntryType::CharArrayDefine:
            {  // 局部数组
                printf("QuadEntryType : CharArrayDefine1\n");
                mipsOutput << "# local char array definition:\n";
                const bool hasInitValue = op2 != nullptr;
                auto *array = (QuadItemCharArray *) op1;
                const int firstAddress = tempStackAddressTop;
                localArrayId2mem.insert({array->arrayId, tempStackAddressTop});
                tempStackAddressTop += 4 * array->length;
                //printf("%d %lu\n",array->length,((QuadItemCharArray *) op2)->itemsToInitArray->size());
                if (hasInitValue) {
                    li(Reg::$t1, firstAddress);  // 数组基地址
                    printf("arraylength : %d\n",array->length);
                    for (int j = 0; j < array->length; ++j) {
                        printf("%d ",j);
                        fflush(stdout);
                        QuadItemChar *rightValue = ((QuadItemCharArray *) op2)->itemsToInitArray->at(j);
                        lw(Reg::$t0, rightValue);
                        sw(Reg::$t0, j * 4, Reg::$t1);
                    }
                }
                break;
            }
            case QuadEntryType::IntArrayDefine: {  // 局部数组
                printf("QuadEntryType : IntArrayDefine\n");
                mipsOutput << "# local int array definition:\n";
                const bool hasInitValue = op2 != nullptr;
                auto *array = (QuadItemIntArray *) op1;
                const int firstAddress = tempStackAddressTop;
                printf("INSERT arrayId:%d\n",array->arrayId);
                localArrayId2mem.insert({array->arrayId, tempStackAddressTop});
                tempStackAddressTop += 4 * array->length;
                if (hasInitValue) {
                    li(Reg::$t1, firstAddress);  // 数组基地址
                    printf("length is %d\n",array->length);
                    for (int j = 0; j < array->length; ++j) {
                        QuadItemInt *rightValue = ((QuadItemIntArray *) op2)->itemsToInitArray->at(j);
                        if(rightValue->isConst)printf("ISCONST\n");
                        lw(Reg::$t0, rightValue);
                        sw(Reg::$t0, j * 4, Reg::$t1);
                    }
                }
                break;
            }
            case QuadEntryType::ConstIntArrayDefine:{
                printf("QuadEntryType : ConstArrayDefine\n");
                mipsOutput << "# local const int array definition:\n";
                auto *array = (QuadItemIntArray *) op1;
                const int firstAddress = tempStackAddressTop;
                localArrayId2mem.insert({array->arrayId, tempStackAddressTop});
                tempStackAddressTop += 4 * array->length;

                li(Reg::$t1, firstAddress);  // 数组基地址
                for (int j = 0; j < array->length; ++j) {
                    const int rightValue = array->value[j];
                    li(Reg::$t0, rightValue);
                    sw(Reg::$t0, j * 4, Reg::$t1);
                }
                break;
            }
            case QuadEntryType::ConstCharArrayDefine: {
                printf("QuadEntryType : ConstArrayDefine\n");
                mipsOutput << "# local const char array definition:\n";
                auto *array = (QuadItemCharArray *) op1;
                const int firstAddress = tempStackAddressTop;
                localArrayId2mem.insert({array->arrayId, tempStackAddressTop});
                tempStackAddressTop += 4 * array->length;

                li(Reg::$t1, firstAddress);  // 数组基地址
                for (int j = 0; j < array->length; ++j) {
                    const char rightValue = array->value[j];
                    li(Reg::$t0, rightValue);
                    sw(Reg::$t0, j * 4, Reg::$t1);
                }
                break;
            }
            case QuadEntryType::FuncCall: {
                printf("QuadEntryType : FuncCall\n");
                auto *calledFunc = (const QuadItemFunc *) (entry->calledFunc);
                pushTempReg();
                printf("QuadEntryType : PushParms\n");
                pushParams(entry->params);
                mipsOutput << "# Call function!\n";
                jal(calledFunc);
                lw(Reg::$ra, 0, Reg::$sp);
                mipsOutput << "# Pop params\n";
                addiu(Reg::$sp, Reg::$sp, 30000);
                popTempReg();
                mipsOutput << "\n";
                break;
            }
            case QuadEntryType::FuncReturnWithValue:  // 主函数结束
            case QuadEntryType::FuncReturn:
            case QuadEntryType::MainFuncEnd:
                exit();
                break;
            case QuadEntryType::Getint: {
                printf("QuadEntryType : Getint\n");
                auto *dst = ((QuadItemInt *) op1);
                getint(dst);
                break;
            }
            case QuadEntryType::Getchar: {
                printf("QuadEntryType : Getint\n");
                auto *dst = ((QuadItemChar *) op1);
                getchar(dst);
                break;
            }
            case QuadEntryType::Print: {
                printf("QuadEntryType : Print\n");
                auto *itemString = ((QuadItemString *) op1);
                for (const auto *strItem: *(itemString->stringItems)) {
                    if (strItem->isString) {
                        //字符串项
                        printf("strItem is String\n");
                        const int strId = strItem->pureStringId;
                        printStr(strId);
                    } else if(!strItem->isChar){
                        //变量项
                        const QuadItem *icItem = strItem->varItem;
                        printf("strItem is Var type is %s\n",quadItemTypeMap2.find(icItem->type)->second.c_str());
//                        assert(icItem->type == QuadItemType::Var);
                        printInt((QuadItemInt *) icItem);
                    }else {
                        const QuadItem *icItem = strItem->varItem;
                        printf("strItem is Var type is %s\n",quadItemTypeMap2.find(icItem->type)->second.c_str());
                        //                        assert(icItem->type == QuadItemType::Var);
                        printChar((QuadItemChar *) icItem);
                    }
                }
                break;
            }
            case QuadEntryType::Assign: {
                if(op2!=nullptr) {
                    if(op1->type==QuadItemType::Char && op2->type==QuadItemType::Char) {
                        auto *left = (QuadItemChar *) op1, *right = (QuadItemChar *) op2;
                        if (right == nullptr) {  // 从函数返回后赋值
                            sw(Reg::$v0, left);
                        } else {
                            lw(Reg::$t0, right);
                            sw(Reg::$t0, left);
                        }
                        break;
                    }else if(op1->type==QuadItemType::Int && op2->type==QuadItemType::Char) {
                        auto *left = (QuadItemInt *) op1;
                        auto *right = (QuadItemChar *) op2;
                        if (right == nullptr) {  // 从函数返回后赋值
                            sw(Reg::$v0, left);
                        } else {
                            lw(Reg::$t0, right);
                            sw(Reg::$t0, left);
                        }
                        break;
                    }else if(op1->type==QuadItemType::Char && op2->type==QuadItemType::Int) {
                        auto *left = (QuadItemChar *) op1;
                        auto *right = (QuadItemInt *) op2;
                        if (right == nullptr) {  // 从函数返回后赋值
                            sw(Reg::$v0, left);
                        } else {
                            lw(Reg::$t0, right);
                            sw(Reg::$t0, left);
                        }
                        break;
                    }else {
                        auto *left = (QuadItemInt *) op1, *right = (QuadItemInt *) op2;
                        if (right == nullptr) {  // 从函数返回后赋值
                            sw(Reg::$v0, left);
                        } else {
                            lw(Reg::$t0, right);
                            sw(Reg::$t0, left);
                        }
                        break;
                    }
                }else {
                    if(op1->type==QuadItemType::Char) {
                        auto *left = (QuadItemChar *) op1;
                        auto *right = (QuadItemInt *) op2;
                        if (right == nullptr) {  // 从函数返回后赋值
                            sw(Reg::$v0, left);
                        } else {
                            lw(Reg::$t0, right);
                            sw(Reg::$t0, left);
                        }
                        break;
                    }else {
                        auto *left = (QuadItemInt *) op1, *right = (QuadItemInt *) op2;
                        if (right == nullptr) {  // 从函数返回后赋值
                            sw(Reg::$v0, left);
                        } else {
                            lw(Reg::$t0, right);
                            sw(Reg::$t0, left);
                        }
                        break;
                    }
                }
            }
            case QuadEntryType::Add: {
                printf("QuadEntryType : Add\n");
                auto *dst = (QuadItemInt *) op1, *r1 = (QuadItemInt *) op2, *r2 = (QuadItemInt *) op3;
                if (r1->isConst && r2->isConst) {
                    const int right = r1->value + r2->value;
                    li(Reg::$t0, right);
                    sw(Reg::$t0, dst);
                } else if (r1->isConst) {
                    li(Reg::$t0, r1->value);
                    lw(Reg::$t1, r2);
                    addu(Reg::$t2, Reg::$t0, Reg::$t1);
                    sw(Reg::$t2, dst);
                } else {
                    lw(Reg::$t0, r1);
                    lw(Reg::$t1, r2);
                    addu(Reg::$t2, Reg::$t0, Reg::$t1);
                    sw(Reg::$t2, dst);
                }
                break;
            }
            case QuadEntryType::Sub: {
                printf("QuadEntryType : Sub\n");
                auto *dst = (QuadItemInt *) op1, *r1 = (QuadItemInt *) op2, *r2 = (QuadItemInt *) op3;
                if (r1->isConst && r2->isConst) {
                    const int right = r1->value - r2->value;
                    li(Reg::$t0, right);
                    sw(Reg::$t0, dst);
                } else if (r1->isConst) {
                    li(Reg::$t0, r1->value);
                    lw(Reg::$t1, r2);
                    subu(Reg::$t2, Reg::$t0, Reg::$t1);
                    sw(Reg::$t2, dst);
                } else {
                    lw(Reg::$t0, r1);
                    lw(Reg::$t1, r2);
                    subu(Reg::$t2, Reg::$t0, Reg::$t1);
                    sw(Reg::$t2, dst);
                }
                break;
            }
            case QuadEntryType::Mul: {

                auto *dst = (QuadItemInt *) op1, *r1 = (QuadItemInt *) op2, *r2 = (QuadItemInt *) op3;
                if (r1->isConst && r2->isConst) {
                    const int right = r1->value * r2->value;
                    li(Reg::$t0, right);
                    sw(Reg::$t0, dst);
                } else if (r1->isConst) {
                    li(Reg::$t0, r1->value);
                    lw(Reg::$t1, r2);
                    mul(Reg::$t2, Reg::$t0, Reg::$t1);
                    sw(Reg::$t2, dst);
                } else {
                    lw(Reg::$t0, r1);
                    lw(Reg::$t1, r2);
                    mul(Reg::$t2, Reg::$t0, Reg::$t1);
                    sw(Reg::$t2, dst);
                }
                break;
            }
            case QuadEntryType::Div: {
                auto *dst = (QuadItemInt *) op1, *r1 = (QuadItemInt *) op2, *r2 = (QuadItemInt *) op3;
                if (r1->isConst && r2->isConst) {
                    const int right = r1->value / r2->value;
                    li(Reg::$t0, right);
                    sw(Reg::$t0, dst);
                } else if (r1->isConst) {
                    li(Reg::$t0, r1->value);
                    lw(Reg::$t1, r2);
                    div(Reg::$t0, Reg::$t1);
                    mflo(Reg::$t2);
                    sw(Reg::$t2, dst);
                } else {
                    lw(Reg::$t0, r1);
                    lw(Reg::$t1, r2);
                    div(Reg::$t0, Reg::$t1);
                    mflo(Reg::$t2);
                    sw(Reg::$t2, dst);
                }
                break;
            }
            case QuadEntryType::Mod: {
                auto *dst = (QuadItemInt *) op1, *r1 = (QuadItemInt *) op2, *r2 = (QuadItemInt *) op3;
                if (r1->isConst && r2->isConst) {
                    const int right = r1->value % r2->value;
                    li(Reg::$t0, right);
                    sw(Reg::$t0, dst);
                } else if (r1->isConst) {
                    li(Reg::$t0, r1->value);
                    lw(Reg::$t1, r2);
                    div(Reg::$t0, Reg::$t1);
                    mfhi(Reg::$t2);
                    sw(Reg::$t2, dst);
                } else {
                    lw(Reg::$t0, r1);
                    lw(Reg::$t1, r2);
                    div(Reg::$t0, Reg::$t1);
                    mfhi(Reg::$t2);
                    sw(Reg::$t2, dst);
                }
                break;
            }
            case QuadEntryType::Neg: {
                auto *dst = (QuadItemInt *) op1, *src = (QuadItemInt *) op2;
                if (src->isConst) {
                    li(Reg::$t0, -src->value);
                    sw(Reg::$t0, dst);
                } else {
                    lw(Reg::$t0, src);
                    subu(Reg::$t0, Reg::$zero, Reg::$t0);
                    sw(Reg::$t0, dst);
                }
                break;
            }
            case QuadEntryType::Not: {
                auto *dst = (QuadItemInt *) op1, *src = (QuadItemInt *) op2;
                if (src->isConst) {
                    assert(op2->type == QuadItemType::Int||op2->type == QuadItemType::Char);
                    li(Reg::$t0, !src->value);
                    sw(Reg::$t0, dst);
                } else {
                    lw(Reg::$t0, src);
                    seq(Reg::$t0, Reg::$zero, Reg::$t0);
                    sw(Reg::$t0, dst);
                }
                break;
            }
            case QuadEntryType::Or: {
                auto *dst = (QuadItemInt *) op1, *r1 = (QuadItemInt *) op2, *r2 = (QuadItemInt *) op3;
                if (r1->isConst && r2->isConst) {
                    const int right = r1->value || r2->value;
                    li(Reg::$t0, right);
                    sw(Reg::$t0, dst);
                } else if (r1->isConst) {
                    li(Reg::$t0, r1->value);
                    lw(Reg::$t1, r2);
                    _or(Reg::$t2, Reg::$t0, Reg::$t1);
                    sw(Reg::$t2, dst);
                } else {
                    lw(Reg::$t0, r1);
                    lw(Reg::$t1, r2);
                    _or(Reg::$t2, Reg::$t0, Reg::$t1);
                    sw(Reg::$t2, dst);
                }
                break;
            }
            case QuadEntryType::And: {
                auto *dst = (QuadItemInt *) op1, *r1 = (QuadItemInt *) op2, *r2 = (QuadItemInt *) op3;
                if (r1->isConst && r2->isConst) {
                    const int right = r1->value && r2->value;
                    li(Reg::$t0, right);
                    sw(Reg::$t0, dst);
                } else if (r1->isConst) {
                    li(Reg::$t0, r1->value);
                    lw(Reg::$t1, r2);
                    _and(Reg::$t2, Reg::$t0, Reg::$t1);
                    sw(Reg::$t2, dst);
                } else {
                    lw(Reg::$t0, r1);
                    lw(Reg::$t1, r2);
                    _and(Reg::$t2, Reg::$t0, Reg::$t1);
                    sw(Reg::$t2, dst);
                }
                break;
            }
            case QuadEntryType::Equal: {
                printf("QuadEntryType : Equal\n");
                auto *dst = (QuadItemInt *) op1, *r1 = (QuadItemInt *) op2, *r2 = (QuadItemInt *) op3;
                if (r1->isConst && r2->isConst) {
                    const int right = r1->value == r2->value;
                    li(Reg::$t0, right);
                    sw(Reg::$t0, dst);
                } else if (r1->isConst) {
                    li(Reg::$t0, r1->value);
                    lw(Reg::$t1, r2);
                    seq(Reg::$t2, Reg::$t0, Reg::$t1);
                    sw(Reg::$t2, dst);
                } else {
                    lw(Reg::$t0, r1);
                    lw(Reg::$t1, r2);
                    seq(Reg::$t2, Reg::$t0, Reg::$t1);
                    sw(Reg::$t2, dst);
                }
                break;
            }
            case QuadEntryType::NotEqual: {
                auto *dst = (QuadItemInt *) op1, *r1 = (QuadItemInt *) op2, *r2 = (QuadItemInt *) op3;
                if (r1->isConst && r2->isConst) {
                    const int right = r1->value != r2->value;
                    li(Reg::$t0, right);
                    sw(Reg::$t0, dst);
                } else if (r1->isConst) {
                    li(Reg::$t0, r1->value);
                    lw(Reg::$t1, r2);
                    sne(Reg::$t2, Reg::$t0, Reg::$t1);
                    sw(Reg::$t2, dst);
                } else {
                    lw(Reg::$t0, r1);
                    lw(Reg::$t1, r2);
                    sne(Reg::$t2, Reg::$t0, Reg::$t1);
                    sw(Reg::$t2, dst);
                }
                break;
            }
            case QuadEntryType::LessEqual: {
                auto *dst = (QuadItemInt *) op1, *r1 = (QuadItemInt *) op2, *r2 = (QuadItemInt *) op3;
                if (r1->isConst && r2->isConst) {
                    const int right = r1->value <= r2->value;
                    li(Reg::$t0, right);
                    sw(Reg::$t0, dst);
                } else if (r1->isConst) {
                    li(Reg::$t0, r1->value);
                    lw(Reg::$t1, r2);
                    sle(Reg::$t2, Reg::$t0, Reg::$t1);
                    sw(Reg::$t2, dst);
                } else {
                    lw(Reg::$t0, r1);
                    lw(Reg::$t1, r2);
                    sle(Reg::$t2, Reg::$t0, Reg::$t1);
                    sw(Reg::$t2, dst);
                }
                break;
            }
            case QuadEntryType::LessThan: {
                auto *dst = (QuadItemInt *) op1, *r1 = (QuadItemInt *) op2, *r2 = (QuadItemInt *) op3;
                if (r1->isConst && r2->isConst) {
                    const int right = r1->value < r2->value;
                    li(Reg::$t0, right);
                    sw(Reg::$t0, dst);
                } else if (r1->isConst) {
                    li(Reg::$t0, r1->value);
                    lw(Reg::$t1, r2);
                    slt(Reg::$t2, Reg::$t0, Reg::$t1);
                    sw(Reg::$t2, dst);
                } else {
                    lw(Reg::$t0, r1);
                    lw(Reg::$t1, r2);
                    slt(Reg::$t2, Reg::$t0, Reg::$t1);
                    sw(Reg::$t2, dst);
                }
                break;
            }
            case QuadEntryType::GreaterThan: {
                auto *dst = (QuadItemInt *) op1, *r1 = (QuadItemInt *) op2, *r2 = (QuadItemInt *) op3;
                if (r1->isConst && r2->isConst) {
                    const int right = r1->value > r2->value;
                    li(Reg::$t0, right);
                    sw(Reg::$t0, dst);
                } else if (r1->isConst) {
                    li(Reg::$t0, r1->value);
                    lw(Reg::$t1, r2);
                    sgt(Reg::$t2, Reg::$t0, Reg::$t1);
                    sw(Reg::$t2, dst);
                } else {
                    lw(Reg::$t0, r1);
                    lw(Reg::$t1, r2);
                    sgt(Reg::$t2, Reg::$t0, Reg::$t1);
                    sw(Reg::$t2, dst);
                }
                break;
            }
            case QuadEntryType::GreaterEqual: {
                auto *dst = (QuadItemInt *) op1, *r1 = (QuadItemInt *) op2, *r2 = (QuadItemInt *) op3;
                if (r1->isConst && r2->isConst) {
                    const int right = r1->value >= r2->value;
                    li(Reg::$t0, right);
                    sw(Reg::$t0, dst);
                } else if (r1->isConst) {
                    li(Reg::$t0, r1->value);
                    lw(Reg::$t1, r2);
                    sge(Reg::$t2, Reg::$t0, Reg::$t1);
                    sw(Reg::$t2, dst);
                } else {
                    lw(Reg::$t0, r1);
                    lw(Reg::$t1, r2);
                    sge(Reg::$t2, Reg::$t0, Reg::$t1);
                    sw(Reg::$t2, dst);
                }
                break;
            }
            case QuadEntryType::Beqz: {
                auto *condition = (QuadItemInt *) op1;
                auto *label = (QuadItemLabel *) op2;
                if (condition->isConst) {
                    if (condition->value == 0) {
                        j(label);
                    }
                } else {
                    lw(Reg::$t0, condition);
                    beqz(Reg::$t0, label);
                }
                break;
            }
            case QuadEntryType::Bnez: {
                auto *condition = (QuadItemInt *) op1;
                auto *label = (QuadItemLabel *) op2;
                if (condition->isConst) {
                    if (condition->value != 0) {
                        j(label);
                    }
                } else {
                    lw(Reg::$t0, condition);
                    bnez(Reg::$t0, label);
                }
                break;
            }
            case QuadEntryType::JumpLabel: {
                auto *label = (QuadItemLabel *) op1;
                j(label);
                break;
            }
            case QuadEntryType::InsertLabel:
                insertLabel((QuadItemLabel *) op1);
                break;
//            case QuadEntryType::FuncDefine:
//            case QuadEntryType::MainFuncStart:
        }
        i++;
    }
    printf("MipsGenerator : MainEntries finish\n");
    inSelfDefinedFunc = true;
    mipsOutput << "\n\n\n\n" << "# self defined functions\n\n";
    //函数定义
    for (const auto &item: *quadItemFuncOrder) {
        QuadItemFunc *func = name2icItemFunc->find(item)->second;
        mipsOutput << "# ---------------- FUCNTION: " << *func->originName << " ------------------\n";
        mipsOutput << func->funcLabel->toString() << ":\n\n";
        translate_FuncDef(func);
        mipsOutput << "\n\n\n";
    }
#ifdef MIPS_DEBUG
    mipsOutput << std::flush;
#endif
}

void MipsTranslator::translate_FuncDef(QuadItemFunc *func) {
    inSelfDefinedFunc = true;
    clearLocalAndTempInFunc();
    funcFArrayParamId2offset.clear();
    funcFVarParamId2offset.clear();
    std::vector<QuadItem *> *params = func->params;
    const int num = params->size();

    int offset = 4;  // offset = 0处放 ra
    for (const QuadItem *param: *params) {
        if (param->type == QuadItemType::Int || param->type==QuadItemType::Char) {
            auto *var = ((QuadItemInt *) param);
            funcFVarParamId2offset.insert({var->varId, offset});  // varId 为负数
            offset += 4;
        } else {
            auto *array = (QuadItemIntArray *) param;
            funcFArrayParamId2offset.insert({array->arrayId, offset});  // arrayId 为正数
            offset += 4;
        }
    }
    tempFuncStackOffsetTop = tempFuncStackOffsetBase + offset;

    std::vector<QuadEntry *> *funcEntries = func->entries;
    int i = 0;
    bool findReturn = false;
    while (i < funcEntries->size()) {
#ifdef MIPS_DEBUG
        mipsOutput << std::flush;
#endif
        QuadEntry *entry = funcEntries->at(i);
        QuadItem *op1 = entry->operator1, *op2 = entry->operator2, *op3 = entry->operator3;
        switch (entry->entryType) {
            case QuadEntryType::CharDefine:{  // 局部变量
                mipsOutput << "# local char var definition:\n";
                const bool hasInitValue = op2 != nullptr;
                auto *var = (QuadItemChar *) op1;
                localVarId2offset.insert({var->varId, tempFuncStackOffsetTop});
                tempFuncStackOffsetTop += 4;
                if (hasInitValue) {
                    auto *rightValue = (QuadItemChar *) op2;
                    lw(Reg::$t0, rightValue);
                    sw(Reg::$t0, var);
                }
                break;
            }
            case QuadEntryType::IntDefine: {  // 局部变量
                mipsOutput << "# local int int var definition:\n";
                const bool hasInitValue = op2 != nullptr;
                auto *var = (QuadItemInt *) op1;
                localVarId2offset.insert({var->varId, tempFuncStackOffsetTop});
                tempFuncStackOffsetTop += 4;
                if (hasInitValue) {
                    auto *rightValue = (QuadItemInt *) op2;
                    lw(Reg::$t0, rightValue);
                    sw(Reg::$t0, var);
                }
                break;
            }
            case QuadEntryType::ConstIntDefine:{  // 局部常量
                mipsOutput << "# local const int var definition:\n";
                auto *constVar = (QuadItemInt *) op1;
                localVarId2offset.insert({constVar->varId, tempFuncStackOffsetTop});
                tempFuncStackOffsetTop += 4;
                const int initValue = constVar->value;
                li(Reg::$t0, initValue);
                sw(Reg::$t0, constVar);
                break;
            }
            case QuadEntryType::ConstCharDefine: {  // 局部常量
                mipsOutput << "# local const char var definition:\n";
                auto *constVar = (QuadItemChar *) op1;
                localVarId2offset.insert({constVar->varId, tempFuncStackOffsetTop});
                tempFuncStackOffsetTop += 4;
                const char initValue = constVar->value;
                li(Reg::$t0, initValue);
                sw(Reg::$t0, constVar);
                break;
            }
            case QuadEntryType::IntArrayDefine:{  // 局部数组
                mipsOutput << "# local int array definition:\n";
                const bool hasInitValue = op2 != nullptr;
                auto *array = (QuadItemIntArray *) op1;
                const int firstAddress = tempFuncStackOffsetTop;
                localArrayId2offset.insert({array->arrayId, tempFuncStackOffsetTop});
                tempFuncStackOffsetTop += 4 * array->length;
                if (hasInitValue) {
                    //                    addiu(Reg::$t1, Reg::$sp, firstAddress);  // 数组基地址
                    ////                  li(Reg::$t1, firstAddress);
                    for (int j = 0; j < array->length; ++j) {
                        QuadItemInt *rightValue = ((QuadItemIntArray *) op2)->itemsToInitArray->at(j);
                        lw(Reg::$t0, rightValue);
                        sw(Reg::$t0, j * 4 + firstAddress, Reg::$sp);
                    }
                }
                break;
            }
            case QuadEntryType::CharArrayDefine: {  // 局部数组
                mipsOutput << "# local char array definition:\n";
                const bool hasInitValue = op2 != nullptr;
                auto *array = (QuadItemCharArray *) op1;
                printf("FuncDef : CharArrayDefine DefinedLong : %d\n",array->length);
                const int firstAddress = tempFuncStackOffsetTop;
                localArrayId2offset.insert({array->arrayId, tempFuncStackOffsetTop});
                tempFuncStackOffsetTop += 4 * array->length;
                if (hasInitValue) {
//                    addiu(Reg::$t1, Reg::$sp, firstAddress);  // 数组基地址
////                  li(Reg::$t1, firstAddress);
                    for (int j = 0; j < array->length; ++j) {
                        QuadItemChar *rightValue = ((QuadItemCharArray *) op2)->itemsToInitArray->at(j);
                        lw(Reg::$t0, rightValue);
                        sw(Reg::$t0, j * 4 + firstAddress, Reg::$sp);
                    }
                }
                break;
            }
            case QuadEntryType::ConstIntArrayDefine:{
                mipsOutput << "# local const int array definition:\n";
                auto *array = (QuadItemIntArray *) op1;
                const int firstAddress = tempFuncStackOffsetTop;
                localArrayId2offset.insert({array->arrayId, tempFuncStackOffsetTop});
                tempFuncStackOffsetTop += 4 * array->length;

                //                addiu(Reg::$t1, Reg::$sp, firstAddress);  // 数组基地址
                ////                li(Reg::$t1, firstAddress);
                for (int j = 0; j < array->length; ++j) {
                    const int rightValue = array->value[j];
                    li(Reg::$t0, rightValue);
                    sw(Reg::$t0, j * 4 + firstAddress, Reg::$sp);
                }
                break;
            }
            case QuadEntryType::ConstCharArrayDefine: {
                mipsOutput << "# local const char array definition:\n";
                auto *array = (QuadItemCharArray *) op1;
                const int firstAddress = tempFuncStackOffsetTop;
                localArrayId2offset.insert({array->arrayId, tempFuncStackOffsetTop});
                tempFuncStackOffsetTop += 4 * array->length;

//                addiu(Reg::$t1, Reg::$sp, firstAddress);  // 数组基地址
////                li(Reg::$t1, firstAddress);
                for (int j = 0; j < array->length; ++j) {
                    const char rightValue = array->value[j];
                    li(Reg::$t0, rightValue);
                    sw(Reg::$t0, j * 4 + firstAddress, Reg::$sp);
                }
                break;
            }
                // case QuadEntryType::FuncDefine:  // 非法，不应该出现这个
            case QuadEntryType::FuncCall: {
                auto *calledFunc = (const QuadItemFunc *) (entry->calledFunc);
                pushTempReg();
                pushParams(entry->params);
                mipsOutput << "# Call function!\n";
                jal(calledFunc);
                lw(Reg::$ra, 0, Reg::$sp);
                mipsOutput << "# Pop params\n";
                addiu(Reg::$sp, Reg::$sp, 30000);
                popTempReg();
                mipsOutput << "\n";
                break;
            }
            case QuadEntryType::FuncReturnWithValue: {
                printf("FuncDef: QuadEntryType :: FuncReturnWithValue : %s\n",quadItemTypeMap2.find(op1->type)->second.c_str());
                findReturn = true;
                if (op1->type == QuadItemType::Imm) {
                    li(Reg::$v0, ((QuadItemImm *) op1)->value);
                } else {
                    lw(Reg::$v0, ((QuadItemInt *) op1));
                }
                jr();
                break;
            }
            case QuadEntryType::FuncReturn:
                findReturn = true;
                // case QuadEntryType::MainFuncEnd:
                jr();
                break;
            case QuadEntryType::Getint: {
                auto *dst = ((QuadItemInt *) op1);
                getint(dst);
                break;
            }
            case QuadEntryType::Print: {
                auto *itemString = ((QuadItemString *) op1);
                for (const auto *strItem: *(itemString->stringItems)) {
                    //String
                    if (strItem->isString) {
                        const int strId = strItem->pureStringId;
                        printStr(strId);
                    }
                    //Int
                    else if(!strItem->isChar){
                        const QuadItem *icItem = strItem->varItem;
//                        assert(icItem->type == QuadItemType::Var);
                        printInt((QuadItemInt *) icItem);
                    }
                    //Char
                    else {
                        const QuadItem *icItem = strItem->varItem;
                        //                        assert(icItem->type == QuadItemType::Var);
                        printChar((QuadItemChar *) icItem);
                    }
                }
                break;
            }
            case QuadEntryType::Assign: {
                if(op2!=nullptr) {
                    if(op1->type==QuadItemType::Char && op2->type==QuadItemType::Char) {
                        auto *left = (QuadItemChar *) op1, *right = (QuadItemChar *) op2;
                        if (right == nullptr) {  // 从函数返回后赋值
                            sw(Reg::$v0, left);
                        } else {
                            lw(Reg::$t0, right);
                            sw(Reg::$t0, left);
                        }
                        break;
                    }else if(op1->type==QuadItemType::Int && op2->type==QuadItemType::Char) {
                        auto *left = (QuadItemInt *) op1;
                        auto *right = (QuadItemChar *) op2;
                        if (right == nullptr) {  // 从函数返回后赋值
                            sw(Reg::$v0, left);
                        } else {
                            lw(Reg::$t0, right);
                            sw(Reg::$t0, left);
                        }
                        break;
                    }else if(op1->type==QuadItemType::Char && op2->type==QuadItemType::Int) {
                        auto *left = (QuadItemChar *) op1;
                        auto *right = (QuadItemInt *) op2;
                        if (right == nullptr) {  // 从函数返回后赋值
                            sw(Reg::$v0, left);
                        } else {
                            lw(Reg::$t0, right);
                            sw(Reg::$t0, left);
                        }
                        break;
                    }else {
                        auto *left = (QuadItemInt *) op1, *right = (QuadItemInt *) op2;
                        if (right == nullptr) {  // 从函数返回后赋值
                            sw(Reg::$v0, left);
                        } else {
                            lw(Reg::$t0, right);
                            sw(Reg::$t0, left);
                        }
                        break;
                    }
                }else {
                    if(op1->type==QuadItemType::Char) {
                        auto *left = (QuadItemChar *) op1;
                        auto *right = (QuadItemInt *) op2;
                        if (right == nullptr) {  // 从函数返回后赋值
                            sw(Reg::$v0, left);
                        } else {
                            lw(Reg::$t0, right);
                            sw(Reg::$t0, left);
                        }
                        break;
                    }else {
                        auto *left = (QuadItemInt *) op1, *right = (QuadItemInt *) op2;
                        if (right == nullptr) {  // 从函数返回后赋值
                            sw(Reg::$v0, left);
                        } else {
                            lw(Reg::$t0, right);
                            sw(Reg::$t0, left);
                        }
                        break;
                    }
                }
            }
            case QuadEntryType::Add: {
                auto *dst = (QuadItemInt *) op1, *r1 = (QuadItemInt *) op2, *r2 = (QuadItemInt *) op3;
                if (r1->isConst && r2->isConst) {
                    const int right = r1->value + r2->value;
                    li(Reg::$t0, right);
                    sw(Reg::$t0, dst);
                } else if (r1->isConst) {
                    li(Reg::$t0, r1->value);
                    lw(Reg::$t1, r2);
                    addu(Reg::$t2, Reg::$t0, Reg::$t1);
                    sw(Reg::$t2, dst);
                } else {
                    lw(Reg::$t0, r1);
                    lw(Reg::$t1, r2);
                    addu(Reg::$t2, Reg::$t0, Reg::$t1);
                    sw(Reg::$t2, dst);
                }
                break;
            }
            case QuadEntryType::Sub: {
                auto *dst = (QuadItemInt *) op1, *r1 = (QuadItemInt *) op2, *r2 = (QuadItemInt *) op3;
                if (r1->isConst && r2->isConst) {
                    const int right = r1->value - r2->value;
                    li(Reg::$t0, right);
                    sw(Reg::$t0, dst);
                } else if (r1->isConst) {
                    li(Reg::$t0, r1->value);
                    lw(Reg::$t1, r2);
                    subu(Reg::$t2, Reg::$t0, Reg::$t1);
                    sw(Reg::$t2, dst);
                } else {
                    lw(Reg::$t0, r1);
                    lw(Reg::$t1, r2);
                    subu(Reg::$t2, Reg::$t0, Reg::$t1);
                    sw(Reg::$t2, dst);
                }
                break;
            }
            case QuadEntryType::Mul: {
                auto *dst = (QuadItemInt *) op1, *r1 = (QuadItemInt *) op2, *r2 = (QuadItemInt *) op3;
                if (r1->isConst && r2->isConst) {
                    const int right = r1->value * r2->value;
                    li(Reg::$t0, right);
                    sw(Reg::$t0, dst);
                } else if (r1->isConst) {
                    li(Reg::$t0, r1->value);
                    lw(Reg::$t1, r2);
                    mul(Reg::$t2, Reg::$t0, Reg::$t1);
                    sw(Reg::$t2, dst);
                } else {
                    lw(Reg::$t0, r1);
                    lw(Reg::$t1, r2);
                    mul(Reg::$t2, Reg::$t0, Reg::$t1);
                    sw(Reg::$t2, dst);
                }
                break;
            }
            case QuadEntryType::Div: {
                auto *dst = (QuadItemInt *) op1, *r1 = (QuadItemInt *) op2, *r2 = (QuadItemInt *) op3;
                if (r1->isConst && r2->isConst) {
                    const int right = r1->value / r2->value;
                    li(Reg::$t0, right);
                    sw(Reg::$t0, dst);
                } else if (r1->isConst) {
                    li(Reg::$t0, r1->value);
                    lw(Reg::$t1, r2);
                    div(Reg::$t0, Reg::$t1);
                    mflo(Reg::$t2);
                    sw(Reg::$t2, dst);
                } else {
                    lw(Reg::$t0, r1);
                    lw(Reg::$t1, r2);
                    div(Reg::$t0, Reg::$t1);
                    mflo(Reg::$t2);
                    sw(Reg::$t2, dst);
                }
                break;
            }
            case QuadEntryType::Mod: {
                auto *dst = (QuadItemInt *) op1, *r1 = (QuadItemInt *) op2, *r2 = (QuadItemInt *) op3;
                if (r1->isConst && r2->isConst) {
                    const int right = r1->value % r2->value;
                    li(Reg::$t0, right);
                    sw(Reg::$t0, dst);
                } else if (r1->isConst) {
                    li(Reg::$t0, r1->value);
                    lw(Reg::$t1, r2);
                    div(Reg::$t0, Reg::$t1);
                    mfhi(Reg::$t2);
                    sw(Reg::$t2, dst);
                } else {
                    lw(Reg::$t0, r1);
                    lw(Reg::$t1, r2);
                    div(Reg::$t0, Reg::$t1);
                    mfhi(Reg::$t2);
                    sw(Reg::$t2, dst);
                }
                break;
            }
            case QuadEntryType::Not: {
                auto *dst = (QuadItemInt *) op1, *src = (QuadItemInt *) op2;
                if (src->isConst) {
                    //assert(op2->type == QuadItemType::Int || op2->type == QuadItemType::Char);
                    li(Reg::$t0, !src->value);
                    sw(Reg::$t0, dst);
                } else {
                    lw(Reg::$t0, src);
                    seq(Reg::$t0, Reg::$zero, Reg::$t0);
                    sw(Reg::$t0, dst);
                }
                break;
            }
            case QuadEntryType::Neg: {
                auto *dst = (QuadItemInt *) op1, *src = (QuadItemInt *) op2;
                if (src->isConst) {
                    li(Reg::$t0, -src->value);
                    sw(Reg::$t0, dst);
                } else {
                    lw(Reg::$t0, src);
                    subu(Reg::$t0, Reg::$zero, Reg::$t0);
                    sw(Reg::$t0, dst);
                }
                break;
            }
//            case QuadEntryType::FuncDefine:
//                break;
//            case QuadEntryType::MainFuncStart:
//                break;
//            case QuadEntryType::MainFuncEnd:
//                break;
            case QuadEntryType::Or: {
                auto *dst = (QuadItemInt *) op1, *r1 = (QuadItemInt *) op2, *r2 = (QuadItemInt *) op3;
                if (r1->isConst && r2->isConst) {
                    const int right = r1->value || r2->value;
                    li(Reg::$t0, right);
                    sw(Reg::$t0, dst);
                } else if (r1->isConst) {
                    li(Reg::$t0, r1->value);
                    lw(Reg::$t1, r2);
                    _or(Reg::$t2, Reg::$t0, Reg::$t1);
                    sw(Reg::$t2, dst);
                } else {
                    lw(Reg::$t0, r1);
                    lw(Reg::$t1, r2);
                    _or(Reg::$t2, Reg::$t0, Reg::$t1);
                    sw(Reg::$t2, dst);
                }
                break;
            }
            case QuadEntryType::And: {
                auto *dst = (QuadItemInt *) op1, *r1 = (QuadItemInt *) op2, *r2 = (QuadItemInt *) op3;
                if (r1->isConst && r2->isConst) {
                    const int right = r1->value && r2->value;
                    li(Reg::$t0, right);
                    sw(Reg::$t0, dst);
                } else if (r1->isConst) {
                    li(Reg::$t0, r1->value);
                    lw(Reg::$t1, r2);
                    _and(Reg::$t2, Reg::$t0, Reg::$t1);
                    sw(Reg::$t2, dst);
                } else {
                    lw(Reg::$t0, r1);
                    lw(Reg::$t1, r2);
                    _and(Reg::$t2, Reg::$t0, Reg::$t1);
                    sw(Reg::$t2, dst);
                }
                break;
            }
            case QuadEntryType::Equal: {
                auto *dst = (QuadItemInt *) op1, *r1 = (QuadItemInt *) op2, *r2 = (QuadItemInt *) op3;
                if (r1->isConst && r2->isConst) {
                    const int right = r1->value == r2->value;
                    li(Reg::$t0, right);
                    sw(Reg::$t0, dst);
                } else if (r1->isConst) {
                    li(Reg::$t0, r1->value);
                    lw(Reg::$t1, r2);
                    seq(Reg::$t2, Reg::$t0, Reg::$t1);
                    sw(Reg::$t2, dst);
                } else {
                    lw(Reg::$t0, r1);
                    lw(Reg::$t1, r2);
#ifdef MIPS_DEBUG
                    mipsOutput << std::flush;
#endif
                    seq(Reg::$t2, Reg::$t0, Reg::$t1);
                    sw(Reg::$t2, dst);
                }
                break;
            }
            case QuadEntryType::NotEqual: {
                auto *dst = (QuadItemInt *) op1, *r1 = (QuadItemInt *) op2, *r2 = (QuadItemInt *) op3;
                if (r1->isConst && r2->isConst) {
                    const int right = r1->value != r2->value;
                    li(Reg::$t0, right);
                    sw(Reg::$t0, dst);
                } else if (r1->isConst) {
                    li(Reg::$t0, r1->value);
                    lw(Reg::$t1, r2);
                    sne(Reg::$t2, Reg::$t0, Reg::$t1);
                    sw(Reg::$t2, dst);
                } else {
                    lw(Reg::$t0, r1);
                    lw(Reg::$t1, r2);
                    sne(Reg::$t2, Reg::$t0, Reg::$t1);
                    sw(Reg::$t2, dst);
                }
                break;
            }
            case QuadEntryType::LessEqual: {
                auto *dst = (QuadItemInt *) op1, *r1 = (QuadItemInt *) op2, *r2 = (QuadItemInt *) op3;
                if (r1->isConst && r2->isConst) {
                    const int right = r1->value <= r2->value;
                    li(Reg::$t0, right);
                    sw(Reg::$t0, dst);
                } else if (r1->isConst) {
                    li(Reg::$t0, r1->value);
                    lw(Reg::$t1, r2);
                    sle(Reg::$t2, Reg::$t0, Reg::$t1);
                    sw(Reg::$t2, dst);
                } else {
                    lw(Reg::$t0, r1);
                    lw(Reg::$t1, r2);
                    sle(Reg::$t2, Reg::$t0, Reg::$t1);
                    sw(Reg::$t2, dst);
                }
                break;
            }
            case QuadEntryType::LessThan: {
                auto *dst = (QuadItemInt *) op1, *r1 = (QuadItemInt *) op2, *r2 = (QuadItemInt *) op3;
                if (r1->isConst && r2->isConst) {
                    const int right = r1->value < r2->value;
                    li(Reg::$t0, right);
                    sw(Reg::$t0, dst);
                } else if (r1->isConst) {
                    li(Reg::$t0, r1->value);
                    lw(Reg::$t1, r2);
                    slt(Reg::$t2, Reg::$t0, Reg::$t1);
                    sw(Reg::$t2, dst);
                } else {
                    lw(Reg::$t0, r1);
                    lw(Reg::$t1, r2);
                    slt(Reg::$t2, Reg::$t0, Reg::$t1);
                    sw(Reg::$t2, dst);
                }
                break;
            }
            case QuadEntryType::GreaterThan: {
                auto *dst = (QuadItemInt *) op1, *r1 = (QuadItemInt *) op2, *r2 = (QuadItemInt *) op3;
                if (r1->isConst && r2->isConst) {
                    const int right = r1->value > r2->value;
                    li(Reg::$t0, right);
                    sw(Reg::$t0, dst);
                } else if (r1->isConst) {
                    li(Reg::$t0, r1->value);
                    lw(Reg::$t1, r2);
                    sgt(Reg::$t2, Reg::$t0, Reg::$t1);
                    sw(Reg::$t2, dst);
                } else {
                    lw(Reg::$t0, r1);
                    lw(Reg::$t1, r2);
                    sgt(Reg::$t2, Reg::$t0, Reg::$t1);
                    sw(Reg::$t2, dst);
                }
                break;
            }
            case QuadEntryType::GreaterEqual: {
                auto *dst = (QuadItemInt *) op1, *r1 = (QuadItemInt *) op2, *r2 = (QuadItemInt *) op3;
                if (r1->isConst && r2->isConst) {
                    const int right = r1->value >= r2->value;
                    li(Reg::$t0, right);
                    sw(Reg::$t0, dst);
                } else if (r1->isConst) {
                    li(Reg::$t0, r1->value);
                    lw(Reg::$t1, r2);
                    sge(Reg::$t2, Reg::$t0, Reg::$t1);
                    sw(Reg::$t2, dst);
                } else {
                    lw(Reg::$t0, r1);
                    lw(Reg::$t1, r2);
                    sge(Reg::$t2, Reg::$t0, Reg::$t1);
                    sw(Reg::$t2, dst);
                }
                break;
            }
            case QuadEntryType::Beqz: {
                auto *condition = (QuadItemInt *) op1;
                auto *label = (QuadItemLabel *) op2;
                if (condition->isConst) {
                    if (condition->value == 0) {
                        j(label);
                    }
                } else {
                    lw(Reg::$t0, condition);
                    beqz(Reg::$t0, label);
                }
                break;
            }
            case QuadEntryType::Bnez: {
                auto *condition = (QuadItemInt *) op1;
                auto *label = (QuadItemLabel *) op2;
                if (condition->isConst) {
                    if (condition->value != 0) {
                        j(label);
                    }
                } else {
                    lw(Reg::$t0, condition);
                    bnez(Reg::$t0, label);
                }
                break;
            }
            case QuadEntryType::JumpLabel: {
                auto *label = (QuadItemLabel *) op1;
                j(label);
                break;
            }
            case QuadEntryType::InsertLabel:
                insertLabel((QuadItemLabel *) op1);
                break;
        }
        i++;
    }
    jr();
    funcFArrayParamId2offset.clear();
    funcFVarParamId2offset.clear();
    clearLocalAndTempInFunc();
}

// 返回params个数
void MipsTranslator::pushParams(const std::vector<QuadItem *> *params) {
#ifdef MIPS_DEBUG
    mipsOutput << std::flush;
#endif
    mipsOutput << "\n\n# Pushing Function Real Params:\n";
//    const int num = params->size();
    addiu(Reg::$sp, Reg::$sp, -30000);
    sw(Reg::$ra, 0, Reg::$sp);
    int offset = 4;
    if (params == nullptr || params->empty()) return;
    for (const QuadItem *param: *params) {
        printf("Type is %s\n",quadItemTypeMap2.find(param->type)->second.c_str());
        if (param->type == QuadItemType::Imm) {
            auto *imm = (QuadItemImm *) param;
            li(Reg::$t0, imm->value);
        } else if (param->type == QuadItemType::Int ) {
            auto *var = (QuadItemInt *) param;
            if (var->isConst) {
                li(Reg::$t0, var->value);
            } else if (var->isGlobal) {
                if(var->isTemp) {
                    la(Reg::$t0, toStringWithCheck(var->tempVarId, var->isTemp, var->isGlobal, var->isConst));
                }else la(Reg::$t0, toStringWithCheck(var->varId,  var->isTemp, var->isGlobal, var->isConst));
                //la(Reg::$t0, var->toString());
                lw(Reg::$t0, 0, Reg::$t0);
            } else {
                lw(Reg::$t0, var, inSelfDefinedFunc);
            }
        }
        else if( param->type == QuadItemType::Char) {
            auto *var = (QuadItemChar *) param;
            if (var->isConst) {
                li(Reg::$t0, var->value);
            } else if (var->isGlobal) {
                if(var->isTemp) {
                    la(Reg::$t0, toStringWithCheck(var->tempVarId,  var->isTemp, var->isGlobal, var->isConst));
                }else la(Reg::$t0, toStringWithCheck(var->varId,  var->isTemp, var->isGlobal, var->isConst));
                //la(Reg::$t0, var->toString());
                lw(Reg::$t0, 0, Reg::$t0);
            } else {
                lw(Reg::$t0, var, inSelfDefinedFunc);
            }
        }
        else if (param->type == QuadItemType::IntArray ) {
            // 会在 lw 中找到 param的reference，传入数组首地址
            lw(Reg::$t0, (QuadItemInt *) param, inSelfDefinedFunc);
        }else if(param->type == QuadItemType::CharArray) {
            lw(Reg::$t0, (QuadItemChar *) param, inSelfDefinedFunc);
        }
        sw(Reg::$t0, offset, Reg::$sp);
        offset += 4;
    }

    mipsOutput << "# Finished Pushing Params!\n";
}


void MipsTranslator::translate_GlobalVarOrArrayDef(QuadEntry *defEntry) {
    printf("GlobaVarOrArrayDef\n");
    if (defEntry->entryType == QuadEntryType::ConstIntDefine ) {
        // 全局常量定义
        auto *constVar = (QuadItemInt *) (defEntry->operator1);
        const int initValue = constVar->value;
        mipsOutput << "\n# " << (*constVar->originalName) << "\n";
        mipsOutput << constVar->toString() << ":  .word  " << initValue << "\n";
        tagTable.push_back(constVar->toString());
    }
    else if(defEntry->entryType == QuadEntryType::ConstCharDefine) {
        // 全局常量定义
        auto *constVar = (QuadItemChar *) (defEntry->operator1);
        const char initValue = constVar->value;
        mipsOutput << "\n# " << (*constVar->originalName) << "\n";
        if(initValue == '\0') {
            mipsOutput << constVar->toString() << ":  .word  " << 0 << "\n";
        }
        else if(initValue == '\\') {
            mipsOutput << constVar->toString() << ":  .word  " << 92 << "\n";
        }else if(initValue == '\'') {
            mipsOutput << constVar->toString() << ":  .word  " << 39 << "\n";
        }else if(initValue == '\"') {
            mipsOutput << constVar->toString() << ":  .word  " << 34 << "\n";
        }else if(initValue == '\f') {
            mipsOutput << constVar->toString() << ":  .word  " << 12 << "\n";
        }else if(initValue == '\v') {
            mipsOutput << constVar->toString() << ":  .word  " << 11 << "\n";
        }else if(initValue == '\n') {
            mipsOutput << constVar->toString() << ":  .word  " << 10 << "\n";
        }else if(initValue == '\t') {
            mipsOutput << constVar->toString() << ":  .word  " << 9 << "\n";
        }else if(initValue == '\b') {
            mipsOutput << constVar->toString() << ":  .word  " << 8 << "\n";
        }else if(initValue == '\a') {
            mipsOutput << constVar->toString() << ":  .word  " << 7 << "\n";
        }else
            mipsOutput << constVar->toString() << ":  .word  '" << initValue << "'\n";
        //mipsOutput << constVar->toString() << ":  .word  '" << initValue << "'\n";
        tagTable.push_back(constVar->toString());
    }
    else if (defEntry->entryType == QuadEntryType::ConstIntArrayDefine ) {
        // 全局常量数组定义
        printf("global Array Def\n");
        auto *constArray = ((QuadItemIntArray *) (defEntry->operator1));
        const int d = constArray->originType.d,
                length1 = constArray->originType.length1,
                length2 = constArray->originType.length2;
        int *  initValues = constArray->value;
        const int length = constArray->length;
        printf("d = %d,length1 = %d,length2 = %d,length = %d\n",d,length1,length2,length);
        if (d == 1) {
            mipsOutput << "\n# " << (*constArray->originalName) << "[" << length1 << "]" << "\n";
        } else {
            mipsOutput << "\n# " << (*constArray->originalName) << "[" << length1 << "]"
                       << "[" << length2 << "]" << "\n";
        }
        mipsOutput << constArray->toString() << ":  .word  ";
        tagTable.push_back(constArray->toString());
        for (int i = 0; i < length - 1; ++i) {
            mipsOutput << initValues[i] << ", ";
        }
        printf("initValues[length-1] : %d\n",initValues[length-1]);
        mipsOutput << initValues[length - 1] << "\n";
                
    }

else if ( defEntry->entryType == QuadEntryType::ConstCharArrayDefine) {
        // 全局常量数组定义
        printf("global Array Def\n");
        auto *constArray = ((QuadItemCharArray *) (defEntry->operator1));
        const int d = constArray->originType.d,
                length1 = constArray->originType.length1,
                length2 = constArray->originType.length2;
        std::string initValues = constArray->value;
        const int length = constArray->length;

        bool has_bracket = true;
        if(initValues.at(0)!='\"'||initValues.at(initValues.length()-1)!='\"')has_bracket = false;
        // 全局变量数组定义
        auto *array = ((QuadItemCharArray *) (defEntry->operator1));
        if (d == 1) {
            mipsOutput << "\n# " << (*array->originalName) << "[" << length1 << "]" << "\n";
        }
        mipsOutput << array->toString() << ":  .word  ";
        tagTable.push_back(array->toString());
        int count_i = 0,analyse_end = 1,count = 0;
        if(has_bracket){count_i = 1;analyse_end = 2;}
        for (; count_i < length - analyse_end; ++count_i) {
            count++;
            if(array->value[count_i] == '\0') {
                mipsOutput  << '0' << ", ";
            }
            else if(array->value[count_i] == '\\') {
                mipsOutput  << "92" << ", ";
            }else if(array->value[count_i] == '\'') {
                mipsOutput << "39" << ", ";
            }else if(array->value[count_i] == '\"') {
                mipsOutput  << "34" << ", ";
            }else if(array->value[count_i] == '\f') {
                mipsOutput  << "12" << ", ";
            }else if(array->value[count_i] == '\v') {
                mipsOutput  << "11" << ", ";
            }else if(array->value[count_i] == '\n') {
                mipsOutput  << "10" << ", ";
            }else if(array->value[count_i] == '\t') {
                mipsOutput  << "9" << ", ";
            }else if(array->value[count_i] == '\b') {
                mipsOutput << "8" << ", ";
            }else if(array->value[count_i] == '\a') {
                mipsOutput << "7" << ", ";
            }
            else
            mipsOutput <<"'"<< array->value[count_i] << "', ";
        }
        if(array->value[length -analyse_end] == '\0') {
            mipsOutput  << '0' << "\n";
        }
        else if(array->value[length -analyse_end] == '\\') {
            mipsOutput  << "92" << "\n";
        }else if(array->value[length -analyse_end] == '\'') {
            mipsOutput << "39" << "\n";
        }else if(array->value[length -analyse_end] == '\"') {
            mipsOutput  << "34" << "\n";
        }else if(array->value[length -analyse_end] == '\f') {
            mipsOutput  << "12" << "\n";
        }else if(array->value[length -analyse_end] == '\v') {
            mipsOutput  << "11" << "\n";
        }else if(array->value[length -analyse_end] == '\n') {
            mipsOutput  << "10" << "\n";
        }else if(array->value[length -analyse_end] == '\t') {
            mipsOutput  << "9" << "\n";
        }else if(array->value[length -analyse_end] == '\b') {
            mipsOutput << "8" << "\n";
        }else if(array->value[length -analyse_end] == '\a') {
            mipsOutput << "7" << "\n";
        }else
        mipsOutput << "'"<<array->value[length - analyse_end] << "'\n";
        //mipsOutput << initValues[length - 1] << "\n";
        count++;
        if(length-count!=0)
        mipsOutput << "     .space " <<(length-count )* 4<<"\n";

    }
    else if (defEntry->entryType == QuadEntryType::IntDefine ) {
        // 全局变量定义
        auto *var = ((QuadItemInt *) (defEntry->operator1));
        const int initValue = var->value;
        mipsOutput << "\n#" << (*var->originalName) << "\n";
        mipsOutput << var->toString() << ": .word  " << initValue << "\n";
        tagTable.push_back(var->toString());
    }
    else if(defEntry->entryType==QuadEntryType::CharDefine) {
        // 全局变量定义
        auto *var = ((QuadItemChar *) (defEntry->operator1));
        const char initValue = var->value;
        mipsOutput << "\n#" << (*var->originalName) << "\n";
        if(initValue == '\0') {
            mipsOutput << var->toString() << ": .word  " << '0' << "\n";
        }
        else if(initValue == '\\') {
            mipsOutput << var->toString() << ": .word  " << "92" << "\n";
        }else if(initValue == '\'') {
            mipsOutput << var->toString() << ": .word  " << "39" << "\n";
        }else if(initValue == '\"') {
            mipsOutput << var->toString() << ": .word  " << "34" << "\n";
        }else if(initValue == '\f') {
            mipsOutput << var->toString() << ": .word  " << "12" << "\n";
        }else if(initValue == '\v') {
            mipsOutput << var->toString() << ": .word  " << "11" << "\n";
        }else if(initValue == '\n') {
            mipsOutput << var->toString() << ": .word  " << "10" << "\n";
        }else if(initValue == '\t') {
            mipsOutput << var->toString() << ": .word  " << "9" << "\n";
        }else if(initValue == '\b') {
            mipsOutput << var->toString() << ": .word  " << "8" << "\n";
        }else if(initValue == '\a') {
            mipsOutput << var->toString() << ": .word  " << "7" << "\n";
        }
        else
        mipsOutput << var->toString() << ": .word  '" << initValue << "'\n";
        tagTable.push_back(var->toString());
    }
    else if (defEntry->entryType == QuadEntryType::IntArrayDefine ) {
        // 全局变量数组定义
        auto *array = ((QuadItemIntArray *) (defEntry->operator1));
        const int d = array->originType.d,
                length1 = array->originType.length1,
                length2 = array->originType.length2;
        const int *initValues = array->value;
        const int length = array->length;
        if (d == 1) {
            mipsOutput << "\n# " << (*array->originalName) << "[" << length1 << "]" << "\n";
        } else {
            mipsOutput << "\n# " << (*array->originalName) << "[" << length1 << "]"
                       << "[" << length2 << "]" << "\n";
        }
        mipsOutput << array->toString() << ":  .word  ";
        tagTable.push_back(array->toString());
        for (int i = 0; i < length - 1; ++i) {
            mipsOutput << initValues[i] << ", ";
        }
        mipsOutput << initValues[length - 1] << "\n";
#ifdef MIPS_DEBUG
        mipsOutput << std::flush;
#endif
    }else if ( defEntry->entryType == QuadEntryType::CharArrayDefine) {
        // 全局变量数组定义
        auto *array = ((QuadItemCharArray *) (defEntry->operator1));
        const int d = array->originType.d,
                length1 = array->originType.length1,
                length2 = array->originType.length2;
        std:: string initValues = array->value;
        const int length = array->length;
        if (d == 1) {
            mipsOutput << "\n# " << (*array->originalName) << "[" << length1 << "]" << "\n";
        } else {
            mipsOutput << "\n# " << (*array->originalName) << "[" << length1 << "]"
                       << "[" << length2 << "]" << "\n";
        }
        mipsOutput << array->toString() << ":  .word  ";
        tagTable.push_back(array->toString());
        for (int i = 0; i < length - 1; ++i) {
            if(array->value[i] == '\0') {
                mipsOutput  << '0' << ", ";
            }
            else if(array->value[i] == '\\') {
                mipsOutput  << "92" << ", ";
            }else if(array->value[i] == '\'') {
                mipsOutput << "39" << ", ";
            }else if(array->value[i] == '\"') {
                mipsOutput  << "34" << ", ";
            }else if(array->value[i] == '\f') {
                mipsOutput  << "12" << ", ";
            }else if(array->value[i] == '\v') {
                mipsOutput  << "11" << ", ";
            }else if(array->value[i] == '\n') {
                mipsOutput  << "10" << ", ";
            }else if(array->value[i] == '\t') {
                mipsOutput  << "9" << ", ";
            }else if(array->value[i] == '\b') {
                mipsOutput << "8" << ", ";
            }else if(array->value[i] == '\a') {
                mipsOutput << "7" << ", ";
            }
            else
            mipsOutput <<"'"<< array->value[i] << "', ";
        }
        if(array->value[length -1] == '\0') {
            mipsOutput  << '0' << "\n";
        }
        else if(array->value[length -1] == '\\') {
            mipsOutput  << "92" << "\n";
        }else if(array->value[length -1] == '\'') {
            mipsOutput << "39" << "\n";
        }else if(array->value[length -1] == '\"') {
            mipsOutput  << "34" << "\n";
        }else if(array->value[length -1] == '\f') {
            mipsOutput  << "12" << "\n";
        }else if(array->value[length -1] == '\v') {
            mipsOutput  << "11" << "\n";
        }else if(array->value[length -1] == '\n') {
            mipsOutput  << "10" << "\n";
        }else if(array->value[length -1] == '\t') {
            mipsOutput  << "9" << "\n";
        }else if(array->value[length -1] == '\b') {
            mipsOutput << "8" << "\n";
        }else if(array->value[length -1] == '\a') {
            mipsOutput << "7" << "\n";
        }else
        mipsOutput << "'"<<array->value[length - 1] << "'\n";
#ifdef MIPS_DEBUG
        mipsOutput << std::flush;
#endif
    }
}

bool MipsTranslator::isFuncFParam(QuadItemInt *var) {
    if(!(var->type == QuadItemType::Int || var->type == QuadItemType::Char))
        printf("Error in Mips:isFuncParam : type is not Int or Char,is : %s\n",quadItemTypeMap2.find(var->type)->second.c_str());
    //assert(var->type == QuadItemType::Int || var->type == QuadItemType::Char);
    return funcFVarParamId2offset.find(var->varId) != funcFVarParamId2offset.end();
}

bool MipsTranslator::isFuncFParam(QuadItemChar *var) {
    if(!(var->type == QuadItemType::Int || var->type == QuadItemType::Char))
        printf("Error in Mips:isFuncParam : type is not Int or Char,is : %s\n",quadItemTypeMap2.find(var->type)->second.c_str());
            //assert(var->type == QuadItemType::Int || var->type == QuadItemType::Char);
    return funcFVarParamId2offset.find(var->varId) != funcFVarParamId2offset.end();
}
bool MipsTranslator::isFuncFParam(QuadItemIntArray *array) {
    array = (QuadItemIntArray *) array;
    return funcFArrayParamId2offset.find(array->arrayId) != funcFArrayParamId2offset.end();
}

bool MipsTranslator::isFuncFParam(QuadItemCharArray *array) {
    array = (QuadItemCharArray *) array;
    return funcFArrayParamId2offset.find(array->arrayId) != funcFArrayParamId2offset.end();
}
void MipsTranslator::lw(Reg reg, QuadItemInt *var, bool whenPushingParamsRecursively) {
    printf("lwInt\n");
    int addr;
    QuadItem *referenceItem = var;
#ifdef MIPS_DEBUG
    mipsOutput << std::flush;
#endif
    //assert(var != nullptr);
    //当为左值时，reference一定不为0
    if (referenceItem->reference != nullptr) {
        // var 为 LVal
        ReferenceType referenceType = referenceItem->referenceType;
        referenceItem = referenceItem->reference;
        if (referenceItem->type == QuadItemType::Imm) {
            li(reg, ((QuadItemImm *) referenceItem)->value);
            return;
        } else {
            switch (referenceType) {
                case ReferenceType::Int:{
                    var = (QuadItemInt *) referenceItem;
                    if (var->isGlobal) {
                        if(var->isTemp) {
                            la(reg, toStringWithCheck(var->tempVarId,  var->isTemp, var->isGlobal, var->isConst));
                        }else la(reg, toStringWithCheck(var->varId,  var->isTemp, var->isGlobal, var->isConst));
                        //la(reg, var->toString());
                        lw(reg, 0, reg);
                    } else if (isFuncFParam(var)) {
                        addr = funcFVarParamId2offset.find(var->varId)->second;
                        if (whenPushingParamsRecursively) {
                            addr += 30000;
                        }
                        lw(reg, addr, Reg::$sp);
                    } else if (var->isConst) {
                        li(reg, var->value);
                    } else {
                        if (inSelfDefinedFunc) {
                            if (var->isTemp) {
                                addr = tempVarId2offset.find(var->tempVarId)->second;
                            } else {
                                addr = localVarId2offset.find(var->varId)->second;
                            }
                            if (whenPushingParamsRecursively) {
                                addr += 30000;
                            }
                            lw(reg, addr, Reg::$sp);
                        } else {
                            if (var->isTemp) {
                                addr = tempVarId2mem.find(var->tempVarId)->second;
                            } else {
                                addr = localVarId2mem.find(var->varId)->second;
                            }
                            lw(reg, addr, Reg::$zero);
                        }
                    }
                    return;
                }
                case ReferenceType::Char: {
                    var = (QuadItemInt *) referenceItem;
                    if (var->isGlobal) {
                        if(var->isTemp) {
                            la(reg, toStringWithCheck(var->tempVarId,  var->isTemp, var->isGlobal, var->isConst));
                        }else la(reg, toStringWithCheck(var->varId,  var->isTemp, var->isGlobal, var->isConst));
                        //la(reg, var->toString());
                        lw(reg, 0, reg);
                    } else if (isFuncFParam(var)) {
                        addr = funcFVarParamId2offset.find(var->varId)->second;
                        if (whenPushingParamsRecursively) {
                            addr += 30000;
                        }
                        lw(reg, addr, Reg::$sp);
                    } else if (var->isConst) {
                        li(reg, var->value);
                    } else {
                        if (inSelfDefinedFunc) {
                            if (var->isTemp) {
                                addr = tempVarId2offset.find(var->tempVarId)->second;
                            } else {
                                addr = localVarId2offset.find(var->varId)->second;
                            }
                            if (whenPushingParamsRecursively) {
                                addr += 30000;
                            }
                            lw(reg, addr, Reg::$sp);
                        } else {
                            if (var->isTemp) {
                                addr = tempVarId2mem.find(var->tempVarId)->second;
                            } else {
                                addr = localVarId2mem.find(var->varId)->second;
                            }
                            lw(reg, addr, Reg::$zero);
                        }
                    }
                    return;
                }
                case ReferenceType::CharArray_Var:
                case ReferenceType::IntArray_Var: {
                    QuadItem *offsetItem = var->array1_var_index;
                    Reg temp;
                    if (reg == Reg::$t7) temp = Reg::$t6;
                    else temp = Reg::$t7;
                    lw(temp, (QuadItemInt *) offsetItem, whenPushingParamsRecursively);  // $t9 = 数组下标
                    sll(temp, Reg::$t7, 2);  // t7 = t7 * 4
                    auto array1Item = (QuadItemIntArray *) referenceItem;
                    if (array1Item->isGlobal) {
                        if(array1Item->isTemp) {
                            la(reg, toStringWithCheck_Array(array1Item->tempArrayId, array1Item->isTemp, array1Item->isGlobal, array1Item->isConst));
                        }else la(reg, toStringWithCheck_Array(array1Item->arrayId, array1Item->isTemp, array1Item->isGlobal, array1Item->isConst));
                        //la(reg, array1Item->toString());
                        addu(reg, reg, temp);  // reg = reg + $t7 = 基地址 + 偏移
                        lw(reg, 0, reg);
                    } else if (isFuncFParam(array1Item)) {
                        addr = funcFArrayParamId2offset.find(array1Item->arrayId)->second;
                        if (whenPushingParamsRecursively) {
                            addr += 30000;
                        }
                        // TODO:!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                        lw(reg, addr, Reg::$sp);
                        addu(reg, reg, temp);
                        lw(reg, 0, reg);
                    } else {
                        if (inSelfDefinedFunc) {
                            if (array1Item->isTemp) {
                                addr = tempArrayId2offset.find(array1Item->tempArrayId)->second;
                            } else {
                                addr = localArrayId2offset.find(array1Item->arrayId)->second;
                            }
                            if (whenPushingParamsRecursively) {
                                addr += 30000;
                            }
                            addiu(Reg::$t8, Reg::$sp, addr);
                            addu(temp, Reg::$t8, temp);
//                            lw(reg, addr, Reg::$sp);
//                            addu(reg, reg, Reg::$t7);
                            lw(reg, 0, temp);
                        } else {
                            if (array1Item->isTemp) {
                                addr = tempArrayId2mem.find(array1Item->tempArrayId)->second;
                            } else {
                                addr = localArrayId2mem.find(array1Item->arrayId)->second;
                            }
                            addiu(reg, temp, addr);
                            lw(reg, 0, reg);
                        }
                    }
                    return;
                }
                case ReferenceType::IntArray:
                case ReferenceType::CharArray: {
                    auto arrayItem = (QuadItemIntArray *) referenceItem;
//                    mipsOutput << "# 实参类型 Array2\n";
                    if (arrayItem->isGlobal) {
                        if(arrayItem->isTemp) {
                            la(reg, toStringWithCheck_Array(arrayItem->tempArrayId, arrayItem->isTemp, arrayItem->isGlobal, arrayItem->isConst));
                        }else la(reg, toStringWithCheck_Array(arrayItem->arrayId, arrayItem->isTemp, arrayItem->isGlobal, arrayItem->isConst));
                        //la(reg, arrayItem->toString());
                    } else if (isFuncFParam(arrayItem)) {
                        addr = funcFArrayParamId2offset.find(arrayItem->arrayId)->second;
                        if (whenPushingParamsRecursively) {
                            addr += 30000;
                        }
                        lw(reg, addr, Reg::$sp);
                    } else {
                        if (inSelfDefinedFunc) {
                            if (arrayItem->isTemp) {
                                addr = tempArrayId2offset.find(arrayItem->tempArrayId)->second;
                            } else {
                                addr = localArrayId2offset.find(arrayItem->arrayId)->second;
                            }
                            if (whenPushingParamsRecursively) {
                                addr += 30000;
                            }
                            addiu(reg, Reg::$sp, addr);
                        } else {
                            if (arrayItem->isTemp) {
                                addr = tempArrayId2mem.find(arrayItem->tempArrayId)->second;
                            } else {
                                addr = localArrayId2mem.find(arrayItem->arrayId)->second;
                            }
                            li(reg, addr);
                        }
                    }
                    return;
                }
                case ReferenceType::Unset:
                    throw std::runtime_error("\nError in MipsTranslator.cpp, line " +
                                             std::to_string(__LINE__) +
                                             ": reaching illegal cases\n");
            }
        }
        return;  // 处理完所有 LVal的情况
    }
    // 非 LVal
    if (var->isGlobal) {
        if(var->isTemp) {
            la(reg, toStringWithCheck(var->tempVarId,  var->isTemp,var->isGlobal,var->isConst));
        }else la(reg, toStringWithCheck(var->varId,  var->isTemp,var->isGlobal,var->isConst));
        //la(reg, var->toString());
        lw(reg, 0, reg);
        return;
    }
    if (var->isConst) {
        li(reg, var->value);
        return;
    }
    if (isFuncFParam(var)) {
        addr = funcFVarParamId2offset.find(var->varId)->second;
        if (whenPushingParamsRecursively) {
            addr += 30000;
        }
        lw(reg, addr, Reg::$sp);
        return;
    }
    if (inSelfDefinedFunc) {
        if (var->isTemp) {
            addr = tempVarId2offset.find(var->tempVarId)->second;
        } else {
            addr = localVarId2offset.find(var->varId)->second;
        }
        if (whenPushingParamsRecursively) {
            addr += 30000;
        }
        lw(reg, addr, Reg::$sp);
    } else {
        if (var->isTemp) {
            printf("LLL\n");
            addr = tempVarId2mem.find(var->tempVarId)->second;
            printf("Check VarId : %d addr : %d\n",var->tempVarId,addr);
        } else {
            addr = localVarId2mem.find(var->varId)->second;
            printf("2Check VarId : %d addr : %d\n",var->tempVarId,addr);
        }
        lw(reg, addr, Reg::$zero);
    }
}

void MipsTranslator::lw(Reg dst, int offset, Reg base) {
    mipsOutput << "lw " << reg2s.find(dst)->second << ", " <<
               offset << "(" << reg2s.find(base)->second << ")\n";
}

void MipsTranslator::sw(Reg reg, QuadItemInt *dst) {
    printf("GOTO IntSw\n");
    int addr;
    QuadItem *dstLVal = dst;
    // [Exp];
    // 例： 1-1;
    if (dst == nullptr) return;
    if (dstLVal->reference != nullptr) {
        // dst 是 LVal
        ReferenceType referenceType = dstLVal->referenceType;
        dstLVal = dstLVal->reference;

        switch (referenceType) {
            case ReferenceType::Char:
            case ReferenceType::Int: {
                dst = (QuadItemInt *) dstLVal;
                if (dst->isGlobal) {
                    if(dst->isTemp) {
                        la(Reg::$t9, toStringWithCheck(dst->tempVarId, dst->isTemp,dst->isGlobal,dst->isConst));
                    }else la(Reg::$t9, toStringWithCheck(dst->varId, dst->isTemp,dst->isGlobal,dst->isConst));
                    //la(Reg::$t9, dst->toString());
                    sw(reg, 0, Reg::$t9);
                    return;
                }
                if (isFuncFParam(dst)) {
                    addr = funcFVarParamId2offset.find(dst->varId)->second;
                    sw(reg, addr, Reg::$sp);
                    return;
                }
                if (dst->isTemp) {
                    if (inSelfDefinedFunc) {
                        if (tempVarId2offset.find(dst->tempVarId) == tempVarId2offset.end()) {
                            tempVarId2offset.insert({dst->tempVarId, tempFuncStackOffsetTop});
                            addr = tempFuncStackOffsetTop;
                            tempFuncStackOffsetTop += 4;
                        } else {
                            addr = tempVarId2offset.find(dst->tempVarId)->second;
                        }
                        sw(reg, addr, Reg::$sp);
                    } else {
                        if (tempVarId2mem.find(dst->tempVarId) == tempVarId2mem.end()) {
                            printf("dst->tempVarId : %d 1sw int tempStackAddressTop = %d\n",dst->tempVarId,tempStackAddressTop);
                            tempVarId2mem.insert({dst->tempVarId, tempStackAddressTop});
                            addr = tempStackAddressTop;
                            tempStackAddressTop += 4;
                        } else {
                            addr = tempVarId2mem.find(dst->tempVarId)->second;
                        }
                        sw(reg, addr, Reg::$zero);
                    }
                } else {
                    if (inSelfDefinedFunc) {
                        if (localVarId2offset.find(dst->varId) == localVarId2offset.end()) {
                            localVarId2offset.insert({dst->varId, tempFuncStackOffsetTop});
                            addr = tempFuncStackOffsetTop;
                            tempFuncStackOffsetTop += 4;
                        } else {
                            addr = localVarId2offset.find(dst->varId)->second;
                        }
                        sw(reg, addr, Reg::$sp);
                    } else {
                        if (localVarId2mem.find(dst->varId) == localVarId2mem.end()) {
                            localVarId2mem.insert({dst->varId, tempStackAddressTop});
                            addr = tempStackAddressTop;
                            tempStackAddressTop += 4;
                        } else {
                            addr = localVarId2mem.find(dst->varId)->second;
                        }
                        sw(reg, addr, Reg::$zero);
                    }
                }
                return;
            }
            case ReferenceType::CharArray_Var:{  // 存到一维数组的某个元素里
                QuadItem *offsetItem = dst->array1_var_index;
                lw(Reg::$t9, (QuadItemChar *) offsetItem);  // $t9 = 数组下标
                sll(Reg::$t9, Reg::$t9, 2);  // t9 = t9 * 4 偏移量
                auto array1Item = (QuadItemCharArray *) dstLVal;
                if (array1Item->isGlobal) {
                    if(array1Item->isTemp) {
                        la(Reg::$t8, toStringWithCheck_Array(array1Item->tempArrayId, array1Item->isTemp, array1Item->isGlobal, array1Item->isConst));
                    }else la(Reg::$t8, toStringWithCheck_Array(array1Item->arrayId, array1Item->isTemp, array1Item->isGlobal, array1Item->isConst));
                    //la(Reg::$t8, array1Item->toString());  // t8 = 基地址
                    addu(Reg::$t9, Reg::$t8, Reg::$t9);  // t9 = t8 + t9 = 基地址 + 偏移
                    sw(reg, 0, Reg::$t9);
                } else if (isFuncFParam(array1Item)) {
                    addr = funcFArrayParamId2offset.find(array1Item->arrayId)->second;
                    lw(Reg::$t8, addr, Reg::$sp);  // t8: 数组实际的首地址
                    addu(Reg::$t9, Reg::$t9, Reg::$t8);  // t9:偏移, t8: 首地址
                    sw(reg, 0, Reg::$t9);
                } else {
                    if (inSelfDefinedFunc) {
                        if (array1Item->isTemp) {
                            addr = tempArrayId2offset.find(array1Item->tempArrayId)->second;
                        } else {
                            addr = localArrayId2offset.find(array1Item->arrayId)->second;
                        }
                        addiu(Reg::$t9, Reg::$t9, addr);
                        addu(Reg::$t9, Reg::$t9, Reg::$sp);
//                        lw(Reg::$t8, addr, Reg::$sp);  // t8 = 数组首地址
//                        addu(Reg::$t9, Reg::$t9, Reg::$t8);
                        sw(reg, 0, Reg::$t9);
                    } else {
                        if (array1Item->isTemp) {
                            addr = tempArrayId2mem.find(array1Item->tempArrayId)->second;
                        } else {
                            addr = localArrayId2mem.find(array1Item->arrayId)->second;
                        }
                        addiu(Reg::$t9, Reg::$t9, addr);  // t9:偏移, addr:基地址
                        sw(reg, 0, Reg::$t9);
                    }
                }
                return;
            }
            case ReferenceType::IntArray_Var: {  // 存到一维数组的某个元素里
                QuadItem *offsetItem = dst->array1_var_index;
                lw(Reg::$t9, (QuadItemInt *) offsetItem);  // $t9 = 数组下标
                sll(Reg::$t9, Reg::$t9, 2);  // t9 = t9 * 4 偏移量
                auto array1Item = (QuadItemIntArray *) dstLVal;
                if (array1Item->isGlobal) {
                    if(array1Item->isTemp) {
                        la(Reg::$t8, toStringWithCheck_Array(array1Item->tempArrayId, array1Item->isTemp, array1Item->isGlobal, array1Item->isConst));
                    }else la(Reg::$t8, toStringWithCheck_Array(array1Item->arrayId, array1Item->isTemp, array1Item->isGlobal, array1Item->isConst));
                    //la(Reg::$t8, array1Item->toString());  // t8 = 基地址
                    addu(Reg::$t9, Reg::$t8, Reg::$t9);  // t9 = t8 + t9 = 基地址 + 偏移
                    sw(reg, 0, Reg::$t9);
                } else if (isFuncFParam(array1Item)) {
                    addr = funcFArrayParamId2offset.find(array1Item->arrayId)->second;
                    lw(Reg::$t8, addr, Reg::$sp);  // t8: 数组实际的首地址
                    addu(Reg::$t9, Reg::$t9, Reg::$t8);  // t9:偏移, t8: 首地址
                    sw(reg, 0, Reg::$t9);
                } else {
                    if (inSelfDefinedFunc) {
                        if (array1Item->isTemp) {
                            addr = tempArrayId2offset.find(array1Item->tempArrayId)->second;
                        } else {
                            addr = localArrayId2offset.find(array1Item->arrayId)->second;
                        }
                        addiu(Reg::$t9, Reg::$t9, addr);
                        addu(Reg::$t9, Reg::$t9, Reg::$sp);
//                        lw(Reg::$t8, addr, Reg::$sp);  // t8 = 数组首地址
//                        addu(Reg::$t9, Reg::$t9, Reg::$t8);
                        sw(reg, 0, Reg::$t9);
                    } else {
                        if (array1Item->isTemp) {
                            addr = tempArrayId2mem.find(array1Item->tempArrayId)->second;
                        } else {
                            addr = localArrayId2mem.find(array1Item->arrayId)->second;
                        }
                        addiu(Reg::$t9, Reg::$t9, addr);  // t9:偏移, addr:基地址
                        sw(reg, 0, Reg::$t9);
                    }
                }
                return;
            }
            case ReferenceType::IntArray:
            case ReferenceType::CharArray:
            case ReferenceType::Unset:
                throw std::runtime_error("\nError in MipsTranslator.cpp, line " +
                                         std::to_string(__LINE__) +
                                         ": reaching illegal cases\n");
        }
        return;  // 处理完所有 LVal的情况
    }
    if (dst->isGlobal) {
        if(dst->isTemp) {
            la(Reg::$t9, toStringWithCheck(dst->tempVarId, dst->isTemp,dst->isGlobal,dst->isConst));
        }else la(Reg::$t9, toStringWithCheck(dst->varId, dst->isTemp,dst->isGlobal,dst->isConst));
        la(Reg::$t9, dst->toString());
        sw(reg, 0, Reg::$t9);
        return;
    }
    if (isFuncFParam(dst)) {
        addr = funcFVarParamId2offset.find(dst->varId)->second;
        sw(reg, addr, Reg::$sp);
        return;
    }
    if (dst->isTemp) {
        if (inSelfDefinedFunc) {
            if (tempVarId2offset.find(dst->tempVarId) == tempVarId2offset.end()) {
                tempVarId2offset.insert({dst->tempVarId, tempFuncStackOffsetTop});
                addr = tempFuncStackOffsetTop;
                tempFuncStackOffsetTop += 4;
            } else {
                addr = tempVarId2offset.find(dst->tempVarId)->second;
            }
            sw(reg, addr, Reg::$sp);
        } else {
            if (tempVarId2mem.find(dst->tempVarId) == tempVarId2mem.end()) {
                printf("dst->tempVarId : %d 2int tempStackAddressTop = %d\n",dst->tempVarId,tempStackAddressTop);
                tempVarId2mem.insert({dst->tempVarId, tempStackAddressTop});
                addr = tempStackAddressTop;
                tempStackAddressTop += 4;
            } else {
                addr = tempVarId2mem.find(dst->tempVarId)->second;
            }
            sw(reg, addr, Reg::$zero);
        }
    } else {
        if (inSelfDefinedFunc) {
            if (localVarId2offset.find(dst->varId) == localVarId2offset.end()) {
                localVarId2offset.insert({dst->varId, tempFuncStackOffsetTop});
                addr = tempFuncStackOffsetTop;
                tempFuncStackOffsetTop += 4;
            } else {
                addr = localVarId2offset.find(dst->varId)->second;
            }
            sw(reg, addr, Reg::$sp);
        } else {
            if (localVarId2mem.find(dst->varId) == localVarId2mem.end()) {
                localVarId2mem.insert({dst->varId, tempStackAddressTop});
                addr = tempStackAddressTop;
                tempStackAddressTop += 4;
            } else {
                addr = localVarId2mem.find(dst->varId)->second;
            }
            sw(reg, addr, Reg::$zero);
        }
    }
}

void MipsTranslator::sw(Reg src, int offset, Reg base) {
    mipsOutput << "sw " << reg2s.find(src)->second << ", " <<
               std::to_string(offset) << "(" << reg2s.find(base)->second << ")\n";
}

void MipsTranslator::li(Reg reg, int imm) {
    mipsOutput << "li " << reg2s.find(reg)->second << ", " << std::to_string(imm) << "\n";
}

void MipsTranslator::move(Reg dst, Reg src) {
    mipsOutput << "move " << reg2s.find(dst)->second << ", " << reg2s.find(src)->second << "\n";
}

void MipsTranslator::pushTempReg() {  // t0 - t9, ra
    return;
//    mipsOutput << "\n\n# store temp regs\n";
//    addiu(Reg::$sp, Reg::$sp, -44);
//    int offset = 0;
//    for (const Reg reg: tempRegs) {
//        sw(reg, offset, Reg::$sp);
//        offset += 4;
//    }
//    sw(Reg::$ra, offset, Reg::$sp);
}

void MipsTranslator::popTempReg() {
    return;
//    mipsOutput << "\n\n# recover temp regs\n";
//    int offset = 0;
//    for (const Reg reg: tempRegs) {
//        lw(reg, offset, Reg::$sp);
//        offset += 4;
//    }
//    lw(Reg::$ra, offset, Reg::$sp);
//    addiu(Reg::$sp, Reg::$sp, 44);
//    mipsOutput << "\n\n";
}

void MipsTranslator::addiu(Reg dst, Reg src, int i) {
    mipsOutput << "addiu " << reg2s.find(dst)->second << ", "
               << reg2s.find(src)->second << ", " << std::to_string(i) << "\n";
}

void MipsTranslator::jal(const QuadItemFunc *calledFunc) {
    mipsOutput << "jal " << calledFunc->funcLabel->toString() << "\n";
}

void MipsTranslator::j(QuadItemLabel *label) {
    mipsOutput << "j " << label->toString() << "\n";
}

void MipsTranslator::beqz(Reg cond, QuadItemLabel *label) {
    mipsOutput << "beqz " << reg2s.find(cond)->second << ", " << label->toString() << "\n";
}

void MipsTranslator::bnez(Reg cond, QuadItemLabel *label) {
    mipsOutput << "bnez " << reg2s.find(cond)->second << ", " << label->toString() << "\n";
}

void MipsTranslator::exit() {
    mipsOutput << "\nli $v0, 10\n";
    syscall();
}

void MipsTranslator::getint(QuadItemInt *dst) {
    mipsOutput << "\nli $v0, 5\n";
    syscall();
    sw(Reg::$v0, dst);
}

void MipsTranslator::getchar(QuadItemChar *dst) {
    mipsOutput << "\nli $v0, 12\n";
    syscall();
    sw(Reg::$v0, dst);
}

void MipsTranslator::printStr(int strId) {
    la(Reg::$a0, strId2label(strId));
    li(Reg::$v0, 4);
    syscall();
}

void MipsTranslator::la(Reg reg, const std::string &label) {
    mipsOutput << "la " << reg2s.find(reg)->second << ", " + label << "\n";
}

std::string MipsTranslator::strId2label(int strId) {
    return "str_" + std::to_string(strId);
}

void MipsTranslator::syscall() {
    mipsOutput << "syscall\n";
}

void MipsTranslator::printInt(QuadItemInt *var) {
    lw(Reg::$a0, var);
    li(Reg::$v0, 1);
    syscall();
}

void MipsTranslator::printChar(QuadItemChar *var) {
    lw(Reg::$a0, var);
    li(Reg::$v0, 11);
    syscall();
}

void MipsTranslator::addu(Reg rd, Reg rs, Reg rt) {
    mipsOutput << "addu " + reg2s.find(rd)->second <<
               ", " << reg2s.find(rs)->second <<
               ", " << reg2s.find(rt)->second << "\n";
}

void MipsTranslator::subu(Reg rd, Reg rs, Reg rt) {
    mipsOutput << "subu " + reg2s.find(rd)->second <<
               ", " << reg2s.find(rs)->second <<
               ", " << reg2s.find(rt)->second << "\n";
}

void MipsTranslator::mul(Reg rd, Reg rs, Reg rt) {
    mipsOutput << "mul " + reg2s.find(rd)->second <<
               ", " << reg2s.find(rs)->second <<
               ", " << reg2s.find(rt)->second << "\n";
}

void MipsTranslator::mul(Reg dst, Reg srcReg, int srcNum) {
    mipsOutput << "mul " + reg2s.find(dst)->second <<
               ", " << reg2s.find(srcReg)->second <<
               ", " << srcNum << "\n";
}

void MipsTranslator::div(Reg rs, Reg rt) {
    mipsOutput << "div " + reg2s.find(rs)->second <<
               ", " << reg2s.find(rt)->second << "\n";
}

void MipsTranslator::mfhi(Reg reg) {
    mipsOutput << "mfhi " + reg2s.find(reg)->second << "\n";
}

void MipsTranslator::mflo(Reg reg) {
    mipsOutput << "mflo " + reg2s.find(reg)->second << "\n";
}

void MipsTranslator::jr() {
    mipsOutput << "jr $ra\n";
}

void MipsTranslator::sll(Reg dst, Reg src, int bits) {
    mipsOutput << "sll " << reg2s.find(dst)->second << ", "
               << reg2s.find(src)->second << ", " << bits << '\n';
}

void MipsTranslator::sne(Reg rd, Reg rs, Reg rt) {
    mipsOutput << "sne " + reg2s.find(rd)->second <<
               ", " << reg2s.find(rs)->second <<
               ", " << reg2s.find(rt)->second << "\n";
}

void MipsTranslator::seq(Reg rd, Reg rs, Reg rt) {
    mipsOutput << "seq " + reg2s.find(rd)->second <<
               ", " << reg2s.find(rs)->second <<
               ", " << reg2s.find(rt)->second << "\n";
}

void MipsTranslator::sle(Reg rd, Reg rs, Reg rt) {
    mipsOutput << "sle " + reg2s.find(rd)->second <<
               ", " << reg2s.find(rs)->second <<
               ", " << reg2s.find(rt)->second << "\n";
}

void MipsTranslator::slt(Reg rd, Reg rs, Reg rt) {
    mipsOutput << "slt " + reg2s.find(rd)->second <<
               ", " << reg2s.find(rs)->second <<
               ", " << reg2s.find(rt)->second << "\n";
}

void MipsTranslator::sge(Reg rd, Reg rs, Reg rt) {
    mipsOutput << "sge " + reg2s.find(rd)->second <<
               ", " << reg2s.find(rs)->second <<
               ", " << reg2s.find(rt)->second << "\n";
}

void MipsTranslator::sgt(Reg rd, Reg rs, Reg rt) {
    mipsOutput << "sgt " + reg2s.find(rd)->second <<
               ", " << reg2s.find(rs)->second <<
               ", " << reg2s.find(rt)->second << "\n";
}

void MipsTranslator::_or(Reg rd, Reg rs, Reg rt) {
    // 按位或，可以用来代替或
    mipsOutput << "or " + reg2s.find(rd)->second <<
               ", " << reg2s.find(rs)->second <<
               ", " << reg2s.find(rt)->second << "\n";
}

void MipsTranslator::_and(Reg rd, Reg rs, Reg rt) {
    // 注意：此处 _and 为逻辑与，不是按位与！！！！！！！！！！！！！！！！
    sne(rs, rs, Reg::$zero);
    sne(rt, rt, Reg::$zero);
    mipsOutput << "and " + reg2s.find(rd)->second <<
               ", " << reg2s.find(rs)->second <<
               ", " << reg2s.find(rt)->second << "\n";
}

void MipsTranslator::insertLabel(QuadItemLabel *label) {
    mipsOutput << label->toString() << ":\n";
}

void MipsTranslator::clearLocalAndTempInFunc() {
    localVarId2offset.clear();
    localArrayId2offset.clear();
    tempVarId2offset.clear();
    tempArrayId2offset.clear();
}

void MipsTranslator::sw(Reg reg, QuadItemChar *dst) {
    printf("GOTO CharSw\n");
    int addr;
    QuadItem *dstLVal = dst;
    // [Exp];
    // 例： 1-1;
    if (dst == nullptr) return;
    if (dstLVal->reference != nullptr) {
        // dst 是 LVal
        ReferenceType referenceType = dstLVal->referenceType;
        dstLVal = dstLVal->reference;

        switch (referenceType) {
            case ReferenceType::Char:
            case ReferenceType::Int: {
                dst = (QuadItemChar *) dstLVal;
                if (dst->isGlobal) {
                    if(dst->isTemp) {
                        la(Reg::$t9, toStringWithCheck(dst->tempVarId, dst->isTemp,dst->isGlobal,dst->isConst));
                    }else la(Reg::$t9, toStringWithCheck(dst->varId, dst->isTemp,dst->isGlobal,dst->isConst));
                    //la(Reg::$t9, dst->toString());
                    sw(reg, 0, Reg::$t9);
                    return;
                }
                if (isFuncFParam(dst)) {
                    addr = funcFVarParamId2offset.find(dst->varId)->second;
                    sw(reg, addr, Reg::$sp);
                    return;
                }
                if (dst->isTemp) {
                    if (inSelfDefinedFunc) {
                        if (tempVarId2offset.find(dst->tempVarId) == tempVarId2offset.end()) {
                            tempVarId2offset.insert({dst->tempVarId, tempFuncStackOffsetTop});
                            addr = tempFuncStackOffsetTop;
                            tempFuncStackOffsetTop += 4;
                        } else {
                            addr = tempVarId2offset.find(dst->tempVarId)->second;
                        }
                        sw(reg, addr, Reg::$sp);
                    } else {
                        if (tempVarId2mem.find(dst->tempVarId) == tempVarId2mem.end()) {
                            printf("dst->tempVarId : %d char tempStackAddressTop = %d\n",dst->tempVarId,tempStackAddressTop);
                            tempVarId2mem.insert({dst->tempVarId, tempStackAddressTop});
                            addr = tempStackAddressTop;
                            tempStackAddressTop += 4;
                        } else {
                            addr = tempVarId2mem.find(dst->tempVarId)->second;
                        }
                        sw(reg, addr, Reg::$zero);
                    }
                } else {
                    if (inSelfDefinedFunc) {
                        if (localVarId2offset.find(dst->varId) == localVarId2offset.end()) {
                            localVarId2offset.insert({dst->varId, tempFuncStackOffsetTop});
                            addr = tempFuncStackOffsetTop;
                            tempFuncStackOffsetTop += 4;
                        } else {
                            addr = localVarId2offset.find(dst->varId)->second;
                        }
                        sw(reg, addr, Reg::$sp);
                    } else {
                        if (localVarId2mem.find(dst->varId) == localVarId2mem.end()) {
                            localVarId2mem.insert({dst->varId, tempStackAddressTop});
                            addr = tempStackAddressTop;
                            tempStackAddressTop += 4;
                        } else {
                            addr = localVarId2mem.find(dst->varId)->second;
                        }
                        sw(reg, addr, Reg::$zero);
                    }
                }
                return;
            }
            case ReferenceType::CharArray_Var:
            case ReferenceType::IntArray_Var: {  // 存到一维数组的某个元素里
                QuadItem *offsetItem = dst->array1_var_index;
                lw(Reg::$t9, (QuadItemChar *) offsetItem);  // $t9 = 数组下标
                sll(Reg::$t9, Reg::$t9, 2);  // t9 = t9 * 4 偏移量
                auto array1Item = (QuadItemCharArray *) dstLVal;
                if (array1Item->isGlobal) {
                    if(array1Item->isTemp) {
                        la(Reg::$t8, toStringWithCheck_Array(array1Item->tempArrayId, array1Item->isTemp, array1Item->isGlobal, array1Item->isConst));
                    }else la(Reg::$t8, toStringWithCheck_Array(array1Item->arrayId, array1Item->isTemp, array1Item->isGlobal, array1Item->isConst));
                    //la(Reg::$t8, array1Item->toString());  // t8 = 基地址
                    addu(Reg::$t9, Reg::$t8, Reg::$t9);  // t9 = t8 + t9 = 基地址 + 偏移
                    sw(reg, 0, Reg::$t9);
                } else if (isFuncFParam(array1Item)) {
                    addr = funcFArrayParamId2offset.find(array1Item->arrayId)->second;
                    lw(Reg::$t8, addr, Reg::$sp);  // t8: 数组实际的首地址
                    addu(Reg::$t9, Reg::$t9, Reg::$t8);  // t9:偏移, t8: 首地址
                    sw(reg, 0, Reg::$t9);
                } else {
                    if (inSelfDefinedFunc) {
                        if (array1Item->isTemp) {
                            addr = tempArrayId2offset.find(array1Item->tempArrayId)->second;
                        } else {
                            addr = localArrayId2offset.find(array1Item->arrayId)->second;
                        }
                        addiu(Reg::$t9, Reg::$t9, addr);
                        addu(Reg::$t9, Reg::$t9, Reg::$sp);
//                        lw(Reg::$t8, addr, Reg::$sp);  // t8 = 数组首地址
//                        addu(Reg::$t9, Reg::$t9, Reg::$t8);
                        sw(reg, 0, Reg::$t9);
                    } else {
                        if (array1Item->isTemp) {
                            addr = tempArrayId2mem.find(array1Item->tempArrayId)->second;
                        } else {
                            addr = localArrayId2mem.find(array1Item->arrayId)->second;
                        }
                        addiu(Reg::$t9, Reg::$t9, addr);  // t9:偏移, addr:基地址
                        sw(reg, 0, Reg::$t9);
                    }
                }
                return;
            }
            case ReferenceType::IntArray:
            case ReferenceType::CharArray:
            case ReferenceType::Unset:
                throw std::runtime_error("\nError in MipsTranslator.cpp, line " +
                                         std::to_string(__LINE__) +
                                         ": reaching illegal cases\n");
        }
        return;  // 处理完所有 LVal的情况
    }
    if (dst->isGlobal) {
        if(dst->isTemp) {
            la(Reg::$t9, toStringWithCheck(dst->tempVarId, dst->isTemp,dst->isGlobal,dst->isConst));
        }else la(Reg::$t9, toStringWithCheck(dst->varId, dst->isTemp,dst->isGlobal,dst->isConst));
        //la(Reg::$t9, dst->toString());
        sw(reg, 0, Reg::$t9);
        return;
    }
    if (isFuncFParam(dst)) {
        addr = funcFVarParamId2offset.find(dst->varId)->second;
        sw(reg, addr, Reg::$sp);
        return;
    }
    if (dst->isTemp) {
        if (inSelfDefinedFunc) {
            if (tempVarId2offset.find(dst->tempVarId) == tempVarId2offset.end()) {
                tempVarId2offset.insert({dst->tempVarId, tempFuncStackOffsetTop});
                addr = tempFuncStackOffsetTop;
                tempFuncStackOffsetTop += 4;
            } else {
                addr = tempVarId2offset.find(dst->tempVarId)->second;
            }
            sw(reg, addr, Reg::$sp);
        } else {
            if (tempVarId2mem.find(dst->tempVarId) == tempVarId2mem.end()) {
                printf("dst->tempVarId : %d char tempStackAddressTop = %d\n",dst->tempVarId,tempStackAddressTop);
                tempVarId2mem.insert({dst->tempVarId, tempStackAddressTop});
                addr = tempStackAddressTop;
                tempStackAddressTop += 4;
            } else {
                addr = tempVarId2mem.find(dst->tempVarId)->second;
            }
            sw(reg, addr, Reg::$zero);
        }
    } else {
        if (inSelfDefinedFunc) {
            if (localVarId2offset.find(dst->varId) == localVarId2offset.end()) {
                localVarId2offset.insert({dst->varId, tempFuncStackOffsetTop});
                addr = tempFuncStackOffsetTop;
                tempFuncStackOffsetTop += 4;
            } else {
                addr = localVarId2offset.find(dst->varId)->second;
            }
            sw(reg, addr, Reg::$sp);
        } else {
            if (localVarId2mem.find(dst->varId) == localVarId2mem.end()) {
                localVarId2mem.insert({dst->varId, tempStackAddressTop});
                addr = tempStackAddressTop;
                tempStackAddressTop += 4;
            } else {
                addr = localVarId2mem.find(dst->varId)->second;
            }
            sw(reg, addr, Reg::$zero);
        }
    }
}

//FIXME:一些Int没有改成Char
//将指定的8字节加载到寄存器中
void MipsTranslator::lb(Reg reg, QuadItemChar *var, bool whenPushingParamsRecursively) {
    int addr;
    QuadItem *referenceItem = var;
#ifdef MIPS_DEBUG
    mipsOutput << std::flush;
#endif
    assert(var != nullptr);
    if (referenceItem->reference != nullptr) {  // var 为 LVal
        ReferenceType referenceType = referenceItem->referenceType;
        referenceItem = referenceItem->reference;

        if (referenceItem->type == QuadItemType::Imc) {
            li(reg, ((QuadItemImc *) referenceItem)->value);
            return;
        } else {
            switch (referenceType) {
                case ReferenceType::Int:
                case ReferenceType::Char: {
                    var = (QuadItemChar *) referenceItem;
                    if (var->isGlobal) {
                        la(reg, var->toString());
                        lb(reg, 0, reg);  // 使用 lb 加载字节
                    } else if (isFuncFParam(var)) {
                        addr = funcFVarParamId2offset.find(var->varId)->second;
                        if (whenPushingParamsRecursively) {
                            addr += 30000;
                        }
                        lb(reg, addr, Reg::$sp);  // 从栈中加载字节
                    } else if (var->isConst) {
                        li(reg, var->value);  // 加载常量值
                    } else {
                        if (inSelfDefinedFunc) {
                            if (var->isTemp) {
                                addr = tempVarId2offset.find(var->tempVarId)->second;
                            } else {
                                addr = localVarId2offset.find(var->varId)->second;
                            }
                            if (whenPushingParamsRecursively) {
                                addr += 30000;
                            }
                            lb(reg, addr, Reg::$sp);  // 从栈中加载字节
                        } else {
                            if (var->isTemp) {
                                addr = tempVarId2mem.find(var->tempVarId)->second;
                            } else {
                                addr = localVarId2mem.find(var->varId)->second;
                            }
                            lb(reg, addr, Reg::$zero);  // 从内存中加载字节
                        }
                    }
                    return;
                }
                case ReferenceType::CharArray_Var:
                case ReferenceType::IntArray_Var: {
                    QuadItem *offsetItem = var->array1_var_index;
                    Reg temp;
                    if (reg == Reg::$t7) temp = Reg::$t6;
                    else temp = Reg::$t7;
                    lb(temp, (QuadItemChar *) offsetItem, whenPushingParamsRecursively);  // 加载数组下标
                    sll(temp, Reg::$t7, 2);  // 乘以 4
                    auto array1Item = (QuadItemIntArray *) referenceItem;
                    if (array1Item->isGlobal) {
                        la(reg, array1Item->toString());
                        addu(reg, reg, temp);  // 计算数组偏移地址
                        lb(reg, 0, reg);  // 使用 lb 加载字节
                    } else if (isFuncFParam(array1Item)) {
                        addr = funcFArrayParamId2offset.find(array1Item->arrayId)->second;
                        if (whenPushingParamsRecursively) {
                            addr += 30000;
                        }
                        lb(reg, addr, Reg::$sp);  // 从栈中加载字节
                        addu(reg, reg, temp);
                        lb(reg, 0, reg);  // 使用 lb 加载字节
                    } else {
                        if (inSelfDefinedFunc) {
                            if (array1Item->isTemp) {
                                addr = tempArrayId2offset.find(array1Item->tempArrayId)->second;
                            } else {
                                addr = localArrayId2offset.find(array1Item->arrayId)->second;
                            }
                            if (whenPushingParamsRecursively) {
                                addr += 30000;
                            }
                            addiu(Reg::$t8, Reg::$sp, addr);
                            addu(temp, Reg::$t8, temp);
                            lb(reg, 0, temp);  // 使用 lb 加载字节
                        } else {
                            if (array1Item->isTemp) {
                                addr = tempArrayId2mem.find(array1Item->tempArrayId)->second;
                            } else {
                                addr = localArrayId2mem.find(array1Item->arrayId)->second;
                            }
                            addiu(reg, temp, addr);
                            lb(reg, 0, reg);  // 使用 lb 加载字节
                        }
                    }
                    return;
                }
                case ReferenceType::IntArray:
                case ReferenceType::CharArray: {
                    auto arrayItem = (QuadItemIntArray *) referenceItem;
                    if (arrayItem->isGlobal) {
                        la(reg, arrayItem->toString());
                    } else if (isFuncFParam(arrayItem)) {
                        addr = funcFArrayParamId2offset.find(arrayItem->arrayId)->second;
                        if (whenPushingParamsRecursively) {
                            addr += 30000;
                        }
                        lb(reg, addr, Reg::$sp);  // 从栈中加载字节
                    } else {
                        if (inSelfDefinedFunc) {
                            if (arrayItem->isTemp) {
                                addr = tempArrayId2offset.find(arrayItem->tempArrayId)->second;
                            } else {
                                addr = localArrayId2offset.find(arrayItem->arrayId)->second;
                            }
                            if (whenPushingParamsRecursively) {
                                addr += 30000;
                            }
                            addiu(reg, Reg::$sp, addr);
                        } else {
                            if (arrayItem->isTemp) {
                                addr = tempArrayId2mem.find(arrayItem->tempArrayId)->second;
                            } else {
                                addr = localArrayId2mem.find(arrayItem->arrayId)->second;
                            }
                            li(reg, addr);
                        }
                    }
                    return;
                }
                case ReferenceType::Unset:
                    throw std::runtime_error("\nError in MipsTranslator.cpp, line " +
                                             std::to_string(__LINE__) +
                                             ": reaching illegal cases\n");
            }
        }
        return;  // 处理完所有 LVal的情况
    }

    // 非 LVal
    if (var->isGlobal) {
        la(reg, var->toString());
        lb(reg, 0, reg);  // 使用 lb 加载字节
        return;
    }
    if (var->isConst) {
        li(reg, var->value);
        return;
    }
    if (isFuncFParam(var)) {
        addr = funcFVarParamId2offset.find(var->varId)->second;
        if (whenPushingParamsRecursively) {
            addr += 30000;
        }
        lb(reg, addr, Reg::$sp);  // 从栈中加载字节
        return;
    }
    if (inSelfDefinedFunc) {
        if (var->isTemp) {
            addr = tempVarId2offset.find(var->tempVarId)->second;
        } else {
            addr = localVarId2offset.find(var->varId)->second;
        }
        if (whenPushingParamsRecursively) {
            addr += 30000;
        }
        lb(reg, addr, Reg::$sp);  // 从栈中加载字节
    } else {
        if (var->isTemp) {
            addr = tempVarId2mem.find(var->tempVarId)->second;
        } else {
            addr = localVarId2mem.find(var->varId)->second;
        }
        lb(reg, addr, Reg::$zero);  // 从内存中加载字节
    }
}


void MipsTranslator::lb(Reg dst, int offset, Reg base) {
    mipsOutput << "lb " << reg2s.find(dst)->second << ", " <<
               offset << "(" << reg2s.find(base)->second << ")\n";
}


void MipsTranslator::lw(Reg reg, QuadItemChar *var, bool whenPushingParamsRecursively) {
    printf("lwChar\n");
    int addr;
    QuadItem *referenceItem = var;
#ifdef MIPS_DEBUG
    mipsOutput << std::flush;
#endif
    assert(var != nullptr);
    if (referenceItem->reference != nullptr) {  // var 为 LVal
        ReferenceType referenceType = referenceItem->referenceType;
        referenceItem = referenceItem->reference;
        if (referenceItem->type == QuadItemType::Imm) {
            li(reg, ((QuadItemImm *)referenceItem)->value);
            return;
        } else {
            switch (referenceType) {
                case ReferenceType::Char:
                case ReferenceType::Int: {
                    var = (QuadItemChar *)referenceItem;
                    //处理全局变量
                    if (var->isGlobal) {
                        //isTemp没用
                        if(var->isTemp) {
                            la(reg, toStringWithCheck(var->tempVarId, var->isTemp,var->isGlobal,var->isConst));
                        }else la(reg, toStringWithCheck(var->varId, var->isTemp,var->isGlobal,var->isConst));
                        //la(reg, var->toString());
                        lw(reg, 0, reg);
                    } else if (isFuncFParam(var)) {
                        addr = funcFVarParamId2offset.find(var->varId)->second;
                        if (whenPushingParamsRecursively) {
                            addr += 30000;
                        }
                        lw(reg, addr, Reg::$sp);
                    } else if (var->isConst) {
                        li(reg, var->value);
                    } else {
                        if (inSelfDefinedFunc) {
                            if (var->isTemp) {
                                addr = tempVarId2offset.find(var->tempVarId)->second;
                            } else {
                                addr = localVarId2offset.find(var->varId)->second;
                            }
                            if (whenPushingParamsRecursively) {
                                addr += 30000;
                            }
                            lw(reg, addr, Reg::$sp);
                        } else {
                            if (var->isTemp) {
                                addr = tempVarId2mem.find(var->tempVarId)->second;
                            } else {
                                addr = localVarId2mem.find(var->varId)->second;
                            }
                            lw(reg, addr, Reg::$zero);
                        }
                    }
                    return;
                }
                case ReferenceType::CharArray_Var:
                case ReferenceType::IntArray_Var: {
                    QuadItem *offsetItem = var->array1_var_index;
                    Reg temp = (reg == Reg::$t7) ? Reg::$t6 : Reg::$t7;
                    lw(temp, (QuadItemChar *)offsetItem, whenPushingParamsRecursively);
                    sll(temp, Reg::$t7, 2);  // t7 = t7 * 4
                    auto array1Item = (QuadItemCharArray *)referenceItem;
                    if (array1Item->isGlobal) {
                        if(array1Item->isTemp) {
                            la(reg, toStringWithCheck_Array(array1Item->tempArrayId, array1Item->isTemp, array1Item->isGlobal, array1Item->isConst));
                        }else la(reg, toStringWithCheck_Array(array1Item->arrayId, array1Item->isTemp, array1Item->isGlobal, array1Item->isConst));
                        //la(reg, array1Item->toString());
                        addu(reg, reg, temp);
                        lw(reg, 0, reg);
                    } else if (isFuncFParam(array1Item)) {
                        addr = funcFArrayParamId2offset.find(array1Item->arrayId)->second;
                        if (whenPushingParamsRecursively) {
                            addr += 30000;
                        }
                        lw(reg, addr, Reg::$sp);
                        addu(reg, reg, temp);
                        lw(reg, 0, reg);
                    } else {
                        if (inSelfDefinedFunc) {
                            if (array1Item->isTemp) {
                                addr = tempArrayId2offset.find(array1Item->tempArrayId)->second;
                            } else {
                                addr = localArrayId2offset.find(array1Item->arrayId)->second;
                            }
                            if (whenPushingParamsRecursively) {
                                addr += 30000;
                            }
                            addiu(Reg::$t8, Reg::$sp, addr);
                            addu(temp, Reg::$t8, temp);
                            lw(reg, 0, temp);
                        } else {
                            if (array1Item->isTemp) {
                                addr = tempArrayId2mem.find(array1Item->tempArrayId)->second;
                            } else {
                                addr = localArrayId2mem.find(array1Item->arrayId)->second;
                            }
                            addiu(reg, temp, addr);
                            lw(reg, 0, reg);
                        }
                    }
                    return;
                }
                case ReferenceType::IntArray:
                case ReferenceType::CharArray: {
                    auto arrayItem = (QuadItemCharArray *) referenceItem;
                    //                    mipsOutput << "# 实参类型 Array2\n";
                    if (arrayItem->isGlobal) {
                        if(arrayItem->isTemp) {
                            la(reg, toStringWithCheck_Array(arrayItem->tempArrayId, arrayItem->isTemp, arrayItem->isGlobal, arrayItem->isConst));
                        }else la(reg, toStringWithCheck_Array(arrayItem->arrayId, arrayItem->isTemp, arrayItem->isGlobal, arrayItem->isConst));
                        //la(reg, arrayItem->toString());
                    } else if (isFuncFParam(arrayItem)) {
                        addr = funcFArrayParamId2offset.find(arrayItem->arrayId)->second;
                        if (whenPushingParamsRecursively) {
                            addr += 30000;
                        }
                        lw(reg, addr, Reg::$sp);
                    } else {
                        if (inSelfDefinedFunc) {
                            if (arrayItem->isTemp) {
                                addr = tempArrayId2offset.find(arrayItem->tempArrayId)->second;
                            } else {
                                addr = localArrayId2offset.find(arrayItem->arrayId)->second;
                            }
                            if (whenPushingParamsRecursively) {
                                addr += 30000;
                            }
                            addiu(reg, Reg::$sp, addr);
                        } else {
                            if (arrayItem->isTemp) {
                                addr = tempArrayId2mem.find(arrayItem->tempArrayId)->second;
                            } else {
                                addr = localArrayId2mem.find(arrayItem->arrayId)->second;
                            }
                            li(reg, addr);
                        }
                    }
                    return;
                }
                case ReferenceType::Unset:
                    throw std::runtime_error("\nError in MipsTranslator.cpp, line " +
                                             std::to_string(__LINE__) +
                                             ": reaching illegal cases\n");
                // Other cases can be implemented similarly
                default: {
                    printf("Type : %s",quadItemTypeMap2.find(referenceItem->type)->second.c_str());
                    fflush(stdout);
                    throw std::runtime_error("\nError: reaching illegal cases\n");
                }
            }
        }
        return;
    }
    // 非 LVal
    if (var->isGlobal) {
        if(var->isTemp) {
            la(reg, toStringWithCheck(var->tempVarId,  var->isTemp,var->isGlobal,var->isConst));
        }else la(reg, toStringWithCheck(var->varId,  var->isTemp,var->isGlobal,var->isConst));
        //la(reg, var->toString());
        lw(reg, 0, reg);
        return;
    }
    if (var->isConst) {
        li(reg, var->value);
        return;
    }
    if (isFuncFParam(var)) {
        addr = funcFVarParamId2offset.find(var->varId)->second;
        if (whenPushingParamsRecursively) {
            addr += 30000;
        }
        lw(reg, addr, Reg::$sp);
        return;
    }
    if (inSelfDefinedFunc) {
        if (var->isTemp) {
            addr = tempVarId2offset.find(var->tempVarId)->second;
        } else {
            addr = localVarId2offset.find(var->varId)->second;
        }
        if (whenPushingParamsRecursively) {
            addr += 30000;
        }
        lw(reg, addr, Reg::$sp);
    } else {
        if (var->isTemp) {
            addr = tempVarId2mem.find(var->tempVarId)->second;
            printf("Check VarId : %d addr : %d\n",var->tempVarId,addr);
        } else {
            addr = localVarId2mem.find(var->varId)->second;
        }
        lw(reg, addr, Reg::$zero);
    }
}


void MipsTranslator::StackPrint() {
    printf("STACKPRINT\n");

    for(auto &pair : funcFArrayParamId2offset) {
        printf("FuncVarOffset VarID : %d addt=r : %d\n",pair.first,pair.second);
    }
    for(auto &pair : funcFVarParamId2offset) {
        printf("FuncArrayOffset VarID : %d addt=r : %d\n",pair.first,pair.second);
    }
    for(auto &pair : localArrayId2offset) {
        printf("localArrayOffset VarID : %d addt=r : %d\n",pair.first,pair.second);
    }
    for(auto &pair : localVarId2offset) {
        printf("localOffset VarID : %d addt=r : %d\n",pair.first,pair.second);
    }
    for(auto &pair : tempArrayId2offset) {
        printf("tempArrayOffset VarID : %d addt=r : %d\n",pair.first,pair.second);
    }
    for(auto &pair : tempVarId2offset) {
        printf("tempOffset VarID : %d addt=r : %d\n",pair.first,pair.second);
    }
    for(auto &pair : tempArrayId2mem) {
        printf("localArray VarID : %d addt=r : %d\n",pair.first,pair.second);
    }
    for(auto &pair : localVarId2mem) {
        printf("local VarID : %d addt=r : %d\n",pair.first,pair.second);
    }
    for(auto &pair : localArrayId2mem) {
        printf("localArray VarID : %d addt=r : %d\n",pair.first,pair.second);
    }
    for(auto &pair : tempVarId2mem) {
        printf("temp VarID : %d addt=r : %d\n",pair.first,pair.second);
    }
}


void MipsTranslator::TagPrint() {
    for(std::string name : tagTable)printf("%s\n",name.c_str());
}

std::string MipsTranslator::toStringWithCheck2(int VarId) {
    return tagTable[VarId-1];
}

std::string MipsTranslator::toStringWithCheck(int varId, bool isTemp, bool isGlobal, bool isConst) {
    auto* string1 = new std::string();
    auto *string2 = new std::string();
    if (isTemp) {
        *string1 += ("temp_char" + std::to_string(abs(varId)));
        *string2 += ("temp_int" + std::to_string(abs(varId)));
    } else {
        if (isGlobal) {
            if (isConst) {
                *string1+= "global_const_char" + std::to_string(abs(varId));
                *string2+= "global_const_int" + std::to_string(abs(varId));
            } else {
                *string1+= "global_char" + std::to_string(abs(varId));
                *string2+= "global_int" + std::to_string(abs(varId));
            }
        } else {
            if (isConst) {
                *string1+= "local_const_char" + std::to_string(abs(varId));
                *string2+= "local_const_int" + std::to_string(abs(varId));
            } else {
                *string1+= "local_char" + std::to_string(abs(varId));
                *string2+= "local_int" + std::to_string(abs(varId));
            }
        }
    }
    bool flag1= contains(tagTable,string1);
    bool flag2 = contains(tagTable,string2);
    if(flag1&&flag2)
    {printf("Error:Conflict\n");return "Error";}
    if(!(flag1||flag2))
    {printf("Error:Not Found %s %s \n",string1->c_str(),string2->c_str());return "Error2";}
    else if(flag1)return *string1;
    else return *string2;


}
std::string MipsTranslator::toStringWithCheck_Array(int arrayId,bool isTemp,bool isGlobal,bool isConst) {
    auto* string1 = new std::string();
    auto *string2 = new std::string();

    if (isTemp) {
        *string1+= "temp_Iarr" + std::to_string(abs(arrayId));
        *string2+= "temp_Carr" + std::to_string(abs(arrayId));
    } else {
        if (isGlobal) {
            if (isConst) {
                *string1+= "global_const_Iarr" + std::to_string(abs(arrayId));
                *string2+= "global_const_Carr" + std::to_string(abs(arrayId));
            } else {
                *string1+= "global_Iarr" + std::to_string(abs(arrayId));
                *string2+= "global_Carr" + std::to_string(abs(arrayId));
            }
        } else {
            if (isConst) {
                *string1+= "local_const_Iarr" + std::to_string(abs(arrayId));
                *string2+= "local_const_Carr" + std::to_string(abs(arrayId));
            } else {
                *string1+= "local_Iarr" + std::to_string(abs(arrayId));
                *string2+= "local_Carr" + std::to_string(abs(arrayId));
            }
        }
    }
    bool flag1= contains(tagTable,string1);
    bool flag2 = contains(tagTable,string2);
    if(flag1&&flag2)
    {printf("Error:Conflict\n");return "Error";}
    if(!(flag1||flag2))
    {printf("Error:Not Found %s %s \n",string1->c_str(),string2->c_str());return "Error2";}
    else if(flag1)return *string1;
    else return *string2;
}


bool MipsTranslator::contains(const std::vector<std::string>& vec, const std::string* query) {
    if (query == nullptr) {
        return false; // 避免空指针访问
    }
    return std::find(vec.begin(), vec.end(), *query) != vec.end();
}