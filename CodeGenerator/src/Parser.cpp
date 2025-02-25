//
// Created by 郑慕函 on 2024/10/9.
//
#include "../include/Parser.h"
#include "../item/tree/include/Node.h"
#include <cassert>
#include "../include/Symbol.h"
#include "../include/Lexer.h"

Parser *Parser::parserInstance = new Parser();

Parser *Parser::getParserInstance() {
    if(parserInstance == nullptr) {
        parserInstance = new Parser();
    }
    return parserInstance;
}
//接收下一个token的值
void Parser::nextToken() {
    if(tokenPos < lexer->tokenMap.size()-1) {
        this->currentWord = &(lexer->tokenMap[++tokenPos]);
        //printf("%d %s %s\n",tokenPos,this->currentWord->word.c_str(),this->currentWord->word_type.c_str());
    }else {
        return;
    }
}


//错误处理
void Parser::handleErrorI(int lineNum) const {
    if(!lexer->lineWrong[lineNum])lexer->lineWrong[lineNum] = 'i';
}

void Parser::handleErrorJ(int lineNum) const {
    if(!lexer->lineWrong[lineNum])lexer->lineWrong[lineNum] = 'j';
}

void Parser::handleErrorK(int lineNum) const {
    if(!lexer->lineWrong[lineNum])lexer ->lineWrong[lineNum] = 'k';
}




//实现简单的递归下降解析器
//将Token转换为词法树


Node *Parser::parse() {
    //printf("scussess go into parse\n");
    //this->nextToken();
    this->currentWord = &(lexer->tokenMap[tokenPos]);//将第一个token转换为currentWord
    //printf("%d %s %s\n",tokenPos,this->currentWord->word.c_str(),this->currentWord->word_type.c_str());
    Node *root = CompUnit(1);
    return root;
}



/* CompUnit → {Decl} {FuncDef} MainFuncDef */
//1.CompUnit → {Decl} {FuncDef} MainFuncDef
Node *Parser::CompUnit(int depth) {
    printf("CompUnit\n");
    Node *root = new Node(GrammarItem::CompUnit, nullptr, 1);
    Node *child = nullptr;
    while (lexer->tokenMap[tokenPos+2].word_type != "LPARENT") { //有多个Decl 多个变量定义
        //printf("%d %s\n",tokenPos,lexer->tokenMap[tokenPos+2].word_type.c_str());
        child = this->Decl(depth + 1);
        root->addChild(child);
        child->setParent(root);
    }
    while (lexer ->tokenMap[tokenPos + 1].word_type != "MAINTK") { //多个函数定义
        child = this->FuncDef(depth + 1);
        root->addChild(child);
        child->setParent(root);
    }
    //MainFuncDef
    child = this->MainFuncDef(depth + 1);
    root->addChild(child);
    child->setParent(root);
    return root;
}

/* Decl → ConstDecl | VarDecl */
//2.Decl → ConstDecl | VarDecl
Node *Parser::Decl(int depth) {
    printf("Decl\n");
    Node *decl = new Node(GrammarItem::Decl, depth);
    Node *child = nullptr;
    if (this->currentWord->word_type == "CONSTTK") {
        child = this->ConstDecl(depth + 1);
    } else {
        child = this->VarDecl(depth + 1);
    }
    decl->addChild(child);
    child->setParent(decl);
    return decl;
}



/* ConstDecl → 'const' BType ConstDef { ',' ConstDef } ';' */
//3.ConstDecl → 'const' BType ConstDef { ',' ConstDef } ';' // i
Node *Parser::ConstDecl(int depth) {
    printf("ConstDecl\n");
    Node *constDecl = new Node(GrammarItem::ConstDecl, depth);
    Node *child = nullptr;
    // 'const'
    if(this->currentWord->word_type == "CONSTTK") {
        constDecl->addChild(new Node(this->currentWord, constDecl, depth));
        this->nextToken();
    }
    // BType
    child = this->BType(depth + 1);
    child->setParent(constDecl);
    constDecl->addChild(child);
    // ConstDef
    child = this->ConstDef(depth + 1);
    child->setParent(constDecl);
    constDecl->addChild(child);
    // { ',' ConstDef }
    while (this->currentWord->word_type == "COMMA") {
        constDecl->addChild(new Node(this->currentWord, constDecl, depth + 1));
        this->nextToken();
        child = this->ConstDef(depth + 1);
        constDecl->addChild(child);
    }
    // ';' 错误处理
    if(this->currentWord->word_type == "SEMICN")
    {
        constDecl->addChild(new Node(this->currentWord, constDecl, depth + 1));
        this->nextToken();
        return constDecl;
    }else {
        Word * semi =  new Word("SEMICN",";",lexer ->tokenMap[tokenPos-1] .line_num);
        constDecl->addChild(new Node(semi, constDecl, depth + 1));
        handleErrorI(lexer ->tokenMap[tokenPos-1] .line_num);
        return constDecl;
    }
}

/* BType → 'int' */
//4.BType → 'int' | 'char'
Node *Parser::BType(int depth) {
    printf("btype %d %s %s \n",tokenPos,this->currentWord->word_type.c_str(),this->currentWord->word.c_str());
    Node *bType = new Node(GrammarItem::BType, depth);
    if(this->currentWord->word_type == "INTTK" || this->currentWord->word_type == "CHARTK") {
        bType->addChild(new Node(this->currentWord, bType, depth));
        this->nextToken();
    }
    return bType;
}


/* ConstDef → Ident { '[' ConstExp ']' } '=' ConstInitVal */
//5.ConstDef → Ident [ '[' ConstExp ']' ] '=' ConstInitVal // k
Node *Parser::ConstDef(int depth) {
    printf("ConstDef\n");
    Node *constDef = new Node(GrammarItem::ConstDef, depth);
    Node *child = nullptr;
    // Ident
    child = Ident(depth + 1);
    child->setParent(constDef);
    constDef->addChild(child);
    // { '[' ConstExp ']' }
    while (this->currentWord->word_type == "LBRACK") {
        // '['
        constDef->addChild(new Node(this->currentWord, constDef, depth + 1));
        this->nextToken();
        // ConstExp
        child = this->ConstExp(depth + 1);
        child->setParent(constDef);
        constDef->addChild(child);
        // ']' 错误处理
        if(this->currentWord->word_type == "RBRACK")
        {
            constDef->addChild(new Node(this->currentWord, constDef, depth + 1));
            this->nextToken();
        }else {
            Word * semi =  new Word("RBREAK","]",lexer ->tokenMap[tokenPos-1] .line_num);
            constDef->addChild(new Node(semi, constDef, depth + 1));
            handleErrorK(lexer ->tokenMap[tokenPos-1] .line_num);
        }

    }
    // '='
    if(this->currentWord->word_type == "ASSIGN") {
        constDef->addChild(new Node(this->currentWord, constDef, depth + 1));
        this->nextToken();
    }
    // ConstInitVal
    child = ConstInitVal(depth + 1);
    child->setParent(constDef);
    constDef->addChild(child);
    return constDef;
}



/* ConstInitVal → ConstExp | '{' [ ConstInitVal { ',' ConstInitVal } ] '}' */
//6.ConstInitVal → ConstExp | '{' [ ConstExp { ',' ConstExp } ] '}' | StringConst
Node *Parser::ConstInitVal(int depth) {
    Node *constInitVal = new Node(GrammarItem::ConstInitVal, depth);
    Node *child = nullptr;

    if (this->currentWord->word_type != "LBRACE" && this->currentWord->word_type != "STRCON") {  // ConstExp
        child = ConstExp(depth + 1);
        child->setParent(constInitVal);
        constInitVal->addChild(child);
    }
    else if(this->currentWord ->word_type == "STRCON") {// StringConst
        child = StringConst(depth +1);
        child -> setParent(constInitVal);
        constInitVal->addChild(child);
    }
    else {  // '{' [ ConstExp { ',' ConstExp } ] '}'
        constInitVal->addChild(new Node(this->currentWord, constInitVal, depth + 1));
        this->nextToken();
        if (this->currentWord->word_type != "RBRACE") {
            // ConstInitVal
            child = ConstExp(depth + 1);
            child->setParent(constInitVal);
            constInitVal->addChild(child);
            // { ',' ConstInitVal }
            while (this->currentWord->word_type == "COMMA") {
                // ','
                constInitVal->addChild(new Node(this->currentWord, constInitVal, depth + 1));
                this->nextToken();
                // ConstInitVal
                child = ConstExp(depth + 1);
                child->setParent(constInitVal);
                constInitVal->addChild(child);
            }
        }
        constInitVal->addChild(new Node(this->currentWord, constInitVal, depth + 1));
        this->nextToken();
    }
    return constInitVal;
}



/* VarDecl → BType VarDef { ',' VarDef } ';' */
//7.VarDecl → BType VarDef { ',' VarDef } ';' // i
Node *Parser::VarDecl(int depth) {
    printf("VarDecl %d\n",tokenPos);
    Node *varDecl = new Node(GrammarItem::VarDecl, depth);
    Node *child = nullptr;
    // BType
    child = BType(depth + 1);
    child->setParent(varDecl);
    varDecl->addChild(child);
    // VarDef
    child = VarDef(depth + 1);
    child->setParent(varDecl);
    varDecl->addChild(child);
    // { ',' VarDef }
    while (this->currentWord->word_type == "COMMA") {
        varDecl->addChild(new Node(this->currentWord, varDecl, depth + 1));
        this->nextToken();

        child = VarDef(depth + 1);
        child->setParent(varDecl);
        varDecl->addChild(child);
    }
    // ';' 错误处理
    if(this->currentWord->word_type == "SEMICN") {
        varDecl->addChild(new Node(this->currentWord, varDecl, depth + 1));
        this->nextToken();
        return varDecl;
    }else {
        Word * semi =  new Word("SEMICN",";",lexer ->tokenMap[tokenPos-1] .line_num);
        varDecl->addChild(new Node(semi, varDecl, depth + 1));
        handleErrorI(lexer ->tokenMap[tokenPos-1] .line_num);
        return varDecl;
    }
}




/* VarDef → Ident { '[' ConstExp ']' }  |  Ident { '[' ConstExp ']' } '=' InitVal */
//8. VarDef → Ident [ '[' ConstExp ']' ] | Ident [ '[' ConstExp ']' ] '=' InitVal // k
//(未解决)
Node *Parser::VarDef(int depth) {
    Node *varDef = new Node(GrammarItem::VarDef, depth);
    Node *child = nullptr;
    // Ident
    child = Ident(depth + 1);
    child->setParent(varDef);
    varDef->addChild(child);
    // { '[' ConstExp ']' }
    while (this->currentWord->word_type == "LBRACK") {
        //'['
        varDef->addChild(new Node(this->currentWord, varDef, depth + 1));
        this->nextToken();
        //ConstExp
        child = ConstExp(depth + 1);
        child->setParent(varDef);
        varDef->addChild(child);
        //']' 错误处理
        if(this->currentWord->word_type == "RBRACK") {
            varDef->addChild(new Node(this->currentWord, varDef, depth + 1));
            this->nextToken();
        }else {
            Word * semi =  new Word("RBREAK","]",lexer ->tokenMap[tokenPos-1] .line_num);
            varDef->addChild(new Node(semi, varDef, depth + 1));
            handleErrorK(lexer ->tokenMap[tokenPos-1] .line_num);
        }
    }
    if (this->currentWord->word_type == "ASSIGN") {  // '=' InitVal
        varDef->addChild(new Node(this->currentWord, varDef, depth + 1));
        this->nextToken();

        child = InitVal(depth + 1);
        child->setParent(varDef);
        varDef->addChild(child);
    }
    return varDef;
}



/* InitVal → Exp | '{' [ InitVal { ',' InitVal } ] '}' */
//9. InitVal → Exp | '{' [ Exp { ',' Exp } ] '}' | StringConst
Node *Parser::InitVal(int depth) {
    Node *initVal = new Node(GrammarItem::InitVal, depth);
    Node *child = nullptr;
    if(this->currentWord ->word_type == "STRCON") {//StringConst
        child = this->StringConst(depth + 1);
        child->setParent(initVal);
        initVal->addChild(child);
    }
    else if (this->currentWord->word_type == "LBRACE") {  // '{' [ InitVal { ',' InitVal } ] '}'
        // '{'
        initVal->addChild(new Node(this->currentWord, initVal, depth + 1));
        this->nextToken();
        // [ InitVal { ',' InitVal } ]
        if (this->currentWord->word_type != "RBRACE") {
            // InitVal
            child = this->Exp(depth + 1);
            child->setParent(initVal);
            initVal->addChild(child);
            // { ',' InitVal }
            while (this->currentWord->word_type == "COMMA") {
                // ','
                initVal->addChild(new Node(this->currentWord, initVal, depth + 1));
                this->nextToken();
                // InitVal
                child = this->Exp(depth + 1);
                child->setParent(initVal);
                initVal->addChild(child);
            }
        }
        // '}'
        initVal->addChild(new Node(this->currentWord, initVal, depth + 1));
        this->nextToken();
    }
    else { // Exp
        child = this->Exp(depth + 1);
        child->setParent(initVal);
        initVal->addChild(child);
    }
    return initVal;
}


/* FuncDef → FuncType Ident '(' [FuncFParams] ')' Block */
//10. FuncDef → FuncType Ident '(' [FuncFParams] ')' Block // j
Node *Parser::FuncDef(int depth) {

    Node *funcDef = new Node(GrammarItem::FuncDef, depth);
    Node *child = nullptr;
    // FuncType
    child = this->FuncType(depth + 1);
    child->setParent(funcDef);
    funcDef->addChild(child);
    // Ident
    child = this->Ident(depth + 1);
    child->setParent(funcDef);
    funcDef->addChild(child);
    // '('
    if(this->currentWord->word_type == "LPARENT") {
        funcDef->addChild(new Node(this->currentWord, funcDef, depth + 1));
        this->nextToken();
    }
    // [FuncFParams]
    // 需要修改
    if (this->currentWord->word_type != "RPARENT" && this->currentWord->word_type!= "LBRACE") {
        // FuncFParams
        child = this->FuncFParams(depth + 1);
        child->setParent(funcDef);
        funcDef->addChild(child);
    }
    // ')' 错误处理
    if(this->currentWord->word_type == "RPARENT") {
        funcDef->addChild(new Node(this->currentWord, funcDef, depth + 1));
        this->nextToken();
    }else {
        Word * semi =  new Word("RPARENT",")",lexer ->tokenMap[tokenPos-1] .line_num);
        funcDef->addChild(new Node(semi, funcDef, depth + 1));
        handleErrorJ(lexer ->tokenMap[tokenPos-1] .line_num);
    }
    // Block
    child = this->Block(depth + 1);
    child->setParent(funcDef);
    funcDef->addChild(child);

    return funcDef;
}

/* MainFuncDef → 'int' 'main' '(' ')' Block */
//11.MainFuncDef → 'int' 'main' '(' ')' Block // j
Node *Parser::MainFuncDef(int depth) {
    printf("MainFuncDef\n");
    Node *mainFuncDef = new Node(GrammarItem::MainFuncDef, depth);
    Node *child = nullptr;
    // 'int'
    if(this->currentWord->word_type == "INTTK") {
        mainFuncDef->addChild(new Node(this->currentWord, mainFuncDef, depth + 1));
        this->nextToken();
    }

    // 'main'
    if(this->currentWord->word_type == "MAINTK") {
        mainFuncDef->addChild(new Node(this->currentWord, mainFuncDef, depth + 1));
        this->nextToken();
    }
    // '('
    if(this->currentWord->word_type == "LPARENT") {
        mainFuncDef->addChild(new Node(this->currentWord, mainFuncDef, depth + 1));
        this->nextToken();
    }
    // ')' 错误处理
    if(this->currentWord->word_type == "RPARENT") {
        mainFuncDef->addChild(new Node(this->currentWord, mainFuncDef, depth + 1));
        this->nextToken();
    }else {
        Word * semi =  new Word("RPARENT",")",lexer ->tokenMap[tokenPos-1] .line_num);
        mainFuncDef->addChild(new Node(semi, mainFuncDef, depth + 1));
        handleErrorJ(lexer ->tokenMap[tokenPos-1] .line_num);
    }
    // Block
    child = this->Block(depth + 1);
    child->setParent(mainFuncDef);
    mainFuncDef->addChild(child);
    return mainFuncDef;
}


/* FuncType → 'void' | 'int' */
//12. FuncType → 'void' | 'int' | 'char'
Node *Parser::FuncType(int depth) {
    Node *funcType = new Node(GrammarItem::FuncType, depth);
    if(this->currentWord->word_type == "VOIDTK" || this->currentWord->word_type == "INTTK" || this -> currentWord -> word_type == "CHARTK") {
        funcType->addChild(new Node(this->currentWord, funcType, depth + 1));
        this->nextToken();
    }
    return funcType;
}


/* FuncFParams → FuncFParam { ',' FuncFParam } */
//13.FuncFParams → FuncFParam { ',' FuncFParam }
Node *Parser::FuncFParams(int depth) {
    printf("FuncFPrams %d %s %s\n",tokenPos,this->currentWord->word.c_str(),this->currentWord->word_type.c_str());
    Node *funcFParams = new Node(GrammarItem::FuncFParams, depth);
    Node *child = nullptr;
    // FuncFParam
    child = this->FuncFParam(depth + 1);
    child->setParent(funcFParams);
    funcFParams->addChild(child);
    // { ',' FuncFParam }
    while (this->currentWord->word_type == "COMMA") {
        // ','
        funcFParams->addChild(new Node(this->currentWord, funcFParams, depth + 1));
        this->nextToken();
        // FuncFParam
        child = this->FuncFParam(depth + 1);
        child->setParent(funcFParams);
        funcFParams->addChild(child);
    }
    return funcFParams;
}





/* FuncFParam → BType Ident ['[' ']' { '[' ConstExp ']' }] */
//14.FuncFParam → BType Ident ['[' ']'] // k
//（未解决）
Node *Parser::FuncFParam(int depth) {
    printf("FuncFPram %d %s %s\n",tokenPos,this->currentWord->word.c_str(),this->currentWord->word_type.c_str());
    Node *funcFParam = new Node(GrammarItem::FuncFParam, depth);
    Node *child = nullptr;
    // BType
    child = this->BType(depth + 1);
    child->setParent(funcFParam);
    funcFParam->addChild(child);
    // Ident
    child = this->Ident(depth + 1);
    child->setParent(funcFParam);
    funcFParam->addChild(child);
    // ['[' ']' ]
    if (this->currentWord->word_type == "LBRACK") {
        // '['
        funcFParam->addChild(new Node(this->currentWord, funcFParam, depth + 1));
        this->nextToken();
        // ']' 错误处理
        if(this->currentWord->word_type == "RBRACK") {
            funcFParam->addChild(new Node(this->currentWord, funcFParam, depth + 1));
            this->nextToken();
        }else {
            Word * semi =  new Word("RBREAK","]",lexer ->tokenMap[tokenPos-1] .line_num);
            funcFParam->addChild(new Node(semi, funcFParam, depth + 1));
            handleErrorK(lexer ->tokenMap[tokenPos-1] .line_num);
        }
    }
    return funcFParam;
}


/* Block → '{' { BlockItem } '}' */
//15.Block → '{' { BlockItem } '}'
Node *Parser::Block(int depth) {
    printf("Block %d\n",depth);
    Node *block = new Node(GrammarItem::Block, depth);
    Node *child = nullptr;
    // '{'
    if(this->currentWord->word_type == "LBRACE") {
        block->addChild(new Node(this->currentWord, block, depth + 1));
        this->nextToken();
    }
    // { BlockItem }
    while (this->currentWord->word_type != "RBRACE") {
        child = this->BlockItem(depth + 1);
        child->setParent(block);
        block->addChild(child);
    }
    // '}'
    block->addChild(new Node(this->currentWord, block, depth + 1));
    this->nextToken();
    return block;
}



/* BlockItem → Decl | Stmt */
//16.BlockItem → Decl | Stmt
Node *Parser::BlockItem(int depth) {
    printf("BlockItem %d\n",depth);
    Node *blockItem = new Node(GrammarItem::BlockItem, depth);
    Node *child = nullptr;
    // Decl → ConstDecl | VarDecl, 以 const 或 int 开头
    if (this->currentWord->word_type == "CONSTTK" || this->currentWord->word_type == "INTTK" || this->currentWord->word_type == "CHARTK") {
        printf("BlockItem -> Decl %d\n",this->tokenPos);
        child = this->Decl(depth + 1);
    } else {
        printf("BlockItem -> Stmt %d\n",this->tokenPos);
        child = this->Stmt(depth + 1);
    }
    child->setParent(blockItem);
    blockItem->addChild(child);
    return blockItem;
}





// 17.Stmt → LVal '=' Exp ';' // i (#)
// | [Exp] ';' // i (未解决)
// | Block (#)
// | 'if' '(' Cond ')' Stmt [ 'else' Stmt ] // j(#)
// | 'for' '(' [ForStmt] ';' [Cond] ';' [ForStmt] ')' Stmt(#) //不需要错误处理吗
// | 'break' ';' | 'continue' ';' // i (#)
// | 'return' [Exp] ';' // i(#)
// | LVal '=' 'getint''('')'';' // i j (#)
// | LVal '=' 'getchar''('')'';' // i j (#)
// | 'printf''('StringConst {','Exp}')'';' // i j(#)
Node *Parser::Stmt(int depth) {
    printf("Stmt %d\n",depth);
    Node *stmt = new Node(GrammarItem::Stmt, depth);
    Node *child = nullptr;
    if (this->currentWord->word_type == "PRINTFTK") {  // 'printf''('FormatString{','Exp}')'';'  i j
        stmt->addChild(new Node(this->currentWord, stmt, depth + 1));
        this->nextToken();
        // '('
        if(this->currentWord->word_type == "LPARENT") {
            stmt->addChild(new Node(this->currentWord, stmt, depth + 1));
            this->nextToken();
        }
        // FormatString //FormatString 改名为StringConst
        child = this->StringConst(depth + 1);
        child->setParent(stmt);
        stmt->addChild(child);
        // {','Exp}
        while (this->currentWord->word_type == "COMMA") {
            stmt->addChild(new Node(this->currentWord, stmt, depth + 1));
            this->nextToken();
            // Exp
            child = this->Exp(depth + 1);
            child->setParent(stmt);
            stmt->addChild(child);
        }
        // ')' 错误处理
        if(this->currentWord->word_type == "RPARENT") {
            stmt->addChild(new Node(this->currentWord, stmt, depth + 1));
            this->nextToken();
        }else {
            Word * semi =  new Word("RPARENT",")",lexer ->tokenMap[tokenPos-1] .line_num);
            stmt->addChild(new Node(semi, stmt, depth + 1));
            handleErrorJ(lexer ->tokenMap[tokenPos-1] .line_num);
        }
        // ';' 错误处理
        if(this->currentWord->word_type == "SEMICN") {
            stmt->addChild(new Node(this->currentWord, stmt, depth + 1));
            this->nextToken();
        }else {
            Word * semi =  new Word("SEMICN",";",lexer ->tokenMap[tokenPos-1] .line_num);
            stmt->addChild(new Node(semi, stmt, depth + 1));
            handleErrorI(lexer ->tokenMap[tokenPos-1] .line_num);
        }
    } else if (this->currentWord->word_type == "RETURNTK") {  // 'return' [Exp] ';'  i
        stmt->addChild(new Node(this->currentWord, stmt, depth + 1));
        this->nextToken();
        // [Exp]
        if (this->currentWord->word_type != "SEMICN") {
            child = this->Exp(depth + 1);
            child->setParent(stmt);
            stmt->addChild(child);
        }
        // ';' //错误处理
        if(this->currentWord->word_type == "SEMICN") {
            stmt->addChild(new Node(this->currentWord, stmt, depth + 1));
            this->nextToken();
        }else {
            Word * semi =  new Word("SEMICN",";",lexer ->tokenMap[tokenPos-1] .line_num);
            stmt->addChild(new Node(semi, stmt, depth + 1));
            handleErrorI(lexer ->tokenMap[tokenPos-1] .line_num);
        }
    } else if (this->currentWord->word_type == "BREAKTK" ||
               this->currentWord->word_type == "CONTINUETK") {  // 'break' ';' | 'continue' ';' i
        stmt->addChild(new Node(this->currentWord, stmt, depth + 1));
        this->nextToken();
        // ';' 错误处理
        if(this->currentWord->word_type == "SEMICN") {
            stmt->addChild(new Node(this->currentWord, stmt, depth + 1));
            this->nextToken();
        }else {
            Word * semi =  new Word("SEMICN",";",lexer ->tokenMap[tokenPos-1] .line_num);
            stmt->addChild(new Node(semi, stmt, depth + 1));
            handleErrorI(lexer ->tokenMap[tokenPos-1] .line_num);
        }
    } else if(this->currentWord -> word_type == "FORTK") { // | 'for' '(' [ForStmt] ';' [Cond] ';' [ForStmt] ')' Stmt
        stmt -> addChild(new Node(this->currentWord, stmt, depth + 1));
        this->nextToken();
        //'('
        if(this->currentWord->word_type == "LPARENT") {
            stmt->addChild(new Node(this->currentWord, stmt, depth + 1));
            this->nextToken();
        }
        //[ForStmt]
        if(this->currentWord->word_type != "SEMICN") {
            child = this->ForStmt(depth + 1);
            child->setParent(stmt);
            stmt->addChild(child);
        }
        //';'
        if(this->currentWord->word_type == "SEMICN") {
            stmt->addChild(new Node(this->currentWord, stmt, depth + 1));
            this->nextToken();
        }
        //[Cond]
        if(this->currentWord->word_type != "SEMICN") {
            child = this->Cond(depth + 1);
            child->setParent(stmt);
            stmt->addChild(child);
        }
        //;
        if(this->currentWord->word_type == "SEMICN") {
            stmt->addChild(new Node(this->currentWord, stmt, depth + 1));
            this->nextToken();
        }
        //[ForStmt]
        printf("in forstmt %s %s\n",this->currentWord->word_type.c_str(),this->currentWord->word.c_str());
        if(this->currentWord->word_type != "RPARENT") { //2024.10.14修改
            child = this->ForStmt(depth + 1);
            child->setParent(stmt);
            stmt->addChild(child);
        }
        //')' 错误处理
        if(this->currentWord->word_type == "RPARENT") {
            stmt->addChild(new Node(this->currentWord, stmt, depth + 1));
            this->nextToken();
        }else {
            Word * semi =  new Word("RPARENT",")",lexer ->tokenMap[tokenPos-1] .line_num);
            stmt->addChild(new Node(semi, stmt, depth + 1));
            handleErrorJ(lexer ->tokenMap[tokenPos-1] .line_num);
        }
        //Stmt
        child = this->Stmt(depth + 1);
        child->setParent(stmt);
        stmt->addChild(child);
    }else if (this->currentWord->word_type == "IFTK") {  // 'if' '(' Cond ')' Stmt [ 'else' Stmt ]  //j
        stmt->addChild(new Node(this->currentWord, stmt, depth + 1));
        this->nextToken();
        // '('
        if(this->currentWord->word_type == "LPARENT") {
            stmt->addChild(new Node(this->currentWord, stmt, depth + 1));
            this->nextToken();
        }
        // Cond
        child = this->Cond(depth + 1);
        child->setParent(stmt);
        stmt->addChild(child);
        // ')'
        if(this->currentWord->word_type == "RPARENT") {
            stmt->addChild(new Node(this->currentWord, stmt, depth + 1));
            this->nextToken();
        }else {
            Word * semi =  new Word("RPARENT",")",lexer ->tokenMap[tokenPos-1] .line_num);
            stmt->addChild(new Node(semi, stmt, depth + 1));
            handleErrorJ(lexer ->tokenMap[tokenPos-1] .line_num);
        }
        // Stmt
        child = this->Stmt(depth + 1);
        child->setParent(stmt);
        stmt->addChild(child);
        // [ 'else' Stmt ]
        if (this->currentWord->word_type == "ELSETK") {
            stmt->addChild(new Node(this->currentWord, stmt, depth + 1));
            this->nextToken();
            // Stmt
            child = this->Stmt(depth + 1);
            child->setParent(stmt);
            stmt->addChild(child);
        }
    } else if (this->currentWord->word_type == "LBRACE") {  // Block
        child = this->Block(depth + 1);
        child->setParent(stmt);
        stmt->addChild(child);
    } else if (this->semicn_before_assign()) {  // 分号比等号先出现: [Exp] ';' i
        if (this->currentWord->word_type != "SEMICN") {
            child = this->Exp(depth + 1);
            child->setParent(stmt);
            stmt->addChild(child);
        }
        // ';' 错误处理
        if(this->currentWord->word_type == "SEMICN") {
            stmt->addChild(new Node(this->currentWord, stmt, depth + 1));
            this->nextToken();
        }else {
            Word * semi =  new Word("SEMICN",";",lexer ->tokenMap[tokenPos-1] .line_num);
            stmt->addChild(new Node(semi, stmt, depth + 1));
            handleErrorI(lexer ->tokenMap[tokenPos-1] .line_num);
        }
    } else {  // LVal '=' Exp ';'  | LVal '=' 'getint''('')'';'  || | LVal '=' 'getchar''('')'';' // i j
        // LVal
        child = this->LVal(depth + 1);
        child->setParent(stmt);
        stmt->addChild(child);
        // '='
        if(this->currentWord->word_type == "ASSIGN") {
            stmt->addChild(new Node(this->currentWord, stmt, depth + 1));
            this->nextToken();
        }
        if (this->currentWord->word_type != "GETINTTK" && this->currentWord ->word_type != "GETCHARTK") {  // LVal '=' Exp ';'
            printf("yeah!\n");
            child = this->Exp(depth + 1);
            child->setParent(stmt);
            stmt->addChild(child);
        } else if(this ->currentWord ->word_type == "GETINTTK")     {  // LVal '=' 'getint''('')'';'
            stmt->addChild(new Node(this->currentWord, stmt, depth + 1));
            this->nextToken();
            // '('
            if(this->currentWord->word_type == "LPARENT") {
                stmt->addChild(new Node(this->currentWord, stmt, depth + 1));
                this->nextToken();
            }
            // ')' 错误处理
            if(this->currentWord->word_type == "RPARENT") {
                stmt->addChild(new Node(this->currentWord, stmt, depth + 1));
                this->nextToken();
            }else {
                Word * semi =  new Word("RPARENT",")",lexer ->tokenMap[tokenPos-1] .line_num);
                stmt->addChild(new Node(semi, stmt, depth + 1));
                handleErrorJ(lexer ->tokenMap[tokenPos-1] .line_num);
            }
        }else if(this->currentWord ->word_type == "GETCHARTK") {
            stmt->addChild(new Node(this->currentWord, stmt, depth + 1));
            this->nextToken();
            // '('
            if(this->currentWord->word_type == "LPARENT") {
                stmt->addChild(new Node(this->currentWord, stmt, depth + 1));
                this->nextToken();
            }
            // ')' 错误处理
            if(this->currentWord->word_type == "RPARENT") {
                stmt->addChild(new Node(this->currentWord, stmt, depth + 1));
                this->nextToken();
            }else {
                Word * semi =  new Word("RPARENT",")",lexer ->tokenMap[tokenPos-1] .line_num);
                stmt->addChild(new Node(semi, stmt, depth + 1));
                handleErrorJ(lexer ->tokenMap[tokenPos-1] .line_num);
        }
        }
        // ';' 错误处理
        if(this->currentWord->word_type == "SEMICN") {
            stmt->addChild(new Node(this->currentWord, stmt, depth + 1));
            this->nextToken();
        }else {
            Word * semi =  new Word("SEMICN",";",lexer ->tokenMap[tokenPos-1] .line_num);
            stmt->addChild(new Node(semi, stmt, depth + 1));
            handleErrorI(lexer ->tokenMap[tokenPos-1] .line_num);
        }
    }
    return stmt;
}


//使用了回溯的Stmt编写方法
Node *Parser::Stmt2(int depth) {
    printf("Stmt \n");
    Node *stmt = new Node(GrammarItem::Stmt, depth);
    Node *child = nullptr;
    if (this->currentWord->word_type == "PRINTFTK") {  // 'printf''('FormatString{','Exp}')'';'  i j
        stmt->addChild(new Node(this->currentWord, stmt, depth + 1));
        this->nextToken();
        // '('
        if(this->currentWord->word_type == "LPARENT") {
            stmt->addChild(new Node(this->currentWord, stmt, depth + 1));
            this->nextToken();
        }
        // FormatString //FormatString 改名为StringConst
        child = this->StringConst(depth + 1);
        child->setParent(stmt);
        stmt->addChild(child);
        // {','Exp}
        while (this->currentWord->word_type == "COMMA") {
            stmt->addChild(new Node(this->currentWord, stmt, depth + 1));
            this->nextToken();
            // Exp
            child = this->Exp(depth + 1);
            child->setParent(stmt);
            stmt->addChild(child);
        }
        // ')' 错误处理
        if(this->currentWord->word_type == "RPARENT") {
            stmt->addChild(new Node(this->currentWord, stmt, depth + 1));
            this->nextToken();
        }else {
            Word * semi =  new Word("RPARENT",")",lexer ->tokenMap[tokenPos-1] .line_num);
            stmt->addChild(new Node(semi, stmt, depth + 1));
            handleErrorJ(lexer ->tokenMap[tokenPos-1] .line_num);
        }
        // ';' 错误处理
        if(this->currentWord->word_type == "SEMICN") {
            stmt->addChild(new Node(this->currentWord, stmt, depth + 1));
            this->nextToken();
        }else {
            Word * semi =  new Word("SEMICN",";",lexer ->tokenMap[tokenPos-1] .line_num);
            stmt->addChild(new Node(semi, stmt, depth + 1));
            handleErrorI(lexer ->tokenMap[tokenPos-1] .line_num);
        }
    } else if (this->currentWord->word_type == "RETURNTK") {  // 'return' [Exp] ';'  i
        stmt->addChild(new Node(this->currentWord, stmt, depth + 1));
        this->nextToken();
        // [Exp]
        if (this->currentWord->word_type != "SEMICN") {
            child = this->Exp(depth + 1);
            child->setParent(stmt);
            stmt->addChild(child);
        }
        // ';' //错误处理
        if(this->currentWord->word_type == "SEMICN") {
            stmt->addChild(new Node(this->currentWord, stmt, depth + 1));
            this->nextToken();
        }else {
            Word * semi =  new Word("SEMICN",";",lexer ->tokenMap[tokenPos-1] .line_num);
            stmt->addChild(new Node(semi, stmt, depth + 1));
            handleErrorI(lexer ->tokenMap[tokenPos-1] .line_num);
        }
    } else if (this->currentWord->word_type == "BREAKTK" ||
               this->currentWord->word_type == "CONTINUETK") {  // 'break' ';' | 'continue' ';' i
        stmt->addChild(new Node(this->currentWord, stmt, depth + 1));
        this->nextToken();
        // ';' 错误处理
        if(this->currentWord->word_type == "SEMICN") {
            stmt->addChild(new Node(this->currentWord, stmt, depth + 1));
            this->nextToken();
        }else {
            Word * semi =  new Word("SEMICN",";",lexer ->tokenMap[tokenPos-1] .line_num);
            stmt->addChild(new Node(semi, stmt, depth + 1));
            handleErrorI(lexer ->tokenMap[tokenPos-1] .line_num);
        }
    } else if(this->currentWord -> word_type == "FORTK") {
        // | 'for' '(' [ForStmt] ';' [Cond] ';' [ForStmt] ')' Stmt
        stmt -> addChild(new Node(this->currentWord, stmt, depth + 1));
        this->nextToken();
        //'('
        if(this->currentWord->word_type == "LPARENT") {
            stmt->addChild(new Node(this->currentWord, stmt, depth + 1));
            this->nextToken();
        }
        //[ForStmt]
        if(this->currentWord->word_type == "IDENFR") {
            child = this->ForStmt(depth + 1);
            child->setParent(stmt);
            stmt->addChild(child);
        }
        //';'
        if(this->currentWord->word_type == "SEMICN") {
            stmt->addChild(new Node(this->currentWord, stmt, depth + 1));
            this->nextToken();
        }
        //[Cond]
        if(this->currentWord->word_type == "IDENFR") {
            child = this->Cond(depth + 1);
            child->setParent(stmt);
            stmt->addChild(child);
        }
        //;
        if(this->currentWord->word_type == "SEMICN") {
            stmt->addChild(new Node(this->currentWord, stmt, depth + 1));
            this->nextToken();
        }
        //[ForStmt]
        //printf("in forstmt %s %s\n",this->currentWord->word_type.c_str(),this->currentWord->word.c_str());
        if(this->currentWord->word_type != "RPARENT") { //2024.10.14修改
            child = this->ForStmt(depth + 1);
            child->setParent(stmt);
            stmt->addChild(child);
        }
        //')' 错误处理
        if(this->currentWord->word_type == "RPARENT") {
            stmt->addChild(new Node(this->currentWord, stmt, depth + 1));
            this->nextToken();
        }else {
            Word * semi =  new Word("RPARENT",")",lexer ->tokenMap[tokenPos-1] .line_num);
            stmt->addChild(new Node(semi, stmt, depth + 1));
            handleErrorJ(lexer ->tokenMap[tokenPos-1] .line_num);
        }
        //Stmt
        child = this->Stmt(depth + 1);
        child->setParent(stmt);
        stmt->addChild(child);
    }else if (this->currentWord->word_type == "IFTK") {  // 'if' '(' Cond ')' Stmt [ 'else' Stmt ]  //j
        stmt->addChild(new Node(this->currentWord, stmt, depth + 1));
        this->nextToken();
        // '('
        if(this->currentWord->word_type == "LPARENT") {
            stmt->addChild(new Node(this->currentWord, stmt, depth + 1));
            this->nextToken();
        }
        // Cond
        child = this->Cond(depth + 1);
        child->setParent(stmt);
        stmt->addChild(child);
        // ')'
        if(this->currentWord->word_type == "RPARENT") {
            stmt->addChild(new Node(this->currentWord, stmt, depth + 1));
            this->nextToken();
        }else {
            Word * semi =  new Word("RPARENT",")",lexer ->tokenMap[tokenPos-1] .line_num);
            stmt->addChild(new Node(semi, stmt, depth + 1));
            handleErrorJ(lexer ->tokenMap[tokenPos-1] .line_num);
        }
        // Stmt
        child = this->Stmt(depth + 1);
        child->setParent(stmt);
        stmt->addChild(child);
        // [ 'else' Stmt ]
        if (this->currentWord->word_type == "ELSETK") {
            stmt->addChild(new Node(this->currentWord, stmt, depth + 1));
            this->nextToken();
            // Stmt
            child = this->Stmt(depth + 1);
            child->setParent(stmt);
            stmt->addChild(child);
        }
    } else if (this->currentWord->word_type == "LBRACE") {  // Block
        child = this->Block(depth + 1);
        child->setParent(stmt);
        stmt->addChild(child);
    }
    //LVal = ...
    else {
        // 检查是Exp或LVal
        int tempTokenPos = tokenPos;
        child = this->LVal(depth + 1);

        //[Exp];
        if(this->currentWord->word_type!="ASSIGN") {
            tokenPos = tempTokenPos;
            this->currentWord = &lexer->tokenMap[tokenPos];
            if (this->currentWord->word_type != "SEMICN") {
                child = this->Exp(depth + 1);
                child->setParent(stmt);
                stmt->addChild(child);
            }
            // ';' 错误处理
            if(this->currentWord->word_type == "SEMICN") {
                stmt->addChild(new Node(this->currentWord, stmt, depth + 1));
                this->nextToken();
            }else {
                Word * semi =  new Word("SEMICN",";",lexer ->tokenMap[tokenPos-1] .line_num);
                stmt->addChild(new Node(semi, stmt, depth + 1));
                handleErrorI(lexer ->tokenMap[tokenPos-1] .line_num);
            }
            return stmt;
        }
        //以LVal加入stmt
        child->setParent(stmt);
        stmt->addChild(child);
        // '='
        if(this->currentWord->word_type == "ASSIGN") {
            stmt->addChild(new Node(this->currentWord, stmt, depth + 1));
            this->nextToken();
        }
        if (this->currentWord->word_type != "GETINTTK" && this->currentWord ->word_type != "GETCHARTK") {  // LVal '=' Exp ';'
            child = this->Exp(depth + 1);
            child->setParent(stmt);
            stmt->addChild(child);
        } else if(this ->currentWord ->word_type == "GETINTTK")     {  // LVal '=' 'getint''('')'';'
            stmt->addChild(new Node(this->currentWord, stmt, depth + 1));
            this->nextToken();
            // '('
            if(this->currentWord->word_type == "LPARENT") {
                stmt->addChild(new Node(this->currentWord, stmt, depth + 1));
                this->nextToken();
            }
            // ')' 错误处理
            if(this->currentWord->word_type == "RPARENT") {
                stmt->addChild(new Node(this->currentWord, stmt, depth + 1));
                this->nextToken();
            }else {
                Word * semi =  new Word("RPARENT",")",lexer ->tokenMap[tokenPos-1] .line_num);
                stmt->addChild(new Node(semi, stmt, depth + 1));
                handleErrorJ(lexer ->tokenMap[tokenPos-1] .line_num);
            }
        }else if(this->currentWord ->word_type == "GETCHARTK") {
            stmt->addChild(new Node(this->currentWord, stmt, depth + 1));
            this->nextToken();
            // '('
            if(this->currentWord->word_type == "LPARENT") {
                stmt->addChild(new Node(this->currentWord, stmt, depth + 1));
                this->nextToken();
            }
            // ')' 错误处理
            if(this->currentWord->word_type == "RPARENT") {
                stmt->addChild(new Node(this->currentWord, stmt, depth + 1));
                this->nextToken();
            }else {
                Word * semi =  new Word("RPARENT",")",lexer ->tokenMap[tokenPos-1] .line_num);
                stmt->addChild(new Node(semi, stmt, depth + 1));
                handleErrorJ(lexer ->tokenMap[tokenPos-1] .line_num);
        }
        }
        // ';' 错误处理
        if(this->currentWord->word_type == "SEMICN") {
            stmt->addChild(new Node(this->currentWord, stmt, depth + 1));
            this->nextToken();
        }else {
            Word * semi =  new Word("SEMICN",";",lexer ->tokenMap[tokenPos-1] .line_num);
            stmt->addChild(new Node(semi, stmt, depth + 1));
            handleErrorI(lexer ->tokenMap[tokenPos-1] .line_num);
        }
    }
    return stmt;
}

//18. ForStmt → LVal '=' Exp
Node *Parser::ForStmt(int depth) {
    Node *forStmt = new Node(GrammarItem::Forstmt,depth);
    Node *child = nullptr;
    //LVal
    child = this->LVal(depth + 1);
    child->setParent(forStmt);
    forStmt->addChild(child);
    //'='
    if(this->currentWord->word_type == "ASSIGN") {
        forStmt->addChild(new Node(this->currentWord, forStmt, depth + 1));
        this->nextToken();
    }
    //Exp
    child = this->Exp(depth + 1);
    child->setParent(forStmt);
    forStmt->addChild(child);
    return forStmt;

}


/*  Exp → AddExp */
//19.Exp → AddExp
Node *Parser::Exp(int depth) {
    printf("Exp %d\n",tokenPos);
    Node *exp = new Node(GrammarItem::Exp, depth);
    Node *child = this->AddExp(depth + 1);
    child->setParent(exp);
    exp->addChild(child);
    return exp;
}


/* Cond → LOrExp */
//20.Cond → LOrExp
Node *Parser::Cond(int depth) {
    printf("Cond %d\n",depth);
    Node *cond = new Node(GrammarItem::Cond, depth);
    Node *child = this->LOrExp(depth + 1);
    child->setParent(cond);
    cond->addChild(child);
    return cond;
}



/* LVal → Ident {'[' Exp ']'} */
//21. LVal → Ident ['[' Exp ']'] // k
Node *Parser::LVal(int depth) {
    printf("LVal %d\n",depth);
    Node *lVal = new Node(GrammarItem::LVal, depth);
    Node *child = this->Ident(depth + 1);  // Ident
    child->setParent(lVal);
    lVal->addChild(child);
    // {'[' Exp ']'}
    while (this->currentWord->word_type == "LBRACK") {
        lVal->addChild(new Node(this->currentWord, lVal, depth + 1));
        this->nextToken();
        // Exp
        child = this->Exp(depth + 1);
        child->setParent(lVal);
        lVal->addChild(child);
        // ']' 错误处理
        if(this->currentWord->word_type == "RBRACK") {
            lVal->addChild(new Node(this->currentWord, lVal, depth + 1));
            this->nextToken();
        }else {
            Word * semi =  new Word("RBREAK","]",lexer ->tokenMap[tokenPos-1] .line_num);
            lVal->addChild(new Node(semi, lVal, depth + 1));
            handleErrorK(lexer ->tokenMap[tokenPos-1] .line_num);
        }
    }
    return lVal;
}



/* PrimaryExp → '(' Exp ')' | LVal | Number */
//22.PrimaryExp → '(' Exp ')' | LVal | Number | Character// j
Node *Parser::PrimaryExp(int depth) {
    Node *primaryExp = new Node(GrammarItem::PrimaryExp, depth);
    Node *child = nullptr;
    if (this->currentWord->word_type == "LPARENT") {  // '(' Exp ')'
        primaryExp->addChild(new Node(this->currentWord, primaryExp, depth + 1));
        this->nextToken();
        // Exp
        child = this->Exp(depth + 1);
        child->setParent(primaryExp);
        primaryExp->addChild(child);
        // ')' 错误处理
        if(this->currentWord->word_type == "RPARENT") {
            primaryExp->addChild(new Node(this->currentWord, primaryExp, depth + 1));
            this->nextToken();
        }else {
            Word * semi =  new Word("RPARENT",")",lexer ->tokenMap[tokenPos-1] .line_num);
            primaryExp->addChild(new Node(semi, primaryExp, depth + 1));
            handleErrorJ(lexer ->tokenMap[tokenPos-1] .line_num);
        }
    } else if (this->currentWord->word_type == "IDENFR") {  // LVal  (LVal → Ident {'[' Exp ']'})
        child = this->LVal(depth + 1);
        child->setParent(primaryExp);
        primaryExp->addChild(child);
    } else if (this->currentWord->word_type == "INTCON"){  // Number
        child = this->Number(depth + 1);
        child->setParent(primaryExp);
        primaryExp->addChild(child);
    }
    else {
        child = this->Character(depth + 1);
        child ->setParent(primaryExp);
        primaryExp ->addChild(child);
    }
    return primaryExp;
}



/* Number → IntConst */
//23. Number → IntConst
Node *Parser::Number(int depth) {
    Node *number = new Node(GrammarItem::Number, depth);
    Node *child = this->IntConst(depth + 1);
    child->setParent(number);
    number->addChild(child);
    return number;
}

Node * Parser:: IntConst(int depth) {
    printf("IntConst %d %s %s\n",this->tokenPos, this->currentWord->word_type.c_str(),this->currentWord->word.c_str());
    Node *intConst = new Node(this->currentWord, depth);
    if(this->currentWord->word_type == "INTCON") {

        this->nextToken();
    }
    return intConst;
}




//24.Character → CharConst
Node *Parser::Character(int depth) {
    Node *character = new Node(GrammarItem::CharacterItem,depth);
    Node *child = this->CharConst(depth+1);
    child->setParent(character);
    character->addChild(child);
    return character;
}

Node * Parser::CharConst(int depth) {
    printf("IntConst %d %s %s\n",this->tokenPos, this->currentWord->word_type.c_str(),this->currentWord->word.c_str());
    Node *charConst = new Node(this->currentWord, depth);
    if(this -> currentWord -> word_type == "CHRCON") {

        this ->nextToken();
    }
    return charConst;
}



/* UnaryExp → PrimaryExp | Ident '(' [FuncRParams] ')' | UnaryOp UnaryExp  */
//25. UnaryExp → PrimaryExp | Ident '(' [FuncRParams] ')' | UnaryOp UnaryExp // j
Node *Parser::UnaryExp(int depth) {
    printf("UnaryExp %d %s %s\n",tokenPos,this->currentWord->word_type.c_str(),this->currentWord->word.c_str());
    Node *unaryExp = new Node(GrammarItem::UnaryExp, depth);
    Node *child = nullptr;
    if (this->currentWord->word_type == "PLUS"
        || this->currentWord->word_type == "MINU"
        || this->currentWord->word_type == "NOT") {  // UnaryOp UnaryExp
        child = this->UnaryOp(depth + 1);
        child->setParent(unaryExp);
        unaryExp->addChild(child);
        child = this->UnaryExp(depth + 1);
        child->setParent(unaryExp);
        unaryExp->addChild(child);
        } else if (this->currentWord->word_type == "IDENFR"
                   && lexer ->tokenMap[tokenPos + 1].word_type == "LPARENT") {  // Ident '(' [FuncRParams] ')'
            child = this->Ident(depth + 1);
            child->setParent(unaryExp);
            unaryExp->addChild(child);
            // '('
            if(this->currentWord->word_type == "LPARENT") {
                unaryExp->addChild(new Node(this->currentWord, unaryExp, depth + 1));
                this->nextToken();
            }
            if (this->currentWord->word_type != "RPARENT") {  // [FuncRParams]
                child = this->FuncRParams(depth + 1);
                child->setParent(unaryExp);
                unaryExp->addChild(child);
            }
            printf("Parser : Check in Unary Error J word type is %s\n",this->currentWord->word_type.c_str());
            // ')' 错误处理
            if(this->currentWord->word_type == "RPARENT") {
                unaryExp->addChild(new Node(this->currentWord, unaryExp, depth + 1));
                this->nextToken();
            }else {
                printf("Error j");
                Word * semi =  new Word("RPARENT",")",lexer ->tokenMap[tokenPos-1] .line_num);
                unaryExp->addChild(new Node(semi, unaryExp, depth + 1));
                handleErrorJ(lexer ->tokenMap[tokenPos-1] .line_num);
            }
                   } else {  // PrimaryExp
                       child = this->PrimaryExp(depth + 1);
                       child->setParent(unaryExp);
                       unaryExp->addChild(child);
                   }
    return unaryExp;
}



/* UnaryOp → '+' | '−' | '!' */
//26.UnaryOp → '+' | '−' | '!' 注：'!'仅出现在条件表达式中
Node *Parser::UnaryOp(int depth) {
    Node *unaryOp = new Node(GrammarItem::UnaryOp, depth);
    if(this->currentWord->word_type == "PLUS"
           || this->currentWord->word_type == "MINU"
           || this->currentWord->word_type == "NOT") {
        unaryOp->addChild(new Node(this->currentWord, unaryOp, depth + 1));
        this->nextToken();
    }
    return unaryOp;
}


/* FuncRParams → Exp { ',' Exp } */
//27. FuncRParams → Exp { ',' Exp }
Node *Parser::FuncRParams(int depth) {
    Node *exp = new Node(GrammarItem::FuncRParams, depth);
    Node *child = nullptr;
    // Exp
    child = this->Exp(depth + 1);
    child->setParent(exp);
    exp->addChild(child);
    // { ',' Exp }
    while (this->currentWord->word_type == "COMMA") {
        exp->addChild(new Node(this->currentWord, exp, depth + 1));
        this->nextToken();
        // Exp
        child = this->Exp(depth + 1);
        child->setParent(exp);
        exp->addChild(child);
    }
    return exp;
}



/* MulExp → UnaryExp | MulExp ('*' | '/' | '%') UnaryExp  FIXME: 左递归 */
/* 改写后： MulExp -> UnaryExp {('*' | '/' | '%') UnaryExp} */
//28.MulExp → UnaryExp | MulExp ('*' | '/' | '%') UnaryExp
Node *Parser::MulExp(int depth) {
    printf("MulExp %d %s %s\n",tokenPos,this->currentWord->word_type.c_str(),this->currentWord->word.c_str());
    Node *current = new Node(GrammarItem::MulExp, depth);
    Node *child = this->UnaryExp(depth + 1);
    current->addChild(child);
    child->setParent(current);
    while (this->currentWord->word_type == "MULT"
           || this->currentWord->word_type == "DIV"
           || this->currentWord->word_type == "MOD") {
        // 在发现右侧还有成分时，先将左侧向上打包一层, FIXME: 这里depth有误
        Node *temp = new Node(GrammarItem::MulExp, depth);
        temp->addChild(current);
        current->setParent(temp);
        current = temp;
        // ('*' | '/' | '%')
        current->addChild(new Node(this->currentWord, current, depth + 1));
        this->nextToken();
        // UnaryExp
        child = this->UnaryExp(depth + 1);
        current->addChild(child);
           }
    // TODO: 递归修改 current 的depth
    return current;
}



/* AddExp → MulExp | AddExp ('+' | '−') MulExp FIXME: 左递归 */
/* 改写后: AddExp → MulExp {('+' | '−') MulExp} */
//29.AddExp → MulExp | AddExp ('+' | '−') MulExp
Node *Parser::AddExp(int depth) {
    printf("AddExp %d %s %s\n",tokenPos,this->currentWord->word_type.c_str(),this->currentWord->word.c_str());
    Node *current = new Node(GrammarItem::AddExp, depth);
    Node *child = this->MulExp(depth + 1);
    current->addChild(child);
    child->setParent(current);
    while (this->currentWord->word_type == "PLUS"
           || this->currentWord->word_type == "MINU") {
        // 在发现右侧还有成分时，先将左侧向上打包一层, FIXME: 这里depth有误
        Node *temp = new Node(GrammarItem::AddExp, depth);
        temp->addChild(current);
        current->setParent(temp);
        current = temp;
        // ('+' | '−')
        current->addChild(new Node(this->currentWord, current, depth + 1));
        this->nextToken();
        // MulExp
        child = this->MulExp(depth + 1);
        current->addChild(child);
           }
    // TODO: 递归修改 current 的depth
    return current;
}




/* RelExp → AddExp | RelExp ('<' | '>' | '<=' | '>=') AddExp FIXME: 左递归 */
/* 改写后： RelExp → AddExp {('<' | '>' | '<=' | '>=') AddExp} */
//30.RelExp → AddExp | RelExp ('<' | '>' | '<=' | '>=') AddExp
Node *Parser::RelExp(int depth) {
    printf("RelExp %d %s %s\n",tokenPos,this->currentWord->word_type.c_str(),this->currentWord->word.c_str());
    Node *current = new Node(GrammarItem::RelExp, depth);
    Node *child = this->AddExp(depth + 1);
    current->addChild(child);
    child->setParent(current);
    while (this->currentWord->word_type == "LEQ"
           || this->currentWord->word_type == "LSS"
           || this->currentWord->word_type == "GEQ"
           || this->currentWord->word_type == "GRE") {
        // 在发现右侧还有成分时，先将左侧向上打包一层, FIXME: 这里depth有误
        Node *temp = new Node(GrammarItem::RelExp, depth);
        temp->addChild(current);
        current->setParent(temp);
        current = temp;
        // ('<' | '>' | '<=' | '>=')
        current->addChild(new Node(this->currentWord, current, depth + 1));
        this->nextToken();
        // AddExp
        child = this->AddExp(depth + 1);
        current->addChild(child);
           }
    // TODO: 递归修改 current 的depth
    return current;
}



/* EqExp → RelExp | EqExp ('==' | '!=') RelExp FIXME: 左递归 */
/* 改写后： EqExp → RelExp {('==' | '!=') RelExp} */
//31.EqExp → RelExp | EqExp ('==' | '!=') RelExp
Node *Parser::EqExp(int depth) {
    Node *current = new Node(GrammarItem::EqExp, depth);
    Node *child = this->RelExp(depth + 1);
    current->addChild(child);
    child->setParent(current);
    while (this->currentWord->word_type == "EQL"
           || this->currentWord->word_type == "NEQ") {
        // 在发现右侧还有成分时，先将左侧向上打包一层, FIXME: 这里depth有误
        Node *temp = new Node(GrammarItem::EqExp, depth);
        temp->addChild(current);
        current->setParent(temp);
        current = temp;
        // ('==' | '!=')
        current->addChild(new Node(this->currentWord, current, depth + 1));
        this->nextToken();
        // RelExp
        child = this->RelExp(depth + 1);
        current->addChild(child);
           }
    // TODO: 递归修改 current 的depth
    return current;
}




/* LAndExp → EqExp | LAndExp '&&' EqExp FIXME: 左递归 */
/* 改写后: LAndExp → EqExp {'&&' EqExp} */
//32. LAndExp → EqExp | LAndExp '&&' EqExp
Node *Parser::LAndExp(int depth) {
    Node *current = new Node(GrammarItem::LAndExp, depth);
    Node *child = this->EqExp(depth + 1);
    current->addChild(child);
    child->setParent(current);
    while (this->currentWord->word_type == "AND") {
        // 在发现右侧还有成分时，先将左侧向上打包一层, FIXME: 这里depth有误
        Node *temp = new Node(GrammarItem::LAndExp, depth);
        temp->addChild(current);
        current->setParent(temp);
        current = temp;
        // '&&'
        current->addChild(new Node(this->currentWord, current, depth + 1));
        this->nextToken();
        // EqExp
        child = this->EqExp(depth + 1);
        current->addChild(child);
    }
    // TODO: 递归修改 current 的depth
    return current;
}



/* LOrExp → LAndExp | LOrExp '||' LAndExp FIXME: 左递归 */
/* 改写后： LOrExp → LAndExp {'||' LAndExp} */
//33.LOrExp → LAndExp | LOrExp '||' LAndExp
Node *Parser::LOrExp(int depth) {
    Node *current = new Node(GrammarItem::LOrExp, depth);
    Node *child = this->LAndExp(depth + 1);
    current->addChild(child);
    child->setParent(current);
    while (this->currentWord->word_type == "OR") {
        // 在发现右侧还有成分时，先将左侧向上打包一层, FIXME: 这里depth有误
        Node *temp = new Node(GrammarItem::LOrExp, depth);
        temp->addChild(current);
        current->setParent(temp);
        current = temp;
        // '||'
        current->addChild(new Node(this->currentWord, current, depth + 1));
        this->nextToken();
        // LAndExp
        child = this->LAndExp(depth + 1);
        current->addChild(child);
    }
    printf("FINISH LOREXP!\n");
    // TODO: 递归修改 current 的depth
    return current;
}





/* ConstExp → AddExp */
//34. ConstExp → AddExp 注：使用的 Ident 必须是常量
Node *Parser::ConstExp(int depth) {
    Node *constExp = new Node(GrammarItem::ConstExp, depth);
    Node *child = this->AddExp(depth + 1);
    child->setParent(constExp);
    constExp->addChild(child);
    return constExp;
}

//
Node *Parser::Ident(int depth) {
    printf("Ident %d %s %s\n",this->tokenPos,this->currentWord->word_type.c_str(),this->currentWord->word.c_str());
    Node *ident = new Node(this->currentWord, depth);
    if(this->currentWord->word_type == "IDENFR") {
        this->nextToken();
    }
    return ident;
}






Node *Parser::StringConst(int depth) {
    printf("StringConst %d\n",depth);
    Node *formatString = new Node(this->currentWord, depth);
    if(this->currentWord->word_type == "STRCON") {
        this->nextToken();
    }
    return formatString;
}


bool Parser::semicn_before_assign() {
    /* 判断接下来先出现 ';'还是 '=' */
    /* 用于Stmt解析中判断是表达式还是赋值 */
    int i = this->tokenPos;
    while (lexer->tokenMap[i].word_type != "SEMICN" &&
           lexer->tokenMap[i].word_type != "ASSIGN"&&lexer->tokenMap[i].line_num==lexer->tokenMap[i+1].line_num)
        ++i;
    if (lexer->tokenMap[i].word_type == "SEMICN"||lexer->tokenMap[i].line_num!=lexer->tokenMap[i+1].line_num) return true;
    return false;
}










/**
else if (this->curToken->symbol == Symbol::WHILETK) {  // 'while' '(' Cond ')' Stmt
    stmt->addChild(new Node(this->curToken, stmt, depth + 1));
    this->nextItem();
    // '('
    assert(this->curToken->symbol == Symbol::LPARENT);
    stmt->addChild(new Node(this->curToken, stmt, depth + 1));
    this->nextItem();
    // Cond
    child = this->parse_Cond(depth + 1);
    child->setParent(stmt);
    stmt->addChild(child);
    // ')'
    if (this->curToken->symbol == Symbol::RPARENT) {
        stmt->addChild(new Node(this->curToken, stmt, depth + 1));
        this->nextItem();
    } else {  // FIXME: j => ErrorType::MissingRPARENT )
        int formerLineNum = this->tokens[tokenPos - 1]->lineNumber;
        stmt->addChild(new ErrorNode(ErrorType::MissingRPARENT, formerLineNum));
    }
    // Stmt
    child = this->parse_Stmt(depth + 1);
    child->setParent(stmt);
    stmt->addChild(child);
}
**/