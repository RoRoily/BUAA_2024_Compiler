#include "../../../handleProcess/include/SymbolTableHandler.h"
#include "../include/SymbolTable.h"

#include <map>
#include <vector>
#include <string>
#include <iostream>
#include "../../tree/include/Node.h"
#include "../../variableType/CharFunc.h"
#include "../../variableType/IntFunc.h"
#include "../../variableType/VoidFunc.h"
#include "../../variableType/Char.h"
#include "../../variableType/Int.h"
#include "../../variableType/IntArray.h"
#include "../../variableType/CharArray.h"
#include "../../variableType/ConstChar.h"
#include "../../variableType/ConstCharArray.h"
#include "../../variableType/ConstInt.h"
#include "../../variableType/ConstIntArray.h"
#include "../include/ConstValue.h"
#include "../../variableType/FuncParam.h"
#include "../../../include/ForBlock.h"

#define IS_GLOBAL ((currentTable->isRoot))



extern std::map<int, std::string> errorLog;
static ForBlock *currentForBlock = new ForBlock;

static bool createSymbolTableBeforeEnterBlock = false;

static int inWhile = 0;
enum class Func {
    IntFunc,
    CharFunc,
    VoidFunc,
    MainFunc,
    NotFunc
};
static bool checkingFuncRParams = false;

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
std::map<QuadItemType, std::string> quadItemTypeMap4 = {
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
static Func currentFunction = Func::NotFunc;
static bool receiveReturn = false;
static int funcEndLineNum = 0;   // 结尾分号的行号

SymbolTableHandler::SymbolTableHandler(Node *root) : root(root) {
    this->currentTable = new SymbolTable(nullptr, true,this->count++);
    this->symbolTableList.push_back(currentTable);
    this->icGenerator = ICGenerator::getInstance();
    if (this->root == nullptr) {
        printf("Error: root is null\n");
        return;
    }else {printf("root of symbolTable handler is not null\n");}
}

//开始构建符号表
void SymbolTableHandler::buildTable() {
    printf("SymbolTable:buildTable\n");
    this->CompUnit(this->root);
}


// CompUnit → {Decl} {FuncDef} MainFuncDef
void SymbolTableHandler::CompUnit(Node *node) {
    printf("Symbol: CompUnit \n");
    for (Node *child: (node->getAllChildren())) {
        if (child->equals(GrammarItem::Decl)) this->Decl(child);
        else if (child->equals(GrammarItem::FuncDef)) this->FuncDef(child);
        else this->MainFuncDef(child);
    }
}

// Decl → ConstDecl | VarDecl
void SymbolTableHandler::Decl(Node *node) {
    printf("Symbol: Decl \n");
    Node *child = node->getFirstChild();
    if (child->equals(GrammarItem::ConstDecl)) this->ConstDecl(child);
    else this->VarDecl(child);
}

// FuncDef → FuncType Ident '(' [FuncFParams] ')' Block // b g j
void SymbolTableHandler::FuncDef(Node *node) {
    printf("Symbol: FuncDef\n");
    // 在Block前写到符号表
    Node *funcType = node->getChildAt(0)->getChildAt(0);//函数类型
    Node *funcName = node->getChildAt(1);//函数名ident
    SymbolTableEntry *funcEntry;//创建符号表条目

    //重复定义
    if (currentTable->nameExistedInCurrentTable(funcName)) {
        //FIXME: handle error b
        this -> handleIdentRedefined(funcName-> token->line_num);
        return;
    }

    //创建新的符号表
    //parent为当前符号表
    auto *funcTable = new SymbolTable(currentTable, false,this->count++);
    this->symbolTableList.push_back(funcTable);
    currentTable->addChildTable(funcTable);

    //VoidFunc
    if (funcType->token->word_type == "VOIDTK") {
        currentFunction = Func::VoidFunc;
        auto *voidFunc = new VoidFunc();
        funcEntry = new SymbolTableEntry(funcName, voidFunc, funcName->token->line_num );
    }
    //IntFunc
    else if(funcType ->token->word_type == "CHARTK"){
        currentFunction = Func::CharFunc;
        auto *charFunc = new CharFunc();
        funcEntry = new SymbolTableEntry(funcName, charFunc, funcName->token->line_num );
    }
    //CharFunc
    else {
        currentFunction = Func::IntFunc;
        auto * intFunc = new IntFunc();
        funcEntry = new SymbolTableEntry(funcName,intFunc,funcName->token->line_num);
    }
    //向符号表添加表项
    currentTable->addEntry(*funcEntry->getName(), funcEntry);


    //进入下一层符号表,参数属于其中
    currentTable = funcTable;
    //FuncFParam
    if (node->getChildAt(3)->equals(GrammarItem::FuncFParams)) {  // 函数带有参数
        this->FuncFParams(node->getChildAt(3), funcEntry);
        printf("go back from FuncFParams\n");
    }
    QuadItemFunc * quadItemFunc = icGenerator->translate_FuncDef(funcEntry,currentTable);
    printf("finish translate_FuncDef\n");
    //Block
    createSymbolTableBeforeEnterBlock = true;
    receiveReturn = false; //是否有返回
    icGenerator->currentFunc = quadItemFunc;

    this->Block(node->getLastChild(), true);
    printf("Block\n");
    icGenerator ->currentFunc = nullptr;

    //检查返回值
    //intFunc
    //TODO: 错误g
    if (currentFunction == Func::IntFunc && (!receiveReturn)) {
        //FIXME : handle error g
        this->handleReturnMissing(funcEndLineNum);
    }
    //charFunc
    if (currentFunction == Func::CharFunc && (!receiveReturn)) {
        //FIXME : handle error g
        this->handleReturnMissing(funcEndLineNum);
    }

    //退出函数，恢复符号表
    currentTable = currentTable->parent;
    if(currentTable->isRoot) {
        printf("success go back from Block now in root\n");
    }
    receiveReturn = false;
    currentFunction = Func::NotFunc;
}


// ConstDecl → 'const' BType ConstDef { ',' ConstDef } ';'
// FIXME: i => ErrorType::MissingSEMICN
void SymbolTableHandler::ConstDecl(Node *node) {
    printf("Symbol: ConstDecl\n");
    // 'const' BType
    //略
    //ConstDef {',' ConstDef}
    for (auto i = 2; i < node->getAllChildren().size() - 1; i += 2) {
        this->ConstDef(node->getChildAt(i));
    }
    Node *lastNode = node->getLastChild();
    // TODO: 检查是否存在错误节点
    // 这里删掉了
}

// VarDecl → BType VarDef { ',' VarDef } ';' // i
// FIXME: i => ErrorType::MissingSEMICN
void SymbolTableHandler::VarDecl(Node *node) {
    printf("Symbol: VarDecl \n");
    for (int i = 1; i < node->getAllChildren().size() - 1; i += 2) {
        this->VarDef(node->getChildAt(i));
    }if(node->getChildNum()%2!=1) {
        printf("Error : Missing semicn in VarDecl");
        this->VarDef(node->getLastChild());
    }
}

// MainFuncDef → 'int' 'main' '(' ')' Block // g j
// FIXME: g => ErrorType::ReturnMissing
// FIXME: j => ErrorType::MissingRPARENT
void SymbolTableHandler::MainFuncDef(Node *node) {
    printf("Symbol: MainFuncDef \n");
    auto *mainFuncTable = new SymbolTable(currentTable, false,this->count++);
    this->symbolTableList.push_back(mainFuncTable);
    currentTable->addChildTable(mainFuncTable);
    currentTable = mainFuncTable;
    createSymbolTableBeforeEnterBlock = true;
    receiveReturn = false;
    currentFunction = Func::MainFunc;

    icGenerator->translate_MainFunc();

    this->Block(node->getLastChild(), true);
    if (!receiveReturn) {
        //FIXME : handle error g
        this ->handleReturnMissing(funcEndLineNum);
    }
    receiveReturn = false;
    currentFunction = Func::NotFunc;
    currentTable = currentTable->parent;
}

// FIXME:新增数组的处理
// VarDef → Ident [ '[' ConstExp ']' ]
// VarDef -> Ident [ '[' ConstExp ']' ] '=' InitVal
void SymbolTableHandler::VarDef(Node *node) {
    printf("Symbol: VarDef \n");
    bool hasError = false;
    Node *ident = node->getFirstChild();
    if (currentTable->nameExistedInCurrentTable(ident)) {
        // FIXME: handle error b
        hasError = true;
        this-> handleIdentRedefined(ident->token->line_num);
    } else {
        int i = 1, length = node->getAllChildren().size();
        std::vector<int> arrayDimensions;  // size() = 0 || 1 || 2
        while (i <= length - 2 && node->getChildAt(i)->token->word_type=="LBRACK") {
            arrayDimensions.push_back(this->ConstExp(node->getChildAt(i + 1), false));
            i += 3;
        }
        // SymbolTableEntry(Node *node, SymbolTableEntryType type, unsigned int defLineNum);
        //InitVal
        //在此构建属性表
        Word *identWord = ident->token;
        int tokenPos = ident->token->tokenPos;
        QuadItem * quadItem; //还未初始化
        bool varIsChar = false;
        bool varIsCharArray = false;
        if (arrayDimensions.empty()) {
            printf("arrayDimensions.empty\n");
            if (IS_GLOBAL) { //// 全局变量初始化，要求出初始值
                //char
                if(this->CHARTK_at_BType(identWord->tokenPos)) {
                    varIsChar = true;
                    quadItem = new QuadItemImc();
                }
                //int
                else quadItem = new QuadItemImm();
            }
            else {
                //char
                if(this->CHARTK_at_BType(identWord->tokenPos)) {
                    varIsChar = true;
                    quadItem = new QuadItemChar(false);
                }
                //int
                else quadItem = new QuadItemInt(false);
            }
        } else {  // 一维数组或二维数组
            //charArray
            if(this->CHARTK_at_BType(identWord->tokenPos)) {
                varIsCharArray = true;
                quadItem = new QuadItemCharArray(IS_GLOBAL);
            }
            //intArray
            else quadItem = new QuadItemIntArray(IS_GLOBAL);

        }
        bool hasInitVal = false;
        if (node->getLastChild()->equals(GrammarItem::InitVal)) {
            hasInitVal = true;
            int dimension;
            if(arrayDimensions.empty())dimension = 0;
            else dimension = arrayDimensions[0];
            this->InitVal(node->getLastChild(),dimension, quadItem,tokenPos);
        }
        if (hasError) return;
        if (IS_GLOBAL && (!hasInitVal)) {
            printf("IS_GLOBAL && !hasInitVal\n");
            // 全局变量但没有显示初始化，自动初始化为0
            if (arrayDimensions.empty()) {
                if(varIsChar) dynamic_cast<QuadItemImc *>(quadItem)->value = '\0';
                else dynamic_cast<QuadItemImm *>(quadItem)->value = 0;
            }
            else {
                int len = arrayDimensions[0];
                if(!varIsCharArray) {
                    dynamic_cast<QuadItemIntArray *>(quadItem)->value = new int[len];
                    for (int j = 0; j < len; ++j) dynamic_cast<QuadItemIntArray *>(quadItem)->value[j] = 0;
                }else {
                    dynamic_cast<QuadItemCharArray *>(quadItem)->value = new char[len];
                    for (int j = 0; j < len; ++j) dynamic_cast<QuadItemCharArray *>(quadItem)->value[j] = '\0';
                }
            }
        }
        switch (arrayDimensions.size()) {
            //普通变量
            case 0: {
                //FIXME ：判断是字符还是整数
                //Char
                if(this->CHARTK_at_BType(identWord->tokenPos)) {
                    auto * var = new Char();
                    auto *varEntry = new SymbolTableEntry(ident, var, ident->token->line_num, false);
                    currentTable->addEntry(*varEntry->getName(),varEntry);
                    icGenerator->translate_VarDef(quadItem, IS_GLOBAL, varEntry, hasInitVal, currentTable);
                    return;
                }
                //Int
                auto *var = new Int();
                auto *varEntry = new SymbolTableEntry(ident, var, ident->token->line_num, false);
                currentTable->addEntry(*varEntry->getName(), varEntry);
                icGenerator->translate_VarDef(quadItem, IS_GLOBAL, varEntry, hasInitVal, currentTable);
                return;
            }
            //一维变量 intArray CharArray
            case 1: {
                //CharArray
                if(this->CHARTK_at_BType(identWord->tokenPos)) {
                    //auto *array = new CharArray(arrayDimensions[0]);
                    //auto *arrayEntry = new SymbolTableEntry(ident, array, ident->token->line_num, false);
                    auto *arrayEntry = new SymbolTableEntry(ident, SymbolTableEntryType::CharArray, ident->token->line_num, false);
                    currentTable->addEntry(*arrayEntry->getName(), arrayEntry);
                    icGenerator->translate_ArrayDef(quadItem, IS_GLOBAL, arrayEntry,
                                                hasInitVal, arrayDimensions[0], currentTable);
                    return;
                }
                //IntArray
                //auto *array = new IntArray(arrayDimensions[0]);
                auto *arrayEntry = new SymbolTableEntry(ident,SymbolTableEntryType::IntArray, ident->token->line_num, false);
                currentTable->addEntry(*arrayEntry->getName(), arrayEntry);
                icGenerator->translate_ArrayDef(quadItem, IS_GLOBAL, arrayEntry,
                            hasInitVal, arrayDimensions[0], currentTable);
                return;
            }
            default: {
                printf("Error : High Dimension Array");
            }
                ;
            //二维数组
            // default:
            //     auto *array2 = new Array2(arrayDimensions[0], arrayDimensions[1]);
            //     auto *array2Entry = new SymbolTableEntry(ident, array2, ident->token->line_num, false);
            //     currentTable->addEntry(*array2Entry->getName(), array2Entry);
            //     return;
        }
    }
}

// ConstDef → Ident { '[' ConstExp ']' } '=' ConstInitVal
// FIXME: b => ErrorType::IdentRedefined
// FIXME: k => ErrorType::MissingRBRACK
void SymbolTableHandler::ConstDef(Node *node) {
    printf("Symbol: ConstDef \n");
    Node *ident = node->getFirstChild();
    int tokenPos = ident->token->tokenPos;
    if (currentTable->nameExistedInCurrentTable(ident)) {
        //FIXME: handle error b
        this ->handleIdentRedefined(ident->token->line_num);
    } else {
        //处理数组维度
        int i = 1, length = node->getAllChildren().size();
        std::vector<int> arrayDimensions;  // size() = 0 || 1
        //Array
        if(node->getChildAt(1)->token->word_type=="LBRACK") {
            //ConstExp
            arrayDimensions.push_back(this->ConstExp(node->getChildAt(i + 1), false));
        }
        int dimension;
        if(arrayDimensions.size()==0)dimension=0;
        else dimension = arrayDimensions[0];
        //ConstInitVal
        //获取常值注入到constValue
        ConstValue *constValue = this->ConstInitVal(
                node->getLastChild(), dimension,tokenPos);
        Word * identWord = ident -> token;
        int caseDimension ;
        if(dimension == 0)caseDimension = 0;
        else caseDimension = 1;
        // SymbolTableEntry(Node *node, SymbolTableEntryType type, unsigned int defLineNum);
        switch (caseDimension) {
            case 0: {
                //ConstChar
                if(this->CHARTK_at_BType(identWord->tokenPos)) {
                    auto * varConst = new ConstChar(constValue->charVar);
                    auto *varEntry = new SymbolTableEntry(ident, varConst, ident->token->line_num, false);
                    currentTable->addEntry(*varEntry->getName(),varEntry);
                    //生成中间代码
                    icGenerator->translate_ConstVarDef(IS_GLOBAL, varEntry, currentTable);
                    return;
                }
                //ConstInt
                auto *varConst = new ConstInt(constValue->intVar);
                auto *varEntry = new SymbolTableEntry(ident, varConst, ident->token->line_num, false);
                currentTable->addEntry(*varEntry->getName(), varEntry);
                icGenerator->translate_ConstVarDef(IS_GLOBAL, varEntry, currentTable);
                return;
                // auto *varConst = new VarConst(constValue->value.var);
                // auto *varConstEntry = new SymbolTableEntry(
                //         ident, varConst, ident->token->line_num, false);
                // currentTable->addEntry(*varConstEntry->getName(), varConstEntry);
                // return;
            }
            case 1: {
                int d = constValue->dimension;
                //CharArray
                if(this->CHARTK_at_BType(identWord->tokenPos)) {
                    if(constValue->charArray==nullptr)constValue->charArray=new std::string();
                    if(!STRCON_befor_SEMICN(tokenPos)) {
                        for(int i = 0;i < d;i++) {
                            *constValue->charArray+=constValue->intArray[i];
                        }
                    }
                    auto *charArrayConst =new ConstCharArray(d,constValue->charArray);
                    auto *charArrayConstEntry = new SymbolTableEntry(
                        ident, charArrayConst, ident->token->line_num, false);
                    currentTable->addEntry(*charArrayConstEntry->getName(), charArrayConstEntry);
                    icGenerator->translate_ConstArrayDef(IS_GLOBAL, charArrayConstEntry,
                                                       d, currentTable);
                    return;
                }
                //IntArray
                auto *intArrayConst =new ConstIntArray(d,constValue->intArray);
                auto *intArrayConstEntry = new SymbolTableEntry(
                    ident, intArrayConst, ident->token->line_num, false);
                currentTable->addEntry(*intArrayConstEntry->getName(), intArrayConstEntry);
                icGenerator->translate_ConstArrayDef(IS_GLOBAL, intArrayConstEntry,
                                                       d, currentTable);
                return;
                // auto *array1Const = new Array1Const(d1, constValue->value.array1);
                // auto *array1ConstEntry = new SymbolTableEntry(
                //         ident, array1Const, ident->token->line_num, false);
                // currentTable->addEntry(*array1ConstEntry->getName(), array1ConstEntry);
                // return;
            }
            //FIXME: 2-dimension array
            // default:
            //     int d1 = constValue->d1;
            //     int d2 = constValue->d2;
            //     auto *array2Const = new Array2Const(d1, d2, constValue->value.array2);
            //     auto *array2ConstEntry = new SymbolTableEntry(
            //             ident, array2Const, ident->token->line_num, false);
            //     currentTable->addEntry(*array2ConstEntry->getName(), array2ConstEntry);
            //     return;
        }
    }
}

// ConstExp → AddExp
int SymbolTableHandler::ConstExp(Node *node,bool isChar) {  // 返回值为常量表达式的值
    printf("Symbol: ConstExp node \n");
    int constValue;
    char constValueInChar;
    //
    if(isChar) {
        auto *quadItem = new QuadItemChar(IS_GLOBAL);
        this->AddExp(node->getFirstChild(), true, &constValue,quadItem, isChar);
    }else {
        auto *quadItem = new QuadItemInt(IS_GLOBAL);
        this->AddExp(node->getFirstChild(), true, &constValue,quadItem, isChar);
    }
    return constValue;
}

// AddExp → MulExp | AddExp ('+' | '−') MulExp
SymbolTableEntry *SymbolTableHandler::AddExp(Node *node, bool fromConstExp, int *constExpValue,QuadItem* quadItem,bool isChar) {
    printf("Symbol: AddExp ");
    SymbolTableEntry *entry1 = nullptr, *entry2 = nullptr;
    //ConstExp中使用，用于计算获得的值
    int value1 = 0,value2 = 0;
    //MulExp
    if (node->getAllChildren().size() == 1) {
        SymbolTableEntry*ret = this->MulExp(node->getFirstChild(), fromConstExp, constExpValue,quadItem);
        return ret;
    }
    //FIXME : MulExp中是否可以直接为整数?
    auto *var1 = new QuadItemInt(IS_GLOBAL);
    auto *var2 = new QuadItemInt(IS_GLOBAL);
    entry1 = this->AddExp(node->getFirstChild(), fromConstExp, &value1, var1, isChar);
    entry2 = this->MulExp(node->getLastChild(), fromConstExp, &value2, var2);
    if (fromConstExp) {  // 必须为常量
        if (node->getChildAt(1)->token->word_type=="PLUS") {
            *constExpValue = value1 + value2;
        } else {
            *constExpValue = value1 - value2;
        }
        return nullptr;
    }
        QuadItemInt *ret = ((QuadItemInt *) quadItem);
        QuadEntryType op = icGenerator->symbolName2binaryOp(node->getChildAt(1)->token->word_type);
        //从条目中生成entry
        icGenerator->translate_BinaryOperator(op, ret, var1, var2);


    if (entry1 == nullptr || entry2 == nullptr) return nullptr;
    return entry1;  // TODO 生成代码时要计算 entry1 +/- entry2
}


// InitVal → Exp | '{' [ Exp { ',' Exp } ] '}' | StringConst
//quadItem在Entry中作为operator2使用
void SymbolTableHandler::InitVal(Node *node, int d, QuadItem *quadItem,int tokenPos) {
    printf("SymbolTabelHandler : InitVal\n");
    auto *constValue = new ConstValue();
    constValue->dimension = d;
    //StringConst
    if(this->STRCON_befor_SEMICN(tokenPos)) {
        int tempp = get_StringConst(tokenPos);
        if(tempp!=-1) {
            constValue->charArray = new std::string();  // 先分配内存
            *constValue->charArray = lexer->tokenMap[tempp].word;
            constValue->charArray->pop_back();
            *constValue->charArray+='\0';
            auto *quadItemArray = dynamic_cast<QuadItemCharArray *>(quadItem);
            const int length = d;
            quadItemArray->length = length;
            quadItemArray->value=*constValue->charArray;
            // for (int i = 1, j = 0; i < childNum - 1; i += 2) {
            //     Node *exp = node->getChildAt(i)->getFirstChild();
            //     this->Exp(exp, true, &temp, nullptr);
            //     quadItemArray->value[j++] = temp;
            // }
        }else printf("not find StringConst\n");
        //exit(0)
    }
    int temp = 0;
    int caseD = 1;
    if(d==0)caseD = 0;
    // 全局变量，直接求出InitVal，这期间不用生成中间代码
    if (IS_GLOBAL) {
        switch (caseD) {
            case 0: {
                this->Exp(node->getFirstChild(), true, &temp, nullptr);
                ((QuadItemImm *)quadItem)->value = temp;
                return;
            }
            case 1: {
                if(this->STRCON_befor_SEMICN(tokenPos)) {
                    int tempp = get_StringConst(tokenPos);
                    if(tempp!=-1) {
                        printf("Symbol: ConstInitVal handle StringConst %s\n",lexer->tokenMap[tempp].word.c_str());
                        constValue->charArray = new std::string();  // 先分配内存
                        int stringLen = lexer->tokenMap[tempp].word.length();
                        for(int i = 1;i < stringLen-1;i++) {
                            char c = lexer->tokenMap[tempp].word.at(i);
                            if(c=='\\') {
                                if(lexer->tokenMap[tempp].word.at(i+1)=='n'){c='\n';i++;}
                                else if(lexer->tokenMap[tempp].word.at(i+1)=='\\'){c='\\';i++;}
                                else if(lexer->tokenMap[tempp].word.at(i+1)=='a'){c='\a';i++;}
                                else if(lexer->tokenMap[tempp].word.at(i+1)=='b'){c='\b';i++;}
                                else if(lexer->tokenMap[tempp].word.at(i+1)=='t'){c='\t';i++;}
                                else if(lexer->tokenMap[tempp].word.at(i+1)=='0'){c='\0';i++;}
                                else if(lexer->tokenMap[tempp].word.at(i+1)=='v'){c='\v';i++;}
                                else if(lexer->tokenMap[tempp].word.at(i+1)=='f'){c='\f';i++;}
                                else if(lexer->tokenMap[tempp].word.at(i+1)=='\"'){c='\"';i++;}
                                else if(lexer->tokenMap[tempp].word.at(i+1)=='\''){c='\'';i++;}
                            }
                            constValue->charArray->push_back(c);
                        }
                        int arrayLen =constValue->charArray->length();
                        for(int count=0;count<d-arrayLen;count++)
                        constValue->charArray+='\0';
                        auto *quadItemArray = dynamic_cast<QuadItemCharArray *>(quadItem);
                        const int length = d;
                        quadItemArray->length = length;
                        quadItemArray->value=*constValue->charArray;
                    }else printf("not find StringConst\n");
                    //exit(0)
                }
                //IntArray
                else if(!this->CHARTK_at_BType(tokenPos)) {
                    int childNum = node->getAllChildren().size();
                    auto *quadItemArray = dynamic_cast<QuadItemIntArray *>(quadItem);
                    const int length = d;
                    quadItemArray->value = new int[length];
                    quadItemArray->length = length;
                    for (int i = 1, j = 0; i < childNum - 1; i += 2) {
                        Node *exp = node->getChildAt(i);//->getFirstChild();
                        this->Exp(exp, true, &temp, nullptr);
                        quadItemArray->value[j++] = temp;
                    }
                }
                //CharArray
                else {
                    int childNum = node->getAllChildren().size();
                    auto *quadItemArray = dynamic_cast<QuadItemCharArray *>(quadItem);
                    const int length = d;
                    quadItemArray->value = std::string(d, '\0');
                    quadItemArray->length = length;
                    for (int i = 1, j = 0; i < childNum - 1; i += 2) {
                        Node *exp = node->getChildAt(i);//->getFirstChild();
                        this->Exp(exp, true, &temp, nullptr);
                        quadItemArray->value[j++] = (char)temp;
                    }
                }
                return;
            }
            // default: {
            //     auto *quadItemArray = (QuadItemIntArray *) quadItem;
            //     const int d1 = (node->getAllChildren().size() - 1) / 2;
            //     const int d2 = (node->getFirstChild()->getAllChildren().size() - 1) / 2;
            //     quadItemArray->value = new int[d1 * d2];
            //     quadItemArray->length = d1 * d2;
            //     int k = 0;
            //     for (int i = 1; i < node->getAllChildren().size() - 1; i += 2) {
            //         for (int j = 1; j < node->getFirstChild()->getAllChildren().size() - 1; j += 2) {
            //             Node *exp = node->getChildAt(i)->getChildAt(j)->getFirstChild();
            //             this->Exp(exp, true, &temp, nullptr);
            //             quadItemArray->value[k++] = temp;
            //         }
            //     }
            //     return;
            // }
        }
    } else {  // 非全局变量初始值，不能求出常数，需要生成相应的中间代码(退出函数，回到varDef后生成)
        printf("NON_GLOBAL EXP\n");
        switch (caseD) {
            case 0: {
                this->Exp(node->getFirstChild(), false, &temp, quadItem);
                return;
            }
            //一维数组
            case 1: {
                if(this->STRCON_befor_SEMICN(tokenPos)) {
                    int tempp = get_StringConst(tokenPos);
                    if(tempp!=-1) {
                        constValue->charArray = new std::string();  // 先分配内存
                        *constValue->charArray = lexer->tokenMap[tempp].word;
                        constValue->charArray->pop_back();
                        *constValue->charArray+='\0';
                        auto *quadItemArray = dynamic_cast<QuadItemCharArray *>(quadItem);
                        const int length = d;
                        quadItemArray->length = length;
                        quadItemArray->value=*constValue->charArray;
                        std::vector<QuadItemChar *> * arrayItems = quadItemArray->itemsToInitArray;
                        int stringLen = constValue->charArray->size();
                        for(int i = 1;i < stringLen-1;i++) {
                            char c = constValue->charArray->at(i);
                            if(c=='\\') {
                                if(lexer->tokenMap[tempp].word.at(i+1)=='n'){c='\n';i++;}
                                else if(lexer->tokenMap[tempp].word.at(i+1)=='\\'){c='\\';i++;}
                                else if(lexer->tokenMap[tempp].word.at(i+1)=='a'){c='\a';i++;}
                                else if(lexer->tokenMap[tempp].word.at(i+1)=='b'){c='\b';i++;}
                                else if(lexer->tokenMap[tempp].word.at(i+1)=='t'){c='\t';i++;}
                                else if(lexer->tokenMap[tempp].word.at(i+1)=='0'){c='\0';i++;}
                                else if(lexer->tokenMap[tempp].word.at(i+1)=='v'){c='\v';i++;}
                                else if(lexer->tokenMap[tempp].word.at(i+1)=='f'){c='\f';i++;}
                                else if(lexer->tokenMap[tempp].word.at(i+1)=='\"'){c='\"';i++;}
                                else if(lexer->tokenMap[tempp].word.at(i+1)=='\''){c='\'';i++;}
                            }
                            char * cc = &c;
                            auto *itemVar = new QuadItemChar(IS_GLOBAL);
                            ((QuadItemChar *) itemVar)->value = c;
                            itemVar->isConst = true;
                            arrayItems->push_back(itemVar);
                        }
                        unsigned long long arrayLen = arrayItems->size();
                        for(int count=0;count<d-arrayLen;count++) {
                            auto *itemVar = new QuadItemChar(IS_GLOBAL);
                            ((QuadItemChar *) itemVar)->value = '\0';
                            itemVar->isConst =true;
                            arrayItems->push_back(itemVar);
                        }
                        //DEBUG
                        //DEBUG
                        // for (int i = 1, j = 0; i < childNum - 1; i += 2) {
                        //     Node *exp = node->getChildAt(i)->getFirstChild();
                        //     this->Exp(exp, true, &temp, nullptr);
                        //     quadItemArray->value[j++] = temp;
                        // }
                    }else printf("not find StringConst\n");
                    //exit(0)
                    return;
                }
                //charArray
                if(CHARTK_at_BType(tokenPos)) {
                    int childNum = node->getAllChildren().size();
                    auto *quadItemArray = dynamic_cast<QuadItemCharArray *>(quadItem);
                    std::vector<QuadItemChar *> * arrayItems = quadItemArray->itemsToInitArray;
                    int count = 0;
                    for (int i = 1; i < childNum - 1; i += 2) {
                        count++;
                        Node *exp = node->getChildAt(i);//->getFirstChild();
                        auto *itemVar = new QuadItemChar(IS_GLOBAL);
                        this->Exp(exp, false, &temp, itemVar);
                        arrayItems->push_back(itemVar);
                    }
                    for(int i =0; i<d-count;i++) {
                        auto *itemVar = new QuadItemChar(IS_GLOBAL);
                        itemVar->isConst =true;
                        itemVar->value = '\0';
                        arrayItems->push_back(itemVar);
                    }
                    // for (int i = 1, j = 0; i < childNum - 1; i += 2) {
                    //     Node *exp = node->getChildAt(i);//->getFirstChild();
                    //     this->Exp(exp, true, &temp, nullptr);
                    //     quadItemArray->value[j++] = (char)temp;
                    // }
                    return;
                }
                auto *quadItemArray = dynamic_cast<QuadItemIntArray *>(quadItem);
                std::vector<QuadItemInt *> *arrayItems = quadItemArray->itemsToInitArray;
                int childNum = node->getAllChildren().size();
                int count = 0;
                for (int i = 1; i < childNum - 1; i += 2) {
                    count++;
                    Node *exp = node->getChildAt(i);//->getFirstChild();
                    auto *itemVar = new QuadItemInt(IS_GLOBAL);
                    this->Exp(exp, false, &temp, itemVar);
                    arrayItems->push_back(itemVar);
                }
                for(int i =0; i<d-count;i++) {
                    auto *itemVar = new QuadItemInt(IS_GLOBAL);
                    itemVar->isConst =true;
                    itemVar->value = 0;
                    arrayItems->push_back(itemVar);
                }
                //exit(0);
                return;
            }
            // default: {
            //     auto *quadItemArray = (QuadItemArray *) quadItem;
            //     std::vector<QuadItemInt *> *arrayItems = quadItemArray->itemsToInitArray;
            //
            //     for (int i = 1; i < node->getAllChildren().size() - 1; i += 2) {
            //         for (int j = 1; j < node->getFirstChild()->getAllChildren().size() - 1; j += 2) {
            //             Node *exp = node->getChildAt(i)->getChildAt(j)->getFirstChild();
            //             auto *itemVar = new QuadItemInt(IS_GLOBAL);
            //             this->Exp(exp, false, &temp, itemVar);
            //             arrayItems->push_back(itemVar);
            //         }
            //     }
            //     return;
            // }
        }
    }
}

// ConstInitVal → ConstExp | '{' [ ConstInitVal { ',' ConstInitVal } ] '}'
//1.常量初值 ConstInitVal → ConstExp | '{' [ ConstExp { ',' ConstExp } ] '}' | StringConst // 1.常表达式初值 2.一维数组初值
//d:维数，d=0时为简单变量
//TODO: ConstValue?
ConstValue *SymbolTableHandler::ConstInitVal(Node *node, int d,int tokenPos)  {
    printf("Symbol: ConstInitVal \n");
    auto *constValue = new ConstValue();
    constValue->dimension = d;
    //StringConst
    //FIXME: 这里StringConst的处理是否正确
    if(this->STRCON_befor_SEMICN(tokenPos)) {
        int temp = get_StringConst(tokenPos);
        if(temp!=-1) {
            constValue->charArray = new std::string();  // 先分配内存
            *constValue->charArray = lexer->tokenMap[temp].word;
        }else printf("not find StringConst\n");
        return constValue;
    }
    int caseD ;
    if(d==0)caseD = 0;
    else caseD = 1;
    switch (caseD) {
        case 0: {
            if(this->CHARTK_at_BType(tokenPos)) {
                constValue->charVar = this->ConstExp(node->getFirstChild(), false);
                return constValue;
            }
            //FIXME: char类型的常量?
            constValue->intVar = this->ConstExp(node->getFirstChild(), false);
            return constValue;
        }
        //数组
        case 1: {  // '{' [ ConstExp { ',' ConstExp } ] '}'
            //charArray
            if(this->CHARTK_at_BType(tokenPos)) {
                //const int d1 = (node->getAllChildren().size() - 1) / 2;
                int *initArray = new int[d];
                for (int i = 1, j = 0; i < node->getAllChildren().size() - 1; i += 2) {
                    Node *constExp = node->getChildAt(i);//->getFirstChild();
                    initArray[j++] = (this->ConstExp(constExp, false));
                }

                // constValue->charArray = new std::string();  // 先分配内存
                // for (int i = 0; i < d; i++) {
                //     *constValue->charArray += (char)(initArray[i]);  // 使用空格分隔数字
                //     printf("%s\n",constValue->charArray->c_str());
                // }
                constValue->intArray = initArray;
                constValue->intVar = 203771777;
                constValue->dimension = d;
                return constValue;

            }else {
                //const int d1 = (node->getAllChildren().size() - 1) / 2;
                int *initArray = new int[d];
                for (int i = 1, j = 0; i < node->getAllChildren().size() - 1; i += 2) {
                    Node *constExp = node->getChildAt(i);//->getFirstChild();
                    initArray[j++] = (this->ConstExp(constExp, false));
                }
                for (int i = 0; i < d; i++) {
                }
                constValue->intArray = initArray;
                constValue->intVar = 999999999;
                constValue->dimension = d;
                return constValue;
            }
        }
        default: ;
        //TODO: handle 2-dimension array
//         default: // ConstInitVal → '{' ConstInitVal { ',' ConstInitVal }  '}'
//             const int d1 = (node->getAllChildren().size() - 1) / 2;
//             int **initArray2 = new int *[d1];
//             for (int i = 1, j = 0; i < node->getAllChildren().size() - 1; i += 2, ++j) {
//                 ConstValue *retArray1 = this->ConstInitVal(node->getChildAt(i), 1);
//                 initArray2[j] = new int[retArray1->d];
//             initArray2[j] = retArray1->value.array1;
//                 for (int k = 0; k < retArray1->d; ++k) {
//                     initArray2[j][k] = retArray1->value.array1[k];
//                 }
//             }
//             constValue->value.array2 = initArray2;
//             return constValue;
    }
}






// MulExp → UnaryExp | MulExp ('*' | '/' | '%') UnaryExp
SymbolTableEntry *SymbolTableHandler::MulExp(Node *node, bool fromConstExp, int *constExpValue,QuadItem *quadItem) {
    printf("Symbol: MulExp \n");
    SymbolTableEntry *entry1 = nullptr, *entry2 = nullptr;
    int value1 = 0, value2 = 1;
    //Unary
    if (node->getAllChildren().size() == 1) {
        SymbolTableEntry * ret =  this->UnaryExp(node->getFirstChild(), fromConstExp, constExpValue,quadItem);
        return ret;
    }
    auto *var1 = new QuadItemInt(IS_GLOBAL);
    auto *var2 = new QuadItemInt(IS_GLOBAL);
    entry1 = this->MulExp(node->getFirstChild(), fromConstExp, &value1, var1);
    entry2 = this->UnaryExp(node->getLastChild(), fromConstExp, &value2,var2);
    if (fromConstExp) {  // 必须为常量
        if (node->getChildAt(1)->token->word_type=="MULT") {
            *constExpValue = value1 * value2;
        } else if (node->getChildAt(1)->token->word_type=="DIV") {
            *constExpValue = value1 / value2;
        } else {
            *constExpValue = value1 % value2;
        }
        return nullptr;
    }

    QuadItemInt *ret = ((QuadItemInt *) quadItem);
    QuadEntryType op = icGenerator->symbolName2binaryOp(node->getChildAt(1)->token->word_type);
    icGenerator->translate_BinaryOperator(op, ret, var1, var2);
    return entry1;  // TODO 生成代码时要计算 entry1 * / % entry2
}

//FIXME:
//UnaryExp → PrimaryExp | Ident '(' [FuncRParams] ')' | UnaryOp UnaryExp
// UnaryExp → PrimaryExp | Ident '(' [FuncRParams] ')' | UnaryOp UnaryExp // c d e
//  c => ErrorType::IdentUndefined
//  d => ErrorType::ParamNumNotMatch
//  e => ErrorType::ParamTypeNotMatch
//  j => ErrorType::MissingRPARENT )
// 全是 UnaryExp → Ident '(' [FuncRParams] ')' 的错
SymbolTableEntry *SymbolTableHandler::UnaryExp(Node *node, bool fromConstExp, int *constExpValue,QuadItem * quadItem) {
    printf("Symbol: UnaryExp \n");
    Node *firstChild = node->getFirstChild();
    //PrimaryExp
    if (firstChild->equals(GrammarItem::PrimaryExp)) {
        return this->PrimaryExp(firstChild, fromConstExp, constExpValue,quadItem);
    }
    //UnaryOp UnaryExp
    if (firstChild->equals(GrammarItem::UnaryOp)) {
        if (firstChild->getFirstChild()->token->word_type == "PLUS") {
            SymbolTableEntry *ret = this->UnaryExp(node->getChildAt(1), fromConstExp,
                                                         constExpValue, quadItem);
            return ret;
        }
        // UnaryOp 不是 +，为 - 或 ！
        auto *dstQuadItem = dynamic_cast<QuadItemInt *>(quadItem);
        auto *srcQuadItem = new QuadItemInt(IS_GLOBAL);
        SymbolTableEntry *ret = this->UnaryExp(node->getChildAt(1), fromConstExp,
                                                     constExpValue, srcQuadItem);
        QuadEntryType op = icGenerator->symbol2unaryOp(firstChild->getFirstChild()->token->word_type);
        //翻译一元运算
        icGenerator->translate_UnaryOperator(op, dstQuadItem, srcQuadItem);
        if(op == QuadEntryType::Neg) {
            *constExpValue = -(*constExpValue);
        }else *constExpValue = !(*constExpValue);
        return ret;
    }
    //Ident [funcParam]
    //使用未声明的变量
    if (!this->currentTable->nameExistedInAllTables(firstChild)) {
        //printf("ERROR: Ident undefined name is : %s\n",firstChild->token->word.c_str());
        this ->handleIdentUndefined(firstChild->token->line_num);
        *constExpValue = 0;
        return nullptr;
    } else {
        SymbolTableEntry *definedEntry = currentTable->getEntryByNameFromAllTables(firstChild);
        //参数数量不匹配
        if (!node->getChildAt(2)->equals(GrammarItem::FuncRParams)) {  // 没有参数
            if (definedEntry->funcParamsNum() > 0) {
                this ->handleParamNumNotMatch(firstChild->token->line_num);
                *constExpValue = 0;
                return nullptr;  // 出错了
            }
            //参数数量匹配，调用FuncCall
            icGenerator->translate_FuncCall(definedEntry->getName());
        } else {  // 有实参
            auto *params = new std::vector<QuadItem *>;
            //调用 FuncRParams 分析实参列表，收集解析出的符号表条目（calledEntry）以及对应的中间代码表示（params）
            std::vector<SymbolTableEntry *> *calledEntry
                    = this->FuncRParams(node->getChildAt(2), definedEntry,params);
            //for(SymbolTableEntry *symbol_table_entry:*calledEntry) printf("%s\n",SymbolTableEntryType2String.find(symbol_table_entry->type)->second.c_str());
            //检查参数类型是否匹配
            if (this->findParamError(definedEntry, calledEntry, firstChild->token->line_num)) {
                *constExpValue = 0;
                return nullptr;  // 出错了

            }
            icGenerator->translate_FuncCall(definedEntry->getName(), params);
        }
        //返回值处理
        if (quadItem != nullptr) {
            QuadItemInt *dstQuadItem = ((QuadItemInt *) quadItem);
            icGenerator->translate_BinaryOperator(QuadEntryType::Assign, dstQuadItem, nullptr);
        }else printf("quadItem=nullptr\n");
        //返回被调用的函数的符号表项
        return definedEntry;
    }
}

// PrimaryExp → '(' Exp ')' | LVal | Number |Character
SymbolTableEntry *SymbolTableHandler::PrimaryExp(Node *node, bool fromConstExp, int *constExpValue ,QuadItem * quadItem) {
    printf("Symbol: PrimaryExp \n");
    SymbolTableEntry *ret = nullptr;
    if (node->getAllChildren().size() > 1) {
#ifdef ERROR_HANDLER_DEBUG
        std::cout << "\nChecking PrimaryExp -> '(' Exp ')':\nline: "
                  << node->getFirstChild()->token->line_num << std::endl;
#endif
        ret = this->Exp(node->getChildAt(1), fromConstExp, constExpValue,quadItem);
        //FIXME : delete error handle
        return ret;
    }
    if (node->getFirstChild()->equals(GrammarItem::LVal)) {
        return this->LVal(node->getFirstChild(), fromConstExp, constExpValue, quadItem);
    }else if(node->getFirstChild()->equals(GrammarItem::Number)) {
        return  this->Number(node->getFirstChild(), fromConstExp, constExpValue, quadItem);
    }else {
        return this -> Character(node->getFirstChild(), fromConstExp,constExpValue, quadItem);
    }
}
// FuncRParams → Exp { ',' Exp }
std::vector<SymbolTableEntry *> *SymbolTableHandler::FuncRParams(
        Node *node,
        SymbolTableEntry *entry,
        std::vector<QuadItem *> *params ) {
    printf("SymbolTableHandler::FuncRParas\n");
    checkingFuncRParams = true;

    SymbolTableEntry *retEntry = nullptr;
    auto *funcRParams = new std::vector<SymbolTableEntry *>();
    std::vector<Node *> children = node->getAllChildren();
    int temp = 0;
    for (auto i = 0; i < children.size(); i += 2) {
        //quadItem : 加入FuncRParams的中间代码实体 ->带入Exp
        auto *quadItem = new QuadItemInt(IS_GLOBAL);
        retEntry = this->Exp(node->getChildAt(i), false, &temp,quadItem);
        params->push_back(quadItem);
        if (retEntry != nullptr) {
            funcRParams->push_back(retEntry);
        } else {  // 函数实参为常数
            auto *constParam = new SymbolTableEntry(temp);
            funcRParams->push_back(constParam);
        }
    }
    checkingFuncRParams = false;
    return funcRParams;
}



// 检查函数定义 definedEntry 和 函数调用 calledEntry 中的下面两个错：
// FIXME: d => ErrorType::ParamNumNotMatch
// FIXME: e => ErrorType::ParamTypeNotMatch
// 返回值表示是否有错
bool SymbolTableHandler::findParamError(SymbolTableEntry *definedEntry,
                                  std::vector<SymbolTableEntry *> *calledEntry,
                                  int lineNum) {
    printf("Symbol:findParamError\n");
    auto size = calledEntry->size();
    if (definedEntry->funcParamsNum() != size) {
        this->handleParamNumNotMatch(lineNum);
        return true;
    }
    std::vector<FuncParam *> *definedFuncParams = definedEntry->getFuncParams();
    for (auto i = 0; i < size; ++i) {
        // hasSameType(SymbolTableEntry *realParam, FuncParam *funcParam)
        bool typeSame = SymbolTableEntry::hasSameType((*calledEntry)[i],
                                                      (*definedFuncParams)[i]);
        if (!typeSame) {
            //printf("Error %s %d\n",SymbolTableEntryType2String.find((*calledEntry)[i]->type)->second.c_str(),(*definedFuncParams)[i]->type);
            this -> handleParamTypeNotMatch(lineNum);
            return true;
        }
    }
    return false;
}

SymbolTableEntry *SymbolTableHandler::Exp(Node *node, bool fromConstExp, int *constExpValue,QuadItem * quadItem) {
    printf("Symbol: Exp\n");
#ifdef ERROR_HANDLER_DEBUG
    currentTable->printAllNames();
#endif
    SymbolTableEntry * ret = this ->AddExp(node->getFirstChild(), fromConstExp, constExpValue, quadItem, false);
    return ret;
}
//TODO: 这里没处理
//LVal → Ident {'[' Exp ']'} // c k
//1.验证变量是否存在
//2，确保从常量表达式中引用常量
//fromConstExp 是否来自常量表达式，如果是则引用变量必须为常量
//constExpValue 存储常量表达式求值的结果

//ICGenerator:
//根据声明值和调用的孩子数量确定是：
//1.变量给变量赋值
//2.数组给变量赋值
//3.数组给数组赋值

SymbolTableEntry *SymbolTableHandler::LVal(Node *node, bool fromConstExp, int *constExpValue,QuadItem * quadItem) {
    std:: map<QuadItemType,string>typeMap = QuadItemType2Name();
    printf("Symbol: LVal \n");
   // printf("QuadItem Type analyse in LVal : %s\n",typeMap.find(quadItem->type)->second.c_str());
    Node *ident = node->getFirstChild();
    //int tokenPos = ident->token->tokenPos;
    //TODO: 错误处理
    if (!this->currentTable->nameExistedInAllTables(ident)) { //变量名未出现在符号表中
        printf("Error in LVal\n");
        this -> handleIdentUndefined(ident->token->line_num);
        *constExpValue = 0;
        return nullptr;
    }
    //获取变量条目和处理节点数量
    auto size = node->getAllChildren().size();  // 1【标识符】 or 4 【一维数组】
    if (size > 1)size = 4;
    //如果存在变量则获取相应条目
    SymbolTableEntry *definedEntry = this->currentTable->getEntryByNameFromAllTables(ident);
    SymbolTableEntryType definedType = definedEntry->getActualType();
    SymbolTableEntry *referencedEntry = nullptr;
    QuadItem *existedItem = currentTable->getQuadItemByNameFromAllTables(ident);
    //这一个步骤出问题了
    if(quadItem==nullptr)quadItem = new QuadItem;
    quadItem->reference = existedItem;
    quadItem->isFuncRParam = checkingFuncRParams;

    // 求出数组引用下标 v1 v2
    int v1 = 0;
    Node *exp1;
    QuadItemInt *quadItemVar1;
    SymbolTableEntry *exp1Entry;

    if (size >= 4) {
        exp1 = node->getChildAt(2);
        quadItemVar1 = new QuadItemInt(IS_GLOBAL);
        exp1Entry = this->Exp(exp1,fromConstExp,&v1,quadItemVar1);
    }

    switch (size) {
        //普通变量
        case 1: {  // ident // 调用时传入的实参为 "param"
            //变量为常量表达式的一部分
            //如果是常量，则从 definedEntry 中获取变量值并赋值给 constExpValue
            if (definedType == SymbolTableEntryType::Int ||
                definedType == SymbolTableEntryType::IntConst) {
                // 定义类型为 Var，实际传进去的是 Var
                quadItem->type = QuadItemType::Int;
                quadItem->referenceType = ReferenceType::Int; //
                if (fromConstExp) { //若为const类型则当作立即数处理
                    quadItem->type = QuadItemType::Imm;
                    quadItem->referenceType = ReferenceType::Unset;
                    *constExpValue = definedEntry->intGet();
                    return nullptr;
                }
                return definedEntry;
                }
            else if (definedType == SymbolTableEntryType::Char ||
               definedType == SymbolTableEntryType::CharConst) {
                // 定义类型为 Var，实际传进去的是 Var
                quadItem->type = QuadItemType::Char;
                quadItem->referenceType = ReferenceType::Char;
                if (fromConstExp) { //若为const类型则当作立即数处理
                    quadItem->type = QuadItemType::Imc;
                    quadItem->referenceType = ReferenceType::Unset;
                    *constExpValue = definedEntry->charGet();
                    return nullptr;
                }
                return definedEntry;
               }
            else if (definedType == SymbolTableEntryType::IntArray ||
                           definedType == SymbolTableEntryType::ConstIntArray) {
                    // 定义类型为 Array1，实际传进去的类型为 Array1
                    quadItem->type = QuadItemType::IntArray;
                    quadItem->referenceType = ReferenceType::IntArray;
                    if (fromConstExp) {
                        throw std::runtime_error("\nError in ErrorHandler.cpp, line " +
                                                 std::to_string(__LINE__) +
                                                 ": get 'fromConstExp = true' while "
                                                 "checking Array1_Array1 LVal\n");
                        *constExpValue = definedEntry->intGet();
                        return nullptr;
                    }
                    return definedEntry;
                           }
            else if (definedType == SymbolTableEntryType::CharArray ||
                           definedType == SymbolTableEntryType::ConstCharArray) {
                printf("definedType : CharArray\n");
                // 定义类型为 Array1，实际传进去的类型为 Array1
                quadItem->type = QuadItemType::CharArray;
                quadItem->referenceType = ReferenceType::CharArray;
                if (fromConstExp) {
                    throw std::runtime_error("\nError in ErrorHandler.cpp, line " +
                                             std::to_string(__LINE__) +
                                             ": get 'fromConstExp = true' while "
                                             "checking Array1_Array1 LVal\n");
                    *constExpValue = definedEntry->intGet();
                    return nullptr;
                }
                return definedEntry;
                           }
        }
        case 4: // ident[exp]
            //exp
            if (definedType == SymbolTableEntryType::IntArray ||
                definedType == SymbolTableEntryType::ConstIntArray) {
                // 实际传进去的是 var
                quadItem->type = QuadItemType::Int;
                quadItem->referenceType = ReferenceType::IntArray_Var;
                referencedEntry = new SymbolTableEntry(definedEntry,
                                                       SymbolTableEntryType::Int, v1);
                if (fromConstExp) {
                    quadItem->array1_var_index = new QuadItemImm(v1);
                    *constExpValue = referencedEntry->getValueFromReferencedIntArray(v1);
                    return nullptr;
                } else {
                    quadItem->array1_var_index = quadItemVar1;
                    return referencedEntry;
                }
                }
                else if(definedType == SymbolTableEntryType::CharArray ||
                    definedType == SymbolTableEntryType::ConstCharArray){
                    printf("LVal : analyse CharArray\n");
                    // 实际传进去的是 var
                    quadItem->type = QuadItemType::Char;
                    quadItem->referenceType = ReferenceType::CharArray_Var;
                    referencedEntry = new SymbolTableEntry(definedEntry,
                                                           SymbolTableEntryType::Char, v1);
                    if (fromConstExp) {
                        quadItem->array1_var_index = new QuadItemImm(v1);
                        *constExpValue = referencedEntry->getValueFromReferencedIntArray(v1);
                        return nullptr;
                    } else {
                        quadItem->array1_var_index = quadItemVar1;
                        return referencedEntry;
                    }
                }
                    }


#ifdef ERROR_HANDLER_DEBUG
    std::cout << "\nSymbolTableHandler::LVal:\n" << ident->getToken()->value
              << ", line " << ident->token->line_num << "\n"
              << "size: " << size << std::endl;
#endif
    throw std::runtime_error("Reach line 481 in SymbolTableHandler.cpp");
}








// Number → IntConst
// IntConst -> 数字
SymbolTableEntry *SymbolTableHandler::Number(Node *node, bool fromConstExp, int *constExpValue,QuadItem * quadItem) {
printf("start analyse Number \n");
    *constExpValue = std::stoi(
            node->getFirstChild()->token->word);
    //更新中间代码项
    if (quadItem != nullptr) {
        std::map<QuadItemType, std::string> typeMap = QuadItemType2Name();
        if (quadItem->type == QuadItemType::Imm) {
            ((QuadItemImm *) quadItem)->value = *constExpValue;
        } else {
            ((QuadItemInt *)quadItem)->isConst = true;
            ((QuadItemInt *)quadItem)->value = *constExpValue;
        }
    }
    printf("-> end analyse Number\n");
    return nullptr;
}

//TODO : 未修改
//Character → CharConst
SymbolTableEntry *SymbolTableHandler::Character(Node *node, bool fromConstExp, int *constExpValue,QuadItem * quadItem) {
    printf("start analyse Character \n");
#ifdef ERROR_HANDLER_DEBUG
    std::cout << "\nChecking Number -> IntConst:\nline: "
              << node->getFirstChild()->token->line_num << ", number: "
              << node->getFirstChild()->getToken()->value << std::endl;
#endif
    if(node->getFirstChild()->token->word.length()==4) {
        if(node->getFirstChild()->token->word[1]=='\\'){
            if(node->getFirstChild()->token->word[2]=='a') {
                *constExpValue = 7;
            }
            else  if(node->getFirstChild()->token->word[2]=='b') {
                *constExpValue = 8;
            }
            else  if(node->getFirstChild()->token->word[2]=='t') {
                *constExpValue = 9;
            }
            else  if(node->getFirstChild()->token->word[2]=='n') {
                *constExpValue = 10;
            }else  if(node->getFirstChild()->token->word[2]=='v') {
                *constExpValue = 11;
            }
            else if(node->getFirstChild()->token->word[2]=='f') {
                *constExpValue = 12;
            }
            else if(node->getFirstChild()->token->word[2]=='\"') {
                *constExpValue = 34;
            }
            else if(node->getFirstChild()->token->word[2]=='\'') {
                *constExpValue = 39;
            }
            else if(node->getFirstChild()->token->word[2]=='\\') {
                *constExpValue = 92;
            }
            else if(node->getFirstChild()->token->word[2]=='0') {
                *constExpValue = 0;
            }
        }
    }else {
        *constExpValue = (int)node->getFirstChild()->token->word[1];
    }
    //printf("WORD IS %s %llu\n",node->getFirstChild()->token->word.c_str(),node->getFirstChild()->token->word.length());
    if (quadItem != nullptr) {
        if (quadItem->type == QuadItemType::Imc) {
            ((QuadItemImc *) quadItem)->value = (char)*constExpValue;
        } else {
            ((QuadItemChar *)quadItem)->isConst = true;
            ((QuadItemChar *)quadItem)->value = (char)*constExpValue;
        }
    }
    return nullptr;
}


//  FuncFParams → FuncFParam { ',' FuncFParam }
// 1.将所有形参加入符号表
void SymbolTableHandler::FuncFParams(Node *funcFParams, SymbolTableEntry *funcIdentEntry) {

    printf("Symbol: FuncFParams\n");
    //存储当前处理的形参条目
    SymbolTableEntry *funcFParam;
    // 获取实际类型，如有变量定义为 int arr[1][2], 其作为实参时为 arr[1]
    // 此时实参arr[1]的actualType为 array1
    //函数的返回类型
    SymbolTableEntryType retType = funcIdentEntry->getActualType();
    for (int i = 0; i < funcFParams->getAllChildren().size(); i += 2) {
        //处理单个形参
        funcFParam = this->FuncFParam(funcFParams->getChildAt(i), retType);
        //TODO: 重复形参的错误处理
        if (currentTable->nameExistedInCurrentTable(funcFParam->node)) {
            //FIXME: Handle error b
            this -> handleIdentRedefined(funcFParam->node->token->line_num);
        } else {
            //1.将条目添加至符号表
            currentTable->addEntry(*(funcFParam->getName()), funcFParam);
        }
        //2.将形参添加至函数的符号表条目
        funcIdentEntry->addParamForFuncEntry(funcFParam);
    }
}

//2024.10.27
// FuncFParam → BType Ident ['[' ']' { '[' ConstExp ']' }]  //   b k
// FIXME: b => ErrorType::IdentRedefined
// FIXME: k => ErrorType::MissingRBRACK  ]
SymbolTableEntry *SymbolTableHandler::FuncFParam(Node *funcFParam, SymbolTableEntryType retType) {
    printf("Symbol: FuncFParam \n");
    Node *ident = funcFParam->getChildAt(1);
#ifdef ERROR_HANDLER_DEBUG
    std::cout << "\nChecking FuncFParam:\nline: "
              << ident->token->line_num << ", number: "
              << ident->getToken()->value << std::endl;
#endif
    if (currentTable->nameExistedInCurrentTable(ident)) {
        //FIXME: handle error b
        this->handleIdentRedefined(ident->token->line_num);
    }
    //只有标识符号的简单变量
    if (funcFParam->getAllChildren().size() == 2) {
        //int param
        if(funcFParam->getFirstChild()->getFirstChild()->token->word_type=="INTTK") {
            printf("INTTK\n");
            auto *intVar =  new Int();
            auto *varParam = new SymbolTableEntry(ident, intVar, ident->token->line_num, true);
            return varParam;
        }
        //char param
        if(funcFParam->getFirstChild()->getFirstChild()->token->word_type=="CHARTK") {
            printf("CHARTK\n");
            auto *charVar =  new Char();
            auto *varParam = new SymbolTableEntry(ident, charVar, ident->token->line_num, true);
            return varParam;
        }
    }
    //一维数组
    if (funcFParam->getAllChildren().size() > 2) {
        //int array
        if(funcFParam->getFirstChild()->getFirstChild()->token->word_type=="INTTK") {
            printf("INTTK\n");
            //创建数组变量条目并存入表项
            auto *intArray = new IntArray(-1);
            auto *intArrayParam = new SymbolTableEntry(
             ident, intArray, ident->token->line_num, true);
            return intArrayParam;
        }
        //char array
        if(funcFParam->getFirstChild()->getFirstChild()->token->word_type=="CHARTK") {
            printf("CHARTK\n");
            auto *intArray = new CharArray(-1);
            auto *intArrayParam = new SymbolTableEntry(
             ident, intArray, ident->token->line_num, true);
            return intArrayParam;
        }
    }
    //TODO : handle mult_dimension array
    // const int d2 = this->ConstExp(funcFParam->getChildAt(5));
    // auto *array2 = new Array2(-1, 2);
    // auto *array2Param = new SymbolTableEntry(ident, array2, ident->token->line_num, true);
    // auto *errorNode = dynamic_cast<ErrorNode *>(funcFParam->getChildAt(3));
    // if (errorNode != nullptr) {
    //     errorLog.insert({errorNode->lineNum, errorNode->error()});
    // }
    // errorNode = dynamic_cast<ErrorNode *>(funcFParam->getChildAt(6));
    // if (errorNode != nullptr) {
    //     errorLog.insert({errorNode->lineNum, errorNode->error()});
    // }
    // return array2Param;
}

// Block → '{' { BlockItem } '}'
// 需在调用前新建符号表!!!!!
//1.获取block结束后的行号
//TODO:检查调用前是否新建符号表
void SymbolTableHandler::Block(Node *block, bool inFuncBlock) {
    printf("Symbol: Block \n");
    createSymbolTableBeforeEnterBlock = false;
    //{ BlockItem }
    for (int i = 1; i < block->getAllChildren().size()-1; ++i) {
        this->BlockItem(block->getChildAt(i), inFuncBlock);
    }
    funcEndLineNum = block->getLastChild()->token->line_num;  // 结尾分号的行号
}


// BlockItem → Decl | Stmt
void SymbolTableHandler::BlockItem(Node *blockItem, bool inFuncBlock) {
    printf("Symbol: BlockItem \n");
    //Decl
    if (blockItem->getFirstChild()->equals(GrammarItem::Decl)) {
        this->Decl(blockItem->getFirstChild());
    } else {
        this->Stmt(blockItem->getFirstChild(), inFuncBlock);
    }
}

//保留了while
//每个左值都需要检查
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
void SymbolTableHandler::Stmt(Node *stmt, bool inFuncBlock) {
    std::map<QuadItemType,string>typeMap = QuadItemType2Name();
    printf("Symbol: Stmt \n");
    Node *first = stmt->getFirstChild();
    Node *last = stmt->getLastChild();
    int temp;
    char tempchar;
    //
    if (first->equals(GrammarItem::LVal)) {
        printf("Stmt -> LVal = ...\n");
        // Stmt → LVal '=' Exp ';'
        // Stmt → LVal '=' 'getint''('')'';'
        auto *lValItem = new QuadItem();
        SymbolTableEntry *firstEntry = this->LVal(first, false, &temp,lValItem);
        //QuadItem *lRefItem = lValItem->reference;

        if (firstEntry == nullptr) return;  // 名字未定义
        if (firstEntry->isConst()) {
            // 行号： LVal → Ident {'[' Exp ']'}
            //FIXME:h
            this->handleConstantModification(first->getFirstChild()->token->line_num);
        }
        if (stmt->getChildAt(2)->equals(GrammarItem::Exp)) {  // LVal '=' Exp ';'
            QuadItem *rightQuadItem = new QuadItemInt(IS_GLOBAL);
            this->Exp(stmt->getChildAt(-2), false, &temp,rightQuadItem);
            printf("SymboltableHandler: translate_UnaryOp  Op is Assign\n");
            icGenerator->translate_UnaryOperator(QuadEntryType::Assign, lValItem, rightQuadItem);
        } else if(stmt->getChildAt(2)->token->word_type=="GETINTTK"){  // LVal '=' 'getint''('')'';'
            icGenerator->translate_getint(lValItem);
            }else if(stmt->getChildAt(2)->token->word_type=="GETCHARTK")
            {  // LVal '=' 'getint''('')'';'
                icGenerator->translate_getchar(lValItem);
            }

    } else if (first->equals(GrammarItem::Block)) {  // Stmt → Block
        auto *childTable = new SymbolTable(currentTable, false,this->count++);
        this->symbolTableList.push_back(childTable);

        currentTable = childTable;
        createSymbolTableBeforeEnterBlock = true;

        this->Block(first);
        currentTable = currentTable->parent;
    } else if (first->equals(GrammarItem::Exp) || stmt->getAllChildren().size() == 1) {  // Stmt → [Exp] ';'
        printf("Stmt → [Exp] ';'\n");
        if (first->equals(GrammarItem::Exp)) {
            //auto *quadItem = new QuadItem;
            this->Exp(first, false, &temp,nullptr);
        }
        //FIXME :delete handle error
    } else if (first->token->word_type=="IFTK") {
        // Stmt → 'if' '(' Cond ')' Stmt [ 'else' Stmt ]
        printf("Stmt → 'if' '(' Cond ')' Stmt [ 'else' Stmt ]\n");
        const bool hasElse = (stmt->getAllChildren().size()>5);

        QuadItemLabel * endLabel;

        QuadItem *cond = new QuadItemInt(IS_GLOBAL);
        this->Cond(stmt->getChildAt(2), cond);

        //条件为假时
        //Beqz： cond等于0时，跳转到jumpLabel
        auto *jumpLabel = new QuadItemLabel();
        icGenerator->translate_Beqz(cond, jumpLabel);  // beqz cond, jumpLabel

        if(stmt->getChildNum()%2==1) {
            this->Stmt(stmt->getChildAt(4));
        }else this->Stmt(stmt->getChildAt(3));

        endLabel = new QuadItemLabel();
        //执行完if中的stmt后跳过else到endLable
        if (hasElse) {
            icGenerator->translate_JumpLabel(endLabel);
        }

        icGenerator->translate_InsertLabel(jumpLabel);  // jumpLabel:

        //当有else时，需要插入endLable并对endLabel进行判断
        if (hasElse) {
            if (stmt->getAllChildren().size() > 5) {
                if(stmt->getChildNum()%2==1)this->Stmt(stmt->getChildAt(6));
                else this->Stmt(stmt->getChildAt(5));
            }
            icGenerator->translate_InsertLabel(endLabel);
        }
    }
    //TODO:handle While
    // else if (first->token ->word_type == "WHILETK") {  // Stmt → 'while' '(' Cond ')' Stmt
    //     printf("Stmt → 'while' '(' Cond ')' Stmt\n");
    //     this->Cond(stmt->getChildAt(2));
    //     inWhile++;
    //     if(stmt->getChildNum()%2==1)this->Stmt(stmt->getChildAt(4));
    //     else this->Stmt(stmt->getChildAt(3));
    //     inWhile--;
    // }
    else if (first->token->word_type == "BREAKTK" || first->token->word_type == "CONTINUETK") {
        if (!currentForBlock->inWhile()) {
            //FIXME:m
            this->handleredundantBreakContinue(first->token->line_num);
            return;
        }
        if (first->token->word_type == "BREAKTK") {
            icGenerator->translate_JumpLabel(currentForBlock->forEndLabel);
        } else {
            //continue:跳转到步进语句
            icGenerator->translate_JumpLabel(currentForBlock->forBodyEndLabel);
        }
        //checkErrorNode(last);  // ErrorType::MissingSEMICN
        //FIXME : delete handle error
    } else if (first->token->word_type == "RETURNTK") {  // 'return' [Exp] ';'
        receiveReturn = inFuncBlock;
        if ((currentFunction != Func::IntFunc && currentFunction != Func::MainFunc)
            && stmt->getChildAt(1)->equals(GrammarItem::Exp)) {
            //FIXME:f
            this->handleReturnRedundant(first->token->line_num);
        }
        if (stmt->getChildAt(1)->equals(GrammarItem::Exp)) {
            QuadItem * quadItem = new QuadItemInt(false);
            this->Exp(stmt->getChildAt(1), false, &temp,quadItem);
            icGenerator->translate_return(quadItem);
        }else {
            icGenerator->translate_return();
        }
    }
   //FIXME:未修改
    else if(first->token->word_type == "FORTK") {//'for' '(' [ForStmt] ';' [Cond] ';' [ForStmt] ')' Stmt // h
        //获取for下三个值的出现情况
        auto *forStartLabel = new QuadItemLabel();//循环条件开始
        auto *forBodytLabel = new QuadItemLabel();//循环体开始
        auto *forBodyEndLable=new QuadItemLabel();
        auto *forEndLabel = new QuadItemLabel();//循环条件结束
        int ifCond[3] ={0,0,0};
        Analyse_forStmt_cond(first->token->tokenPos,ifCond);
        //for(int i = 0; i < 3; i++)printf("%d ",ifCond[i]);
        //printf("\n");

        //ForStmt
        if(ifCond[0])this->ForStmt(stmt->getChildAt(2),&temp); //TODO：初始化语句生成目标代码

        //插入循环条件检查位置
        icGenerator->translate_InsertLabel(forStartLabel);

        //Cond
        if(ifCond[1]) {
            int first_semi_pos = 2+ifCond[0];
            QuadItem *cond = new QuadItemInt(IS_GLOBAL); //定义一个条件变量
            this->Cond(stmt->getChildAt(first_semi_pos+1), cond); //生成条件表达式代码
            icGenerator->translate_Beqz(cond,forEndLabel); //Cond条件不成立时，跳转到循环结束
        }

        //TODO:未对第二个ForStmt进行修改
        //ForStmt
        // should been insert here
        inWhile ++;
        //Stmt

        //插入循环体开始位置
        icGenerator->translate_InsertLabel(forBodytLabel);

        //创建并更新 ForBlock(类似 WhileBlock 的结构)
        auto *newForBlock = new ForBlock(currentForBlock, forStartLabel,forBodytLabel,forBodyEndLable, forEndLabel);
        currentForBlock->child = newForBlock;
        currentForBlock = newForBlock;

        int rparent_pos = 4+ ifCond[0] + ifCond[1] + ifCond[2];
        this->Stmt(stmt->getChildAt(rparent_pos+1));
        inWhile--;

        icGenerator->translate_InsertLabel(forBodyEndLable);
        //TODO: 是否需要修改第二个ForStmt的判断
        //处理步进语句 ForStmt → LVal '=' Exp
        if(ifCond[2]) {
            int second_semi_pos = 2 + ifCond[0] + 1 + ifCond[1];
            this->ForStmt(stmt->getChildAt(second_semi_pos+1),&temp);
        }
         // 循环跳转到条件检查位置
        icGenerator->translate_JumpLabel(forStartLabel);
        // 插入循环结束位置
        icGenerator->translate_InsertLabel(forEndLabel);
        // 恢复父级块
        currentForBlock = currentForBlock->parent;
    }
    else {  // 'printf''('FormatString{','Exp}')'';'
        // ErrorType::FormatStrNumNotMatch
        std::map<QuadItemType,string>typeMap = QuadItemType2Name();
        int leftNum = 0, rightNum = 0;
        Node *formatStr = stmt->getChildAt(2);
        auto *posOfPer = new std::vector<int>;
        auto *posOfChar = new std::vector<int>;
        auto *varItems = new std::vector<QuadItem*>;
        printf("Stmt: printf sentence \n"); //ConstString is %s tokenPos is %d firstChildren's TokenType is %s\n",formatStr->token->word.c_str(),formatStr->token->tokenPos,formatStr->token->word_type.c_str());
        bool hasIllegalChar = SymbolTableHandler::StringConst(formatStr, &leftNum, posOfPer, posOfChar);
        for (int i = 4; i < stmt->getAllChildren().size() - 2; i += 2) {
            ++rightNum;
            QuadItem * quadItem = new QuadItemInt(IS_GLOBAL);
            this->Exp(stmt->getChildAt(i), false, &temp,quadItem);
            printf("fromExp insert into quadItem: type:%s\n",typeMap.find(quadItem->type)->second.c_str());
            varItems->push_back(quadItem);
        }
        if (leftNum != rightNum) {
            printf("leftNum is %d rightNum is %d\n",leftNum,rightNum);
          //FIXME:l
            this->handleFormatStrNumNotMatch(formatStr->token->line_num);
        }else {
            printf("SymbolHandler::Stmt -> icGenerator::translator_printf\n");
            icGenerator->translate_printf(posOfPer,varItems,&formatStr->token->word);
        }
    }

}

//TODO : 未进行修改
//ForStmt → LVal '=' Exp // h
void SymbolTableHandler::ForStmt(Node * forstmt,int *temp) {
    printf("SymbolTableHandler:ForStmt\n");
    // Stmt → LVal '=' Exp ';'
    // Stmt → LVal '=' 'getint''('')'';'
    auto *lValICItem = new QuadItem();
    SymbolTableEntry *firstEntry = this->LVal(forstmt->getFirstChild(), false, temp, lValICItem);
    //        ICItem *leftReferenceICItem = lValICItem->lValReference;
    if (firstEntry == nullptr) {
        this->handleFormatStrNumNotMatch(forstmt->getFirstChild()->getFirstChild()->token->line_num);
        return;
    }  // 名字未定义;
    if(firstEntry->isConst()) {
        this->handleConstantModification(forstmt->getFirstChild()->getFirstChild()->token->line_num);
        //return;
    }
    QuadItem *rightICItem = new QuadItemInt(IS_GLOBAL);
    this->Exp(forstmt->getChildAt(-1), false, temp, rightICItem);
    // TODO：在 MipsTranslator 中判断 LVal
    icGenerator->translate_UnaryOperator(QuadEntryType::Assign, lValICItem, rightICItem);
}


// Cond → LOrExp
void SymbolTableHandler::Cond(Node *node,QuadItem *quadItem) {
    printf("Symbol: Cond \n");
    this->LOrExp(node->getFirstChild(),quadItem);
}

//StringConst
//1.检查是否有非法字符 有非法字符则返回true
//2.检查占位符数量
//TODO: 修改此函数
bool SymbolTableHandler::StringConst(Node *node, int *formatNum,std::vector<int> *PosOfPer,std::vector<int> *PosofChar) {
    printf("Symbol: StringConst %s\n",node->token->word.c_str());
    //格式化占位符的数量
    *formatNum = 0;
    std::string s = node ->token ->word;
    bool illegalChar = false;
    //更新所有%的位置
    int cur = 1,index = 0;
    while(cur < s.size()-1) {
        index = s.find('%',cur);
        if(index > s.size())break;
        PosOfPer ->push_back(index);
        cur = index + 2;
    }
    //检查字符串的每个字符是否合法
    for (int i = 1; i < s.size() - 1; ++i) {
        // check <FormatChar>
        auto now = s[i];
        auto next = s[i + 1];  // i < length - 1保证了不越界

        //占位符与非法字符
        if (now == '%' && ((next != 'd'&&next != 'c' && next != 's') ||i >= s.size() - 2 )) {
            illegalChar = true;
        } else if (now == '%' && (next =='d' ||next == 'c' || next == 's')) {//%d
            *formatNum = *formatNum + 1;
            i++;  // 从后两个开始判断
        } else if (now == '\\' && next == 'n') { //\n
            ++i; //跳过两个
        } else if (now == '\\' && next != 'n') {//转义符下的非法字符
            illegalChar = true;
        } else { //其他的非法字符
            int ascii = (int) ((unsigned char) now);
            if (!((ascii >= 40 && ascii <= 126)||ascii == 32 || ascii == 33  )) {
                illegalChar = true; //空格，感叹号，常见符号，字母，数字
            }
        }
    }
    return illegalChar;
}



// LOrExp → LAndExp | LOrExp '||' LAndExp
void SymbolTableHandler::LOrExp(Node *node, QuadItem *quadItem) {
    printf("Symbol: LOrExp \n");
    if (node->getAllChildren().size() == 1) {
        this->LAndExp(node->getFirstChild(), quadItem);
        return;
    }

    //新声明两个QuadItem
    auto *var1 = new QuadItemInt(IS_GLOBAL);
    auto *var2 = new QuadItemInt(IS_GLOBAL);
    auto *var1EndLabel = new QuadItemLabel();
    auto *endLabel = new QuadItemLabel();

    this->LOrExp(node->getFirstChild(), var1);
    icGenerator->translate_Bnez(var1, var1EndLabel); //

    this->LAndExp(node->getLastChild(), var2);
    icGenerator->translate_BinaryOperator(QuadEntryType::Assign, quadItem, var2);
    icGenerator->translate_JumpLabel(endLabel);

    icGenerator->translate_InsertLabel(var1EndLabel);
    icGenerator->translate_BinaryOperator(QuadEntryType::Assign, quadItem, var1);


    icGenerator->translate_InsertLabel(endLabel);

    //    QuadItemInt *ret = ((QuadItemInt *) quadItem);
    //    QuadEntryType op = icGenerator->symbol2binaryOp(node->getChildAt(1)->getToken()->symbol);
    //    icGenerator->translate_BinaryOperator(op, ret, var1, var2);
}



// LAndExp → EqExp | LAndExp '&&' EqExp
void SymbolTableHandler::LAndExp(Node *node, QuadItem *quadItem) {
    printf("SymbolHandler:LAndExp\n");
    if (node->getAllChildren().size() == 1) {
        this->EqExp(node->getFirstChild(), quadItem);
    }
    else {
        auto *var1 = new QuadItemInt(IS_GLOBAL);
        auto *var2 = new QuadItemInt(IS_GLOBAL);
        auto *var1EndLabel = new QuadItemLabel();
        auto *endLabel = new QuadItemLabel();

        this->LAndExp(node->getFirstChild(), var1);
        icGenerator->translate_Beqz(var1, var1EndLabel); //当左边=0时，直接跳过右边部分

        this->EqExp(node->getLastChild(), var2);
        icGenerator->translate_BinaryOperator(QuadEntryType::Assign, quadItem, var2);
        icGenerator->translate_JumpLabel(endLabel);

        icGenerator->translate_InsertLabel(var1EndLabel);
        icGenerator->translate_BinaryOperator(QuadEntryType::Assign, quadItem, var1);

        icGenerator->translate_InsertLabel(endLabel);
    }

    //    QuadItemInt *ret = ((QuadItemInt *) quadItem);
    //    QuadEntryType op = icGenerator->symbol2binaryOp(node->getChildAt(1)->getToken()->symbol);
    //    icGenerator->translate_BinaryOperator(op, ret, var1, var2);
}


// EqExp → RelExp | EqExp ('==' | '!=') RelExp
void SymbolTableHandler::EqExp(Node *node, QuadItem *quadItem) {
    printf("Symbol: EqExp \n");
    if (node->getAllChildren().size() == 1) {
        this->RelExp(node->getFirstChild(), quadItem);
    }
    else {
        auto *var1 = new QuadItemInt(IS_GLOBAL);
        auto *var2 = new QuadItemInt(IS_GLOBAL);
        this->EqExp(node->getFirstChild(), var1);
        this->RelExp(node->getLastChild(), var2);

        QuadItemInt *ret = ((QuadItemInt *) quadItem);
        QuadEntryType op = icGenerator->symbolName2binaryOp(node->getChildAt(1)->token->word_type);
        icGenerator->translate_BinaryOperator(op, ret, var1, var2);
    }
}



// RelExp → AddExp | RelExp ('<' | '>' | '<=' | '>=') AddExp
void SymbolTableHandler::RelExp(Node *node, QuadItem *quadItem) {
    int temp = 0;
    if (node->getAllChildren().size() == 1) {
        this->AddExp(node->getFirstChild(), false, &temp, quadItem,false);
        return;
    }

    auto *var1 = new QuadItemInt(IS_GLOBAL);
    auto *var2 = new QuadItemInt(IS_GLOBAL);
    this->RelExp(node->getFirstChild(), var1);
    this->AddExp(node->getLastChild(), false, &temp, var2,false);

    QuadItemInt *ret = ((QuadItemInt *) quadItem);
    QuadEntryType op = icGenerator->symbolName2binaryOp(node->getChildAt(1)->token->word_type);
    icGenerator->translate_BinaryOperator(op, ret, var1, var2);
}


void SymbolTableHandler::handleIdentRedefined(int line_num) const {
    if(!lexer->lineWrong[line_num])lexer->lineWrong[line_num] = 'b';
}

void SymbolTableHandler::handleIdentUndefined(int line_num) const {
    if(!lexer->lineWrong[line_num])lexer->lineWrong[line_num] = 'c';
}

void SymbolTableHandler::handleParamNumNotMatch(int line_num) const {
    if(!lexer->lineWrong[line_num])lexer->lineWrong[line_num] = 'd';
}

void SymbolTableHandler::handleParamTypeNotMatch(int line_num) const {
    if(!lexer->lineWrong[line_num])lexer->lineWrong[line_num] = 'e';
}


void SymbolTableHandler::handleReturnRedundant(int line_num) const {
    if(!lexer->lineWrong[line_num])lexer->lineWrong[line_num] = 'f';
}

void SymbolTableHandler::handleReturnMissing(int line_num) const {
    if(!lexer->lineWrong[line_num])lexer->lineWrong[line_num] = 'g';
}

void SymbolTableHandler::handleConstantModification(int line_num) const {
    lexer->lineWrong[line_num] = 'h';
}

void SymbolTableHandler::handleFormatStrNumNotMatch(int line_num) const {
    lexer->lineWrong[line_num] = 'l';
}

void SymbolTableHandler::handleredundantBreakContinue(int line_num) const {
    lexer->lineWrong[line_num] = 'm';
}


bool SymbolTableHandler::findCharacter(Node * node) {
    if(node ==nullptr)return false;
    if(node->grammarItem==GrammarItem::CharacterItem){return true;}
    for(int i = 0; i<node->getChildNum();i++) {
        if(findCharacter(node->getChildAt(i)))return true;
    }
    return false;
}

bool SymbolTableHandler::findNumber(Node *node) {
    if(node ==nullptr)return false;
    if(node->grammarItem==GrammarItem::Number){return true;}
    for(int i = 0; i<node->getChildNum();i++) {
        if(findCharacter(node->getChildAt(i)))return true;
    }
    return false;
}

//用于检验变量是'IntVal'还是‘charVal’
//检验普通变量
bool SymbolTableHandler::CHARCON_befor_SEMICN(int tokenPos) const {
    printf("CHARCON_before_SEMICN %d\n",tokenPos);
    while(lexer->tokenMap[tokenPos].word_type!="CHRCON" && lexer->tokenMap[tokenPos].word_type!= "SEMICN"&&
        (lexer->tokenMap[tokenPos].line_num==lexer->tokenMap[tokenPos+1].line_num))tokenPos++;
    printf("analysis for charcon stop at %d\n",tokenPos);
    if(lexer->tokenMap[tokenPos].word_type=="CHRCON"){printf("CHARCON_before_SEMICN cons is CHAR\n");return true;}
    return false;
}

//用于检验变量是"IntArray"还是"CharArray"
bool SymbolTableHandler::STRCON_befor_SEMICN(int tokenPos) const {
    while(lexer->tokenMap[tokenPos].word_type!="STRCON" && lexer->tokenMap[tokenPos].word_type!= "SEMICN"&&lexer->tokenMap[tokenPos].word_type!="COMMA"&&
        (lexer->tokenMap[tokenPos].line_num==lexer->tokenMap[tokenPos+1].line_num))tokenPos++;
    if(lexer->tokenMap[tokenPos].word_type=="STRCON") {
        return true;
    }
    return false;
}
bool SymbolTableHandler::CHARTK_at_BType(int tokenPos) const {
    printf("CHARTK_at_BType %d\n",tokenPos);
    while(lexer->tokenMap[tokenPos].word_type!="CHARTK" && lexer->tokenMap[tokenPos].word_type!= "INTTK"&&
      (lexer->tokenMap[tokenPos].line_num==lexer->tokenMap[tokenPos-1].line_num))tokenPos--;
    if(lexer->tokenMap[tokenPos].word_type=="CHARTK") {
        return true;
    }
    return false;
}

int SymbolTableHandler::get_StringConst(int tokenPos) const {
    string return_sentence = "StringConst not found\n";
    while(lexer->tokenMap[tokenPos].word_type!="STRCON" && lexer->tokenMap[tokenPos].word_type!= "SEMICN"&&
        (lexer->tokenMap[tokenPos].line_num==lexer->tokenMap[tokenPos+1].line_num))tokenPos++;
    if(lexer->tokenMap[tokenPos].word_type=="STRCON") {
        return tokenPos;
    }
    return -1;
}


void SymbolTableHandler::PrintSymbolTable(Node *node,FILE * fp) const {
    for(SymbolTable * symbol_table:this->symbolTableList) {
        // for(auto& pair:symbol_table->name2symbolTableEntry) {
        //     const std::string& name = pair.first;           // 键（变量名）
        //     SymbolTableEntry* entry = pair.second;          // 值（符号表条目指针）
        //     fprintf(fp,"%d %s %s\n",symbol_table->identifier,name.c_str(),entry->symbolTableEntryType2string.at(entry->type).c_str());
        // }
        for(const string& name:symbol_table->insertionOrder) {
            SymbolTableEntry* entry = symbol_table->name2symbolTableEntry[name];          // 值（符号表条目指针）
            fprintf(fp,"%d %s %s\n",symbol_table->identifier,name.c_str(),entry->symbolTableEntryType2string.at(entry->type).c_str());
        }
    }
}

void SymbolTableHandler::output(FILE * fp1) {
    std::map<QuadItemType, std::string> typeMap = QuadItemType2Name();
    fflush(stdout);
    for(SymbolTable * symbol_table:this->symbolTableList) {
        fflush(stdout);
        for(auto& pair:symbol_table->name2QuadItem) {
            fflush(stdout);
            const std::string& name = pair.first;           // 键（变量名）
            fflush(stdout);
            QuadItem * quad_item = pair.second;          // 值（四元式条目）
            fflush(stdout);
            fprintf(fp1,"%d %s type: %s  \n",symbol_table->identifier,name.c_str(),typeMap.find(quad_item->type)->second.c_str());
        }
    }
    fflush(stdout);
    //从Entries中进行输出
}

//1: 1 1 1
//2: 1 1 0
//3: 1 0 1
//4: 0 1 1
//5: 0 0 1
//6: 0 1 0
//7: 1 0 0
//8: 0 0 0
//'for' '(' [ForStmt] ';' [Cond] ';' [ForStmt] ')' Stmt
void SymbolTableHandler::Analyse_forStmt_cond(int tokenPos,int * ifCond) const {
    int temp = tokenPos;
    int first,second,third,forth;
    while(lexer->tokenMap[temp].word_type!="LPARENT")temp++;
    first = temp;
    temp++;
    while(lexer->tokenMap[temp].word_type!="SEMICN")temp++;
    second = temp;
    temp++;
    while(lexer->tokenMap[temp].word_type!="SEMICN")temp++;
    third = temp;
    temp++;
    while(lexer->tokenMap[temp].word_type!="RPARENT" )temp++;
    forth = temp;
    //printf("%d %d %d %d\n",first,second,third,forth);
    if(second-first!=1)ifCond[0]=1;
    if(third-second!=1)ifCond[1]=1;
    if(forth-third!=1)ifCond[2]=1;
}


/**
else if (first->is(Symbol::WHILETK)) {  // Stmt → 'while' '(' Cond ')' Stmt

    auto *whileStartLabel = new ICItemLabel(), *whileEndLabel = new ICItemLabel();
    icTranslator->translate_InsertLabel(whileStartLabel);

    auto *newWhileBlock = new WhileBlock(currentWhileBlock, whileStartLabel, whileEndLabel);
    currentWhileBlock->child = newWhileBlock;
    currentWhileBlock = newWhileBlock;

    ICItem *cond = new ICItemVar(IS_GLOBAL);
    this->check_Cond(stmt->getChildAt(2), cond);

    icTranslator->translate_Beqz(cond, whileEndLabel);  // beqz cond, whileEndLabel
    checkErrorNode(stmt->getChildAt(3));  // ErrorType::MissingRPARENT

    this->check_Stmt(stmt->getChildAt(4));

    icTranslator->translate_JumpLabel(currentWhileBlock->whileStartLabel);
    icTranslator->translate_InsertLabel(currentWhileBlock->whileEndLabel);

    currentWhileBlock = currentWhileBlock->parent;

}
**/