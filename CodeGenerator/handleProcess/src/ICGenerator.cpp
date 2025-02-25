//
// Created by 郑慕函 on 24-11-12.
//
#include "../include/ICGenerator.h"

#include <cassert>

extern std::ofstream icOutput;
std::map<QuadEntryType, std::string> quadEntryTypeMap = {
    // 第一次代码生成作业:
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
    {QuadEntryType::ArrayGet, "ArrayGet"},

    {QuadEntryType::Bnez, "Bnez"},
    {QuadEntryType::Beqz, "Beqz"},

    {QuadEntryType::JumpLabel, "JumpLabel"},
    {QuadEntryType::InsertLabel, "InsertLabel"},

    // 第二次代码生成作业:
    {QuadEntryType::Or, "Or"},
    {QuadEntryType::And, "And"},
    {QuadEntryType::Equal, "Equal"},
    {QuadEntryType::NotEqual, "NotEqual"},
    {QuadEntryType::LessEqual, "LessEqual"},
    {QuadEntryType::LessThan, "LessThan"},
    {QuadEntryType::GreaterThan, "GreaterThan"},
    {QuadEntryType::GreaterEqual, "GreaterEqual"}
};

inline std::map<QuadItemType, std::string> QuadItemType2Name() {
    std::map<QuadItemType, std::string> typeMap;
    typeMap[QuadItemType::String] = "String";
    typeMap[QuadItemType::Imm] = "Imm";
    typeMap[QuadItemType::Imc] = "Imc";
    typeMap[QuadItemType::Label] = "Label";
    typeMap[QuadItemType::IntArray] = "IntArray";
    typeMap[QuadItemType::CharArray] = "CharArray";
    typeMap[QuadItemType::Int] = "Int";
    typeMap[QuadItemType::Char] = "Char";
    typeMap[QuadItemType::Func] = "Func";
    typeMap[QuadItemType::Reference] = "Reference";
    return typeMap;
}
std::map<QuadItemType, std::string> quadItemTypeMap3 = {
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

ICGenerator *ICGenerator::self = nullptr;

ICGenerator *ICGenerator::getInstance() {
    if (self == nullptr) {
        self = new ICGenerator();
    }
    return self;
}

ICGenerator::ICGenerator() {
    mainEntries = new std::vector<QuadEntry *>;
    id2allPureString = new std::map<int, std::string *>;
    name2quadItemFunc = new std::map<std::string *, QuadItemFunc *>;
    quadItemFuncOrder = new list<string *>;
    currentFunc = nullptr;
}

ICGenerator::~ICGenerator() {
    for (const auto *pItem: *mainEntries) delete pItem;
    delete mainEntries;
    for (const auto &i: *id2allPureString) delete i.second;
    delete id2allPureString;
    delete currentFunc;
    delete quadItemFuncOrder;
}

//处理常量声明
//初始化QuadItem，QuadEntry
//QuadEntry是QuadItem在逻辑链中的进一步封装
void ICGenerator::translate_ConstVarDef(bool isGlobal, SymbolTableEntry *tableEntry,
                                         SymbolTable *currentTable) const {
    printf("ICGenerate :: translate_ConstVarDef %s\n",SymbolTableEntryType2String.find(tableEntry->type)->second.c_str());
    //int 类型
    if(tableEntry->type == SymbolTableEntryType::Int || tableEntry->type==SymbolTableEntryType::IntConst) {
        //QuadItem
        auto *quadItemInt = new QuadItemInt(tableEntry->getName(), tableEntry,
                                        true, isGlobal, tableEntry->intGet());
        currentTable->addQuadItem(*quadItemInt->originalName, quadItemInt);

        //QuadItem->QuadEntry
        auto *icEntry = new QuadEntry(QuadEntryType::ConstIntDefine, quadItemInt);
        //QuadEntry->QuadEntries
        if (currentFunc == nullptr) {
            mainEntries->push_back(icEntry);
        } else {
            currentFunc->entries->push_back(icEntry);
        }
    }
    //Char类型
    else if(tableEntry->type == SymbolTableEntryType::Char||tableEntry->type==SymbolTableEntryType::CharConst) {
        printf("translate_ConstVarDef:Char\n");
        auto *quadItemChar = new QuadItemChar(tableEntry->getName(), tableEntry,
                                        true, isGlobal, tableEntry->charGet());
        currentTable->addQuadItem(*quadItemChar->originalName, quadItemChar);
        auto *icEntry = new QuadEntry(QuadEntryType::ConstCharDefine, quadItemChar);
        if (currentFunc == nullptr) {
            printf("mainEntries\n"); mainEntries->push_back(icEntry);
        } else {
            currentFunc->entries->push_back(icEntry);
        }
    }
    else {
     printf("Error in translate_ConstVarDef : EntryType not found\n");
    }
}


//常量数组定义
//将数组信息封装到 QuadItem 和 QuadEntry 中并插入到中间代码列表
void ICGenerator::translate_ConstArrayDef(bool isGlobal, SymbolTableEntry *tableEntry, int d,
                                            SymbolTable *currentTable) const {
    printf("ICGenerate :: translate_ConstArrayDef d = %d\n",d);
    if(tableEntry->type== SymbolTableEntryType::ConstIntArray) {
        printf("translate_ConstArrayDef:IntArray\n");
        //QuadItem
        //创建四元式表，添加到符号表
        auto *quadItemArray = new QuadItemIntArray(tableEntry->getName(), tableEntry, isGlobal, true, d,
                                            tableEntry->getAllIntArrayConst());
        quadItemArray->setOriginType(1, d, -1);
        currentTable->addQuadItem(*quadItemArray->originalName, quadItemArray);

        //QuadItem -> QuadEntry
        auto *icEntry = new QuadEntry(QuadEntryType::ConstIntArrayDefine, quadItemArray);

        if (currentFunc == nullptr) {
             mainEntries->push_back(icEntry);
        } else {
            currentFunc->entries->push_back(icEntry);
        }
    }
    else if(tableEntry->type== SymbolTableEntryType::ConstCharArray) {
        auto *quadItemArray = new QuadItemCharArray(tableEntry->getName(), tableEntry, isGlobal, true, d,
                                            tableEntry->getAllCharArrayConst());
        quadItemArray->setOriginType(1, d, -1);
        currentTable->addQuadItem(*quadItemArray->originalName, quadItemArray);

        auto *icEntry = new QuadEntry(QuadEntryType::ConstCharArrayDefine, quadItemArray);
        //printf("222d = %d,length1 = %d,length2 = %d,length = %d\n",icEntry->operator1->originType.d,quadItemArray->originType.length1,quadItemArray->originType.length2,quadItemArray->length);
        if (currentFunc == nullptr) {
             mainEntries->push_back(icEntry);
        } else {
            //QuadItemCharArray operator1 = (QuadItemCharArray*)(icEntry->operator1);
            currentFunc->entries->push_back(icEntry);
        }
    }
    else {
        printf("Error in translate_ConstArrayDef : type not found %s\n",SymbolTableEntryType2String.find(tableEntry->type)->second.c_str());
    }
}



//处理变量声明
void ICGenerator::translate_VarDef(QuadItem *initItem, bool isGlobal,
                                    SymbolTableEntry *tableEntry, bool hasInitVal,
                                    SymbolTable *currentTable) const {
    printf("ICGenerate :: translate_VarDef type of Var is %s\n",quadItemTypeMap3.find(initItem->type)->second.c_str());
    QuadEntry *icEntry;
    if (isGlobal) {  // 全局变量，初值确定，一个操作数
        //int类型
        if(initItem->type == QuadItemType::Int || initItem->type==QuadItemType::Imm) {

            auto *quadItemImm = dynamic_cast<QuadItemImm *>(initItem);
            auto *quadItemVar = new QuadItemInt(tableEntry->getName(), tableEntry,
                                            false, true, quadItemImm->value);
            currentTable->addQuadItem(*quadItemVar->originalName, quadItemVar);

            icEntry = new QuadEntry(QuadEntryType::IntDefine, quadItemVar);
        }
        else if(initItem->type == QuadItemType::Char|| initItem->type==QuadItemType::Imc) {
            auto *quadItemImc = dynamic_cast<QuadItemImc *>(initItem);
            auto *quadItemVar = new QuadItemInt(tableEntry->getName(), tableEntry,
                                            false, true, quadItemImc->value);
            currentTable->addQuadItem(*quadItemVar->originalName, quadItemVar);

            icEntry = new QuadEntry(QuadEntryType::CharDefine, quadItemVar);
        }
        else {
            std::map<QuadItemType,string> typeMap = QuadItemType2Name();
            printf("Error in translate_VarDef: iniItem type wrong(Global Var) type is %s\n",typeMap.find(initItem->type)->second.c_str());
        }
    } else {  // 非全局变量，若hasInitVal == false, 则无初值!
        if(initItem->type == QuadItemType::Int) {
            auto *rightValue = (QuadItemInt *) initItem;
            auto *quadItemVar = new QuadItemInt(tableEntry->getName(), tableEntry,
                                            false, false);
            currentTable->addQuadItem(*quadItemVar->originalName, quadItemVar);

            //注入Entry中
            if (hasInitVal) {
                icEntry = new QuadEntry(QuadEntryType::IntDefine, quadItemVar, rightValue);
            } else {
                icEntry = new QuadEntry(QuadEntryType::IntDefine, quadItemVar);
            }
        }else if(initItem->type == QuadItemType::Char) {
            auto *rightValue = (QuadItemChar *) initItem;
            auto *quadItemVar = new QuadItemInt(tableEntry->getName(), tableEntry,
                                            false, false);
            currentTable->addQuadItem(*quadItemVar->originalName, quadItemVar);

            if (hasInitVal) {
                icEntry = new QuadEntry(QuadEntryType::CharDefine, quadItemVar, rightValue);
            } else {
                icEntry = new QuadEntry(QuadEntryType::CharDefine, quadItemVar);
            }
        }
        else {printf("Error in translate_VarDef: iniItem type wrong(NonGlobal Var)\n");}
    }
    //判断是否在函数上下文
    if (currentFunc == nullptr) {
        printf("mainEntries\n"); mainEntries->push_back(icEntry);
    } else {
        currentFunc->entries->push_back(icEntry);
    }
}


//数组定义
void ICGenerator::translate_ArrayDef(QuadItem *initItem, bool isGlobal,
                                      SymbolTableEntry *tableEntry, bool hasInitVal, int length,
                                      SymbolTable *currentTable) const {
    printf("ICGenerate :: translate_ArrayDef length : %d\n",length);
    QuadEntry *icEntry;
    auto *initIntArray = ((QuadItemIntArray *)initItem);
    auto *initCharArray = ((QuadItemCharArray *)initItem);
    //全局数组
    if (isGlobal) {
        if(initItem->type == QuadItemType::IntArray){
            auto *quadItemArray = new QuadItemIntArray(tableEntry->getName(), tableEntry, true,
                                                false, length, initIntArray->value);
            quadItemArray->setOriginType(1, length, -1);
            currentTable->addQuadItem(*quadItemArray->originalName, quadItemArray);

            icEntry = new QuadEntry(QuadEntryType::IntArrayDefine, quadItemArray);
        }else if(initItem ->type == QuadItemType::CharArray) {
            auto *quadItemArray = new QuadItemCharArray(tableEntry->getName(), tableEntry, true,
                                               false, length, initCharArray->value);
            quadItemArray->setOriginType(1, length, -1);
            currentTable->addQuadItem(*quadItemArray->originalName, quadItemArray);

            icEntry = new QuadEntry(QuadEntryType::CharArrayDefine, quadItemArray);
        }
        else {
            printf("Error in translate_ArrayDef : iniItem type wrong(Global Var)\n");
        }
    //非全局数组
    } else {
        if(initItem->type == QuadItemType::IntArray){
            auto *quadItemArray = new QuadItemIntArray(tableEntry->getName(), tableEntry, false,
                                            false, length);
            quadItemArray->setOriginType(1, length, -1);
            currentTable->addQuadItem(*quadItemArray->originalName, quadItemArray);

            if (hasInitVal) {
                icEntry = new QuadEntry(QuadEntryType::IntArrayDefine, quadItemArray, initIntArray);
            } else {
                icEntry = new QuadEntry(QuadEntryType::IntArrayDefine, quadItemArray);
            }
        }else if(initItem->type == QuadItemType::CharArray) {
            auto *quadItemArray = new QuadItemCharArray(tableEntry->getName(), tableEntry, false,
                                            false, length);
            quadItemArray->setOriginType(1, length, -1);
            currentTable->addQuadItem(*quadItemArray->originalName, quadItemArray);
            if (hasInitVal) {
                icEntry = new QuadEntry(QuadEntryType::CharArrayDefine, quadItemArray, initCharArray);
            } else {
                icEntry = new QuadEntry(QuadEntryType::CharArrayDefine, quadItemArray);
            }
        }else {
            printf("Error in translate_ArrayDef : iniItem type wrong(NonGlobal Var)\n");
        }
    }
    if (currentFunc == nullptr) {
        printf("mainEntries\n"); mainEntries->push_back(icEntry);
    } else {
        currentFunc->entries->push_back(icEntry);
    }
}

/**
 * 翻译双目运算
 * @param icEntryType
 * @param dst 运算结果的目标存储位置
 * @param src1
 * @param src2
 */
//对quadItem的计算

/**
 * 翻译双目运算
 * @param icEntryType
 * @param dst 运算结果的目标存储位置
 * @param src1
 * @param src2
 */
//对quadItem的计算
void ICGenerator::translate_BinaryOperator(QuadEntryType icEntryType,
                                            QuadItem *dst, QuadItem *src1, QuadItem *src2) const {
    printf("ICGenerate :: translate_BinaryOperator type :%s \n",quadEntryTypeMap.find(icEntryType)->second.c_str());
    if (currentFunc == nullptr) {
        if (icEntryType == QuadEntryType::Assign) {
            mainEntries->push_back(new QuadEntry(icEntryType, dst, src1));
        }
        /**
        else if(icEntryType == QuadEntryType::Mul) {
            auto *dstin = (QuadItemInt *) dst, *r1 = (QuadItemInt *) src1, *r2 = (QuadItemInt *) src2;
            if(r2->isConst) {
                auto multier = new QuadItemInt(false);
                mainEntries->push_back(new QuadEntry(QuadEntryType::Add, dst, src1, src2));
                mainEntries->push_back(new QuadEntry(QuadEntryType::Add, multier, src1, src2));
                for(int i = 0;i < r2->value-1;i++) {
                    mainEntries->push_back(new QuadEntry(QuadEntryType::Mul, dst, dst, multier));
                }
            }
            else {
                // i = 0
                auto counter = new QuadItemInt(false);
                auto initier = new QuadItemInt(false);
                auto adder = new QuadItemInt(false);
                adder->value = 1;
                adder->isConst = true;
                auto analyzer  = new QuadItemInt(false);
                initier->isConst = true;
                initier->value = 0;
                mainEntries->push_back(new QuadEntry(QuadEntryType::IntDefine, counter, initier));


                mainEntries->push_back(new QuadEntry(QuadEntryType::Assign, dst, adder));
                auto *loopBeginLabel = new QuadItemLabel();
                auto *LoopEndLabel = new QuadItemLabel();
                mainEntries ->push_back(new QuadEntry(QuadEntryType::InsertLabel, loopBeginLabel));

                mainEntries -> push_back(new QuadEntry(QuadEntryType::LessThan, analyzer,counter,src2));
                mainEntries -> push_back(new QuadEntry(QuadEntryType::Beqz, analyzer,LoopEndLabel,src2));

                //循环体
                mainEntries->push_back(new QuadEntry(QuadEntryType::Mul, dst, dst,r1));

                mainEntries->push_back(new QuadEntry(QuadEntryType::Add, counter, counter,adder));
                mainEntries->push_back(new QuadEntry(QuadEntryType::JumpLabel, loopBeginLabel));

                mainEntries->push_back(new QuadEntry(QuadEntryType::InsertLabel, LoopEndLabel));
            }
        }
        **/
        else {
            auto *dstin = (QuadItemInt *) dst, *r1 = (QuadItemInt *) src1, *r2 = (QuadItemInt *) src2;
            // if(r1->isConst && r2->isConst) {
            //     dstin->isConst = true;
            //     if(icEntryType == QuadEntryType::Add)dstin->value = r1->value + r2->value;
            //     if(icEntryType == QuadEntryType::Sub)dstin->value = r1->value - r2->value;
            //     if(icEntryType == QuadEntryType::Mul)dstin->value = r1->value * r2->value;
            //     if(icEntryType == QuadEntryType::Div)dstin->value = r1->value / r2->value;
            //     if(icEntryType == QuadEntryType::Mod)dstin->value = r1->value % r2->value;
            // }
            //else
                mainEntries->push_back(new QuadEntry(icEntryType, dst, src1, src2));
        }
    } else {
        if (icEntryType == QuadEntryType::Assign) {
            currentFunc->entries->push_back(new QuadEntry(icEntryType, dst, src1));
        } else {
            auto *dstin = (QuadItemInt *) dst, *r1 = (QuadItemInt *) src1, *r2 = (QuadItemInt *) src2;
            // if(r1->isConst && r2->isConst) {
            //     dstin->isConst = true;
            //     if(icEntryType == QuadEntryType::Add)dstin->value = r1->value + r2->value;
            //     if(icEntryType == QuadEntryType::Sub)dstin->value = r1->value - r2->value;
            //     if(icEntryType == QuadEntryType::Mul)dstin->value = r1->value * r2->value;
            //     if(icEntryType == QuadEntryType::Div)dstin->value = r1->value / r2->value;
            //     if(icEntryType == QuadEntryType::Mod)dstin->value = r1->value % r2->value;
            // } else
                currentFunc->entries->push_back(new QuadEntry(icEntryType, dst, src1, src2));
        }
    }
}

//查找给定符号对应的中间代码类型
QuadEntryType ICGenerator::symbol2binaryOp(Symbol symbol) const {
    return _symbol2binaryOp.find(symbol)->second;
}

QuadEntryType ICGenerator::symbolName2binaryOp(string symbol) const{
    return _symbolName2binaryOp.find(symbol) ->second;
}
/**
 * 翻译单目运算符
 * @param icEntryType
 * @param dst
 * @param src
 */
void ICGenerator::translate_UnaryOperator(QuadEntryType icEntryType, QuadItem *dst, QuadItem *src) const {
    printf("ICGenerate :: translate_UnaryOperator\n");
    if (currentFunc == nullptr) {
        printf("mainEntries\n"); mainEntries->push_back(new QuadEntry(icEntryType, dst, src));
    } else {
        currentFunc->entries->push_back(new QuadEntry(icEntryType, dst, src));
    }
}

QuadEntryType ICGenerator::symbol2unaryOp(string name) const {
    return _symbol2unaryOp.find(name)->second;
}

void ICGenerator::translate_getint(QuadItem *dst) const {
    printf("ICGenerate :: translate_getint\n");
    if (currentFunc == nullptr) {
        printf("mainEntries\n"); mainEntries->push_back(new QuadEntry(QuadEntryType::Getint, dst));
    } else {
        currentFunc->entries->push_back(new QuadEntry(QuadEntryType::Getint, dst));
    }
}

void ICGenerator::translate_getchar(QuadItem *dst) const {
    printf("ICGenerate :: translate_getchar\n");
    if (currentFunc == nullptr) {
        printf("mainEntries\n"); mainEntries->push_back(new QuadEntry(QuadEntryType::Getchar, dst));
    } else {
        currentFunc->entries->push_back(new QuadEntry(QuadEntryType::Getchar, dst));
    }
}

//int Items : Exp返回的QuadItem参数表
void ICGenerator::translate_printf(std::vector<int> *indexOfPercentSign,
                                    std::vector<QuadItem *> *intItems,
                                    std::string *s) const {
    printf("ICGenerate :: translate_printf\n ");
    std::map<QuadItemType,string>typeMap = QuadItemType2Name();
    if(intItems!=nullptr){printf("size of intItems: %lu\n",intItems->size());}
    auto *quadItemString = new QuadItemString();
    //将Exp的解析值按照posOfPer加入quadItemString的intItem和charItem,同时加入
    int start = 1, len, intPos = 0,charPos=0;
    for (const int cur: *indexOfPercentSign) {
        len = cur - start;

        if (len > 0) {
            //加入字符串
            auto *substring = new std::string(s->substr(start, len));
            quadItemString->addStringItem(substring);
        }
        start = cur + 2;
        if(s->at(cur+1)=='d') {
            quadItemString->addIntItem(intItems->at(intPos++));
        }else if(s->at(cur+1)=='c') {
            quadItemString->addCharItem(intItems->at(intPos++));
        }else printf("Error : 非法占位符");
    }

    // 没有%d; 最后一个%d后的字符串
    if (indexOfPercentSign->empty()) {
        auto *substring = new std::string(s->substr(1, s->size() - 2));
        quadItemString->addStringItem(substring);
    } else if (indexOfPercentSign->back() + 2 < s->size() - 1) {
        len = s->size() - 1 - (indexOfPercentSign->back() + 2);
        auto *substring = new std::string(s->substr(indexOfPercentSign->back() + 2, len));
        quadItemString->addStringItem(substring);
    }
    // 加入全局记录字符串
    for (const auto &item: *(quadItemString->id2pureString)) {
        id2allPureString->insert(item);
    }
    // 记录printf操作
    if (currentFunc == nullptr) {
         mainEntries->push_back(new QuadEntry(QuadEntryType::Print, quadItemString));
    } else {
        currentFunc->entries->push_back(new QuadEntry(QuadEntryType::Print, quadItemString));
    }
}


QuadItemFunc *ICGenerator::translate_FuncDef(SymbolTableEntry *funcEntry,
                                            SymbolTable *currentTable) const {
    printf("translate_FuncDef \n");
    auto *func = new QuadItemFunc(funcEntry);
    name2quadItemFunc->insert({funcEntry->getName(), func});
    quadItemFuncOrder->push_back(funcEntry->getName());
    for (QuadItem *p : *(func->params)) {
        // TODO: 假设都是变量
         if (p->type == QuadItemType::Int || p->type == QuadItemType::Char) {
            auto *var = (QuadItemInt *) p;
            currentTable->addQuadItem(*var->originalName, p);
        } else {
            auto *array = (QuadItemIntArray *) p;
            currentTable->addQuadItem(*array->originalName, p);
        }
    }

    return func;
}



void ICGenerator::translate_FuncCall(std::string *funcName, std::vector<QuadItem *> *params) const {
    printf("ICGenerate :: translate_FuncCall\n");
    QuadItemFunc *calledFunc = name2quadItemFunc->find(funcName)->second;
    if (currentFunc != nullptr) {
        currentFunc->entries->push_back(new QuadEntry(calledFunc, params));
    } else {
        mainEntries->push_back(new QuadEntry(calledFunc, params));
    }

}

void ICGenerator::translate_return(QuadItem *quadItem) const {
    printf("ICGenerate :: translate_return(QuadItem)\n");
    if (currentFunc != nullptr) {
        currentFunc->entries->push_back(new QuadEntry(QuadEntryType::FuncReturnWithValue, quadItem));
    } else {mainEntries->push_back(new QuadEntry(QuadEntryType::MainFuncEnd));
    }
}

void ICGenerator::translate_return() const {
    printf("ICGenerate :: translate_return\n");
    if (currentFunc != nullptr) {
        currentFunc->entries->push_back(new QuadEntry(QuadEntryType::FuncReturn));
    } else {
         mainEntries->push_back(new QuadEntry(QuadEntryType::MainFuncEnd));
    }
}


void ICGenerator::translate_MainFunc() const {
    printf("ICGenerate :: translate_MainFunc\n");
    assert(currentFunc == nullptr);
     mainEntries->push_back(new QuadEntry(QuadEntryType::MainFuncStart));
}

void ICGenerator::entryOutput(FILE * fp) {
    std::map<QuadItemType,string> typeMap = QuadItemType2Name();
    //主函数
    for(QuadEntry * quadEntry: *mainEntries) {

        auto it = quadEntryTypeMap.find(quadEntry->entryType);
        if (it != quadEntryTypeMap.end()) {
            fprintf(fp, "%s ", it->second.c_str());  // 找到了，输出
        } else {
            fprintf(fp, "Unknown entry type ");  // 未找到，输出错误信息
        }
        if(quadEntry->operator1 != nullptr)fprintf(fp,"operator1: %s %s ",typeMap.find(quadEntry->operator1->type)->second.c_str(),quadEntry->operator1->Name.c_str());
        if(quadEntry->operator2 != nullptr)fprintf(fp,"operator2: %s %s ",typeMap.find(quadEntry->operator2->type)->second.c_str(),quadEntry->operator2->Name.c_str());
        if(quadEntry->operator3 != nullptr)fprintf(fp,"operator3: %s %s ",typeMap.find(quadEntry->operator3->type)->second.c_str(),quadEntry->operator3->Name.c_str());
        if(quadEntry->params!=nullptr) {
            fprintf(fp,"params (size %d):",quadEntry->params->size());
            for(const QuadItem* param : *quadEntry->params)fprintf(fp,"%s ",quadItemTypeMap3.find(param->type)->second.c_str());fprintf(fp,"\n");
        }
        //printf("opNum: %d ",quadEntry->opNum);
        fprintf(fp,"\n");
    }
    //声明函数
    for(auto &pair : *name2quadItemFunc) {
        string * funcName = pair.first;
        QuadItemFunc * quad_item_func = pair.second;
        fprintf(fp,"Func : %s\n",funcName->c_str());
        if(quad_item_func->params!=nullptr) {
            fprintf(fp,"Def : ");
            for(const QuadItem*quad_item : *quad_item_func->params) {
                fprintf(fp,"%s ",quadItemTypeMap3.find(quad_item->type)->second.c_str());
            }fprintf(fp,"\n");
        }
        for(QuadEntry *quadEntry : *quad_item_func->entries) {
            auto it = quadEntryTypeMap.find(quadEntry->entryType);
            if (it != quadEntryTypeMap.end()) {
                fprintf(fp, "%s ", it->second.c_str());  // 找到了，输出
            } else {
                fprintf(fp, "Unknown entry type ");  // 未找到，输出错误信息
            }
            if(quadEntry->operator1 != nullptr)fprintf(fp,"operator1: %s %s %d  ",typeMap.find(quadEntry->operator1->type)->second.c_str(),quadEntry->operator1->Name.c_str());
            if(quadEntry->operator2 != nullptr)fprintf(fp,"operator2: %s %s %d  ",typeMap.find(quadEntry->operator2->type)->second.c_str(),quadEntry->operator2->Name.c_str());
            if(quadEntry->operator3 != nullptr)fprintf(fp,"operator3: %s %s %d  ",typeMap.find(quadEntry->operator3->type)->second.c_str(),quadEntry->operator3->Name.c_str());
            if(quadEntry->params!=nullptr) {
                fprintf(fp,"params (size %d):",quadEntry->params->size());
                for(const QuadItem* param : *quadEntry->params)fprintf(fp,"%s ",quadItemTypeMap3.find(param->type)->second.c_str());fprintf(fp,"\n");
            }
            //printf("opNum: %d ",quadEntry->opNum);
            fprintf(fp,"\n");
        }
    }
}


void ICGenerator::translate_Bnez(QuadItem *condition, QuadItemLabel *label) const {
    if (currentFunc == nullptr) {
        mainEntries->push_back(new QuadEntry(QuadEntryType::Bnez, condition, label));
    } else {
        currentFunc->entries->push_back(new QuadEntry(QuadEntryType::Bnez, condition, label));
    }
}

void ICGenerator::translate_Beqz(QuadItem *condition, QuadItemLabel *label) const {
    if (currentFunc == nullptr) {
        mainEntries->push_back(new QuadEntry(QuadEntryType::Beqz, condition, label));
    } else {
        currentFunc->entries->push_back(new QuadEntry(QuadEntryType::Beqz, condition, label));
    }
}

void ICGenerator::translate_JumpLabel(QuadItemLabel *label) const {
    if (currentFunc == nullptr) {
        mainEntries->push_back(new QuadEntry(QuadEntryType::JumpLabel, label));
    } else {
        currentFunc->entries->push_back(new QuadEntry(QuadEntryType::JumpLabel, label));
    }
}

void ICGenerator::translate_InsertLabel(QuadItemLabel *label) const {
    if (currentFunc == nullptr) {
        mainEntries->push_back(new QuadEntry(QuadEntryType::InsertLabel, label));
    } else {
        currentFunc->entries->push_back(new QuadEntry(QuadEntryType::InsertLabel, label));
    }
}




//处理二维数组
/**
void ICGenerator::translate_ConstArray2Def(bool isGlobal, SymbolTableEntry *tableEntry,
                                            int d1, int d2, SymbolTable *currentTable) const {
    int **oldArray = tableEntry->array2ConstGetAll();
    int *newArray = new int[d1 * d2];

    int k = 0;
    for (int i = 0; i < d1; ++i) {
        for (int j = 0; j < d2; ++j) {
            newArray[k++] = oldArray[i][j];
        }
    }

    auto *quadItemArray = new QuadItemIntArray(tableEntry->getName(), tableEntry, isGlobal,
                                        true, d1 * d2, newArray);
    currentTable->addQuadItem(*quadItemArray->originalName, quadItemArray);

    auto *icEntry = new QuadEntry(QuadEntryType::ConstArrayDefine, quadItemArray);

    if (currentFunc == nullptr) {
        printf("mainEntries\n");->push_back(icEntry);
    } else {
        currentFunc->entries->push_back(icEntry);
    }
}**/