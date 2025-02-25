//
// Created by 郑慕函 on 24-10-17.
//

#ifndef SYMBOLTABLEHANDLE_H
#define SYMBOLTABLEHANDLE_H
#include "../../item/symbolTable/include/ConstValue.h"
#include "../../item/symbolTable/include/SymbolTable.h"
#include "../../item/symbolTable/include/SymbolTableEntryType.h"
#include "../../item/symbolTable/include/SymbolTableEntry.h"
#include "ICGenerator.h"
#include "../../item/tree/include/Node.h"
class SymbolTableHandler {
public:
    Node * root;
    int count = 1;
    SymbolTable * currentTable;
    ICGenerator *icGenerator;

    vector<SymbolTable * > symbolTableList;
    Lexer * lexer;

    explicit SymbolTableHandler(Node *);

    void buildTable();

    //错误处理
    void handleIdentRedefined(int line_num) const;//b

    void handleIdentUndefined(int line_num) const;//c

    void handleParamNumNotMatch(int line_num) const;//d

    void handleParamTypeNotMatch(int line_num) const;//e

    void handleReturnRedundant(int line_num) const;//f

    void handleReturnMissing(int line_num) const;//g

    void handleConstantModification(int line_num) const;//h

    void handleFormatStrNumNotMatch(int line_num) const;//l

    void handleredundantBreakContinue(int line_num) const;//m
    bool findCharacter(Node *node);

    bool findNumber(Node *node);

    bool CHARCON_befor_SEMICN(int tokenPos) const;

    bool STRCON_befor_SEMICN(int tokenPos) const;

    bool CHARTK_at_BType(int tokenPos) const;

    int get_StringConst(int tokenPos) const;


    void Stmt(Node * node,bool inFuncBlock=false);

    void ForStmt(Node *node,int * temp);

    void CompUnit(Node *node);

    void Decl(Node *node);

    void FuncDef(Node *node);

    void ConstDecl(Node *node);

    void VarDecl(Node *node);

    void MainFuncDef(Node *node);

    void VarDef(Node *node);

    void ConstDef(Node *node);

    int ConstExp(Node *node, bool isChar);

    SymbolTableEntry *AddExp(Node *node, bool fromConstExp, int *constExpValue, QuadItem *quadItem, bool isChar);

    void InitVal(Node *node, int d, QuadItem *quadItem, int tokenPos);

    ConstValue *ConstInitVal(Node *node, int d,int tokenPos);

    SymbolTableEntry *MulExp(Node *node, bool fromConstExp, int *constExpValue, QuadItem *quadItem);

    SymbolTableEntry *UnaryExp(Node *node, bool fromConstExp, int *constExpValue, QuadItem *quadItem);

    SymbolTableEntry *PrimaryExp(Node *node, bool fromConstExp, int *constExpValue, QuadItem *quadItem);

    std::vector<SymbolTableEntry *> *FuncRParams(Node *node, SymbolTableEntry *entry, std::vector<QuadItem *> *params);

    bool findParamError(SymbolTableEntry *definedEntry, std::vector<SymbolTableEntry *> *calledEntry, int lineNum);

    SymbolTableEntry *Exp(Node *node, bool fromConstExp, int *constExpValue, QuadItem *quadItem);

    SymbolTableEntry *LVal(Node *node, bool fromConstExp, int *constExpValue, QuadItem *quadItem);

    SymbolTableEntry *Number(Node *node, bool fromConstExp, int *constExpValue, QuadItem *quadItem);

    SymbolTableEntry *Character(Node *node, bool fromConstExp, int *constExpValue, QuadItem *quadItem);

    std::vector<SymbolTableEntry *> *
    check_FuncRParams(Node *node, SymbolTableEntry *entry, std::vector<QuadItem *> *params);

    void FuncFParams(Node *funcFParams, SymbolTableEntry *funcIdentEntry);

    SymbolTableEntry *FuncFParam(Node *funcFParam, SymbolTableEntryType retType);

    void Block(Node *block, bool inFuncBlock=false);

    void BlockItem(Node *blockItem, bool inFuncBlock=false);

    void Cond(Node *node, QuadItem *quadItem);

    bool StringConst(Node *node, int *formatNum, std::vector<int> *PosOfPer, std::vector<int> *PosofChar);

    SymbolTableEntry *check_LVal(Node *node, bool fromConstExp, int *constExpValue);

    void LOrExp(Node *node, QuadItem *quad_item);

    void LAndExp(Node *node, QuadItem *icItem);

    void EqExp(Node *node, QuadItem *icItem);

    void RelExp(Node *node, QuadItem *icItem);

    void PrintSymbolTable(Node *node,FILE * fp) const;

    //中间代码生成
    void output(FILE *fp1);

    void Analyse_forStmt_cond(int tokenPos,int * ifCond) const;
};
#endif //SYMBOLTABLEHANDLE_H
