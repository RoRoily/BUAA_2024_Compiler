//
// Created by 郑慕函 on 24-10-14.
//
#include "../include/GrammarItem.h"

#include <string>

std::string grammarItemToString(GrammarItem item) {
    switch (item) {
        case GrammarItem::Forstmt: return "ForStmt";
        case GrammarItem::CharacterItem: return "Character";
        case GrammarItem::BlockItem: return "BlockItem";
        case GrammarItem::BType: return "BType";
        case GrammarItem::Decl: return "Decl";
        case GrammarItem::CompUnit: return "CompUnit";
        case GrammarItem::ConstDecl: return "ConstDecl";
        case GrammarItem::VarDecl: return "VarDecl";
        case GrammarItem::ConstDef: return "ConstDef";
        case GrammarItem::ConstInitVal: return "ConstInitVal";
        case GrammarItem::VarDef: return "VarDef";
        case GrammarItem::InitVal: return "InitVal";
        case GrammarItem::FuncDef: return "FuncDef";
        case GrammarItem::MainFuncDef: return "MainFuncDef";
        case GrammarItem::FuncType: return "FuncType";
        case GrammarItem::FuncFParams: return "FuncFParams";
        case GrammarItem::FuncFParam: return "FuncFParam";
        case GrammarItem::Block: return "Block";
        case GrammarItem::Stmt: return "Stmt";
        case GrammarItem::Exp: return "Exp";
        case GrammarItem::Cond: return "Cond";
        case GrammarItem::LVal: return "LVal";
        case GrammarItem::PrimaryExp: return "PrimaryExp";
        case GrammarItem::Number: return "Number";
        case GrammarItem::UnaryExp: return "UnaryExp";
        case GrammarItem::UnaryOp: return "UnaryOp";
        case GrammarItem::FuncRParams: return "FuncRParams";
        case GrammarItem::MulExp: return "MulExp";
        case GrammarItem::AddExp: return "AddExp";
        case GrammarItem::RelExp: return "RelExp";
        case GrammarItem::EqExp: return "EqExp";
        case GrammarItem::LAndExp: return "LAndExp";
        case GrammarItem::LOrExp: return "LOrExp";
        case GrammarItem::ConstExp: return "ConstExp";
        default: return "Unknown";
    }
}


