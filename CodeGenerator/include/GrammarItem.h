//
// Created by ROILYY on 24-10-9.
//

#ifndef GRAMMARITEM_H
#define GRAMMARITEM_H
#include <string>

enum class GrammarItem {
   Forstmt,
   CharacterItem,
   BlockItem,
   BType,
   Decl,
   // 需要输出
   CompUnit,
   ConstDecl,
   VarDecl,
   ConstDef,
   ConstInitVal,
   VarDef,
   InitVal,
   FuncDef,
   MainFuncDef,
   FuncType,
   FuncFParams,
   FuncFParam,
   Block,
   Stmt,
   Exp,
   Cond,
   LVal,
   PrimaryExp,
   Number,
   UnaryExp,
   UnaryOp,
   FuncRParams,
   MulExp,
   AddExp,
   RelExp,
   EqExp,
   LAndExp,
   LOrExp,
   ConstExp
};

std::string grammarItemToString(GrammarItem item);


#endif //GRAMMARITEM_H
