//
// Created by 郑慕函 on 2024/10/9.
//

#include "../item/tree/include/Node.h"
#include "Lexer.h"
#ifndef PARSER_H
#define PARSER_H
using namespace std;
class Parser {
    static Parser* parserInstance;

    Word * currentWord;
    int tokenPos  = 0;
public:
    Lexer *lexer = Lexer::getLexerInstance();
    static Parser *getParserInstance();

    //获取下一个单词
    void nextToken();

    //错误处理
    void HandleMistake();


    Node *parse();

    //处理语法条目
    Node * CompUnit(int depth); // 编译单元
    Node *Decl(int depth);

    Node * Decel(int depth); //声明
    Node * ConstDecl(int depth);
    Node * BType(int depth);
    Node * ConstDef(int depth);
    Node * ConstInitVal(int depth);
    Node * VarDecl(int depth);
    Node * VarDef(int depth);
    Node * InitVal(int depth);
    Node * FuncDef(int depth);
    Node * MainFuncDef(int depth);

    Node *FuncType(int depth);

    Node * Functype(int depth);
    Node * FuncFParams(int depth);
    Node * FuncFParam(int depth);
    Node * Block(int depth);
    Node * BlockItem(int depth);
    Node * Stmt(int depth);

    Node *Stmt2(int depth);

    Node * ForStmt(int depth);
    Node * Exp(int depth);
    Node * Cond(int depth);
    Node * LVal(int depth);
    Node * PrimaryExp(int depth);
    Node * Number(int depth);

    Node *IntConst(int depth);

    Node * Character(int depth);

    Node *CharConst(int depth);

    Node * UnaryExp(int depth);
    Node * UnaryOp(int depth);
    Node * FuncRParams(int depth);
    Node * MulExp(int depth);
    Node * AddExp(int depth);
    Node * RelExp(int depth);
    Node * EqExp(int depth);
    Node * LAndExp(int depth);
    Node * LOrExp(int depth);
    Node * ConstExp(int depth);

    Node *Ident(int depth);

    Node *StringConst(int depth);

    bool semicn_before_assign();

    //错误处理
    void handleErrorI(int lineNum) const;

    void handleErrorJ(int lineNum) const;

    void handleErrorK(int lineNum) const;

private:
};
#endif //PARSER_H
