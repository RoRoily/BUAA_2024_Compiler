//
// Created by ROILYY on 24-10-9.
//

#ifndef SYMBOL_H
#define SYMBOL_H
#include <map>
enum class Symbol {
    /* 保留字 */
    MAINTK, // main
    CONSTTK, // item
    INTTK, // int
    BREAKTK, // break
    CONTINUETK, // continue
    IFTK, // if
    ELSETK, // else
    WHILETK, // while
    GETINTTK, // getint
    PRINTFTK, // printf
    RETURNTK, // return
    VOIDTK, // void

    /* 其他 */
    IDENFR, // Ident
    INTCON, // IntConst
    STRCON, // FormatString
    NOT, // !
    AND, // &&
    OR, // ||
    PLUS, // +
    MINU, // -
    MULT, // *
    DIV, // /
    MOD, // %
    LSS, // <
    POWER,
    LEQ, // <=
    GRE, // >
    GEQ, // >=
    EQL, // ==
    NEQ, // !=
    ASSIGN, // =
    SEMICN, // ;
    COMMA, // ,
    LPARENT, // (
    RPARENT, // )
    LBRACK, // [
    RBRACK, // ]
    LBRACE, // {
    RBRACE,  // }
};
#endif //SYMBOL_H
