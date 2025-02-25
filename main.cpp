#include "fstream"

#include "./CodeGenerator/include/FileProcess.h"
#include "./CodeGenerator/include/Lexer.h"
#include "CodeGenerator/item/tree/include/Node.h"
#include<iostream>

#include "CodeGenerator/include/Parser.h"
#include "CodeGenerator/include/TokenType.h"
#include "./CodeGenerator/handleProcess/include/SymbolTableHandler.h"
#include "./CodeGenerator/handleProcess/include/ICGenerator.h"
#include "CodeGenerator/handleProcess/include/MipsTranslator.h"
std::ofstream mipsOutput("./mips.txt");

using namespace std;
string source;

void outputAll(Node * node,FILE *fp) {
    if(!node -> isLeaf) {
        for(auto child: node -> getAllChildren()) {
            //printf("childrensize: %lu\n",node->getAllChildren().size());
            outputAll(child,fp);
            //对grammarItem进行输出
        }
        if(node->needOutput())fprintf(fp,"<%s>\n",grammarItemToString(node->grammarItem).c_str());
      return;
    }
    //对token进行输出
    fprintf(fp,"%s %s\n",node->token->word_type.c_str(),node->token->word.c_str());
    }
    //输出Token
    //if(node ->needOutput()) {
        //fprintf(fp,"%s %s",node->token->word_type.c_str(),node->token->word.c_str());
        //printf("oneleaf %s %s\n",node->token->word_type.c_str(),node->token->word.c_str());
    //}





int main() {
    int cont = 0;
    source = FileProcess::readFileContent("testfile.txt");
    FILE *fp = fopen("parser.txt","w"); //输出目标程序
    FILE *fp1 = fopen("symbol.txt","w"); //
    FILE *fp2 = fopen("lexer.txt","w");
    FILE *fp3 = fopen("icCode.txt","w");
    FILE *fp4 = fopen("icEntry.txt","w");
    TokenType* tokenTypeMain = TokenType::getTokenTypeInstance();
    Lexer *lexer = Lexer::getLexerInstance(); //生成词法分析实例
    Parser *parser = Parser::getParserInstance(); //生成词法分析程序
    //词法分析
    lexer->setCharPtr(const_cast<char *>(source.c_str()));
    lexer->handleSource(); //完全处理所有单词
    //输出
     for(const auto&[fst,snd]:lexer->tokenMap) {
         fprintf(fp2,"%s %s\n",snd.word_type.c_str(),snd.word.c_str());
         cont++;
     }
    for(const auto&[fst,snd]:lexer->tokenMap) {
        printf("%d %s %s\n",snd.tokenPos,snd.word_type.c_str(),snd.word.c_str());
        cont++;
    }
    printf("finish handle lexer\n");

    //语法分析
    parser->lexer = lexer;
    Node * root = parser -> parse();
    //输出语法分析结果
    outputAll(root,fp);

    //语义分析

    printf("Start analyse Symbol table\n");
    SymbolTableHandler * symbol_table_handler = new SymbolTableHandler(root);
    printf("finish build symbolTable handler\n");
    symbol_table_handler->lexer = lexer;
    //symbol_table_handler->root =  root;
    printf("finish assignment to lexer & root\n");
    symbol_table_handler->buildTable();
    printf("finish analysis Symbol table\n");

    //输出符号表结果
    symbol_table_handler->PrintSymbolTable(root,fp1);
    ICGenerator *icGenerator = symbol_table_handler->icGenerator;
    //中间代码生成
    printf("Start Intermediate code generate\n");
    //symbol_table_handler->output(fp3);
    icGenerator->entryOutput(fp4);
    //Mips目标代码生成
    printf("Start Mips generate\n");
    auto *mipsTranslator = new MipsTranslator(icGenerator);
    printf("generate mipsTranslator\n");
    mipsTranslator->translate();
    //mipsTranslator->StackPrint();
    //mipsTranslator->TagPrint();
    //if(lexer->lineWrong.empty())outputAll(root,fp);
    lexer->printWrong();
    //lexer->printRight();
    fclose(fp); //关闭文件
    fclose(fp1);
    fclose(fp2);
    fclose(fp3);
    Lexer::deleteLexerInstance();
    TokenType::deleteTokenTypeInstance();
    printf("Congratulations! Run Pass!\n");
    return 0;
}
