//
// Created by 郑慕函 on 24-11-11.
//

#ifndef QuadGENERATOR_H
#define QuadGENERATOR_H


#include "../../item/lexer/include/Symbol.h"
#include "../../item/tree/include/Node.h"
#include "../../item/symbolTable/include/SymbolTable.h"

#include "../../item/quadrupleGenerator/include/QuadItemImc.h"
#include "../../item/quadrupleGenerator/include/QuadItemType.h"
#include "../../item/quadrupleGenerator/include/QuadItem.h"
#include "../../item/quadrupleGenerator/include/QuadItemIntArray.h"
#include "../../item/quadrupleGenerator/include/QuadItemCharArray.h"
#include "../../item/quadrupleGenerator/include/QuadItemImm.h"
#include "../../item/quadrupleGenerator/include/QuadItemLabel.h"
#include "../../item/quadrupleGenerator/include/QuadItemString.h"
#include "../../item/quadrupleGenerator/include/QuadItemInt.h"
#include "../../item/quadrupleGenerator/include/QuadItemChar.h"
#include "../../item/quadrupleGenerator/include/QuadEntryType.h"
#include "../../item/quadrupleGenerator/include/QuadEntry.h"
#include "../../item/quadrupleGenerator/include/QuadItemFunc.h"


/**
 * Intermediate Code Translator 中间代码翻译器
 */
class ICGenerator {
    static ICGenerator *self;  // 单例模式

    ICGenerator();

    ~ICGenerator();

    const std::map<Symbol, QuadEntryType> _symbol2binaryOp = {
            {Symbol::PLUS,   QuadEntryType::Add},
            {Symbol::ASSIGN, QuadEntryType::Assign},
            {Symbol::MINU,   QuadEntryType::Sub},
            {Symbol::MULT,   QuadEntryType::Mul},
            {Symbol::DIV,    QuadEntryType::Div},
            {Symbol::MOD,    QuadEntryType::Mod},

        //第二次代码生成
        // 代码生成作业2
            {Symbol::OR,     QuadEntryType::Or},
            {Symbol::AND,    QuadEntryType::And},
            {Symbol::EQL,    QuadEntryType::Equal},
            {Symbol::NEQ,    QuadEntryType::NotEqual},
            {Symbol::LEQ,    QuadEntryType::LessEqual},
            {Symbol::LSS,    QuadEntryType::LessThan},
            {Symbol::GRE,    QuadEntryType::GreaterThan},
            {Symbol::GEQ,    QuadEntryType::GreaterEqual}


    };

    const std::map<string, QuadEntryType> _symbolName2binaryOp = {
        {"PLUS",   QuadEntryType::Add},
        {"ASSIGN", QuadEntryType::Assign},
        {"MINU",   QuadEntryType::Sub},
        {"MULT",   QuadEntryType::Mul},
        {"DIV",    QuadEntryType::Div},
        {"MOD",    QuadEntryType::Mod},
        //第二次代码生成
        // 代码生成作业2
        {"OR",     QuadEntryType::Or},
        {"AND",    QuadEntryType::And},
        {"EQL",    QuadEntryType::Equal},
        {"NEQ",    QuadEntryType::NotEqual},
        {"LEQ",    QuadEntryType::LessEqual},
        {"LSS",    QuadEntryType::LessThan},
        {"GRE",    QuadEntryType::GreaterThan},
        {"GEQ",    QuadEntryType::GreaterEqual}


    };

    const std::map<string, QuadEntryType> _symbol2unaryOp = {
        {"NOT",  QuadEntryType::Not},
        {"MINU", QuadEntryType::Neg},
    };


public:
    static ICGenerator *getInstance();

    std::vector<QuadEntry *> *mainEntries;  // 包括全局变量、常量和main函数

    std::map<int, std::string *> *id2allPureString;

    std::map<std::string *, QuadItemFunc *> *name2quadItemFunc;
    std::list<std::string * > *quadItemFuncOrder;

    QuadItemFunc *currentFunc;

    bool inFunc{false};

    QuadEntryType symbol2binaryOp(Symbol symbol) const;

    QuadEntryType symbolName2binaryOp(string symbol) const;

    QuadEntryType symbol2unaryOp(string name) const;

    /* 常量声明 */

    void translate_ConstVarDef(bool isGlobal, SymbolTableEntry *tableEntry,
                               SymbolTable *currentTable) const;

    void translate_ConstArrayDef(bool isGlobal, SymbolTableEntry *tableEntry, int d1,
                                  SymbolTable *currentTable) const;

    void translate_ConstArray2Def(bool isGlobal, SymbolTableEntry *tableEntry, int d1, int d2,
                                  SymbolTable *currentTable) const;

    /* 变量声明 */
    void translate_VarDef(QuadItem *initItem, bool isGlobal,
                          SymbolTableEntry *tableEntry, bool hasInitVal,
                          SymbolTable *currentTable) const;

    void translate_ArrayDef(QuadItem *initItem, bool isGlobal,
                            SymbolTableEntry *tableEntry, bool hasInitVal, int length,
                            SymbolTable *currentTable) const;

    /* 双目运算符 */
    void translate_BinaryOperator(
            QuadEntryType QuadEntryType, QuadItem *dst, QuadItem *src1, QuadItem *src2 = nullptr) const;

    /* 单目运算符 */
    void translate_UnaryOperator(QuadEntryType QuadEntryType, QuadItem *dst, QuadItem *src) const;

    /* getint() */
    void translate_getint(QuadItem *dst) const;

    void translate_getchar(QuadItem *dst) const;

    /* printf */
    void translate_printf(std::vector<int> *indexOfPercentSign,
                          std::vector<QuadItem *> *intItems,
                          std::string *s) const;

    /* 转到主函数 */
    void translate_MainFunc() const;

    void entryOutput(FILE *fp);

    void translate_Bnez(QuadItem *condition, QuadItemLabel *label) const;

    void translate_Beqz(QuadItem *condition, QuadItemLabel *label) const;

    void translate_JumpLabel(QuadItemLabel *label) const;

    void translate_InsertLabel(QuadItemLabel *label) const;

    /* 函数定义 */
    QuadItemFunc *translate_FuncDef(SymbolTableEntry *funcEntry,
                                  SymbolTable *currentTable) const;

    /* 函数调用 */
    void translate_FuncCall(std::string *funcName, std::vector<QuadItem *> *params = nullptr) const;

    /* 函数返回 */
    void translate_return(QuadItem *) const;

    void translate_return() const;




};

#endif //QuadGENERATOR_H
