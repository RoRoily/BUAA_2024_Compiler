//
// Created by Peng Xinyang on 24-9-20.
//

#include "../include/Lexer.h"
#include <map>
#include <string.h>
#include <unordered_map>

#include "../include/TokenType.h"
#include "../include/Tool.h"
enum CommentState { NONE, LINE_COMMENT, BLOCK_COMMENT };
int i=0;
char Char = ' ';
bool flag = true;
std::vector<int> arr = {100, 12, 21, 43};
TokenType* tokenTypeN = TokenType::getTokenTypeInstance();
map<string,string> tokenTypeMap = tokenTypeN->getTokenTypeMap();






Lexer*Lexer::lexerInstance = new Lexer();
void Lexer::setCharPtr(char *source) {
	charPtr = source;
}


Lexer *Lexer::getLexerInstance() {
    if (lexerInstance == nullptr) {
        lexerInstance = new Lexer();
    }
    return lexerInstance;
}
void Lexer::deleteLexerInstance() {
    if(lexerInstance) {
        delete lexerInstance;
        lexerInstance = nullptr;
    }
}

string Lexer::getToken() {
	if(flag)return token;
	return token;
}
Word Lexer::handleNext() {
	handle_next();
	Word word = Word(tokenType, token,lineCount);
	return word;
}
void Lexer::handleSource() {
	while(charPtr && *charPtr) {
		Word p = handleNext();
		if(!p.word_type.empty()&&!p.word.empty()) { //分别检查token类型和token字符串是否为空
			tokenMap[i++] = p;
		}
	}
	for(int i = 0; i<tokenMap.size();i++) {
		tokenMap[i].tokenPos = i;
	}
}

//输出正确的结果
void Lexer::printRight() {
	if(lineWrong.empty()) {
		FILE*fp = fopen("lexer.txt", "w");
		for(const auto&[fst,snd]:tokenMap) {
			fprintf(fp,"%s %s\n",snd.word_type.c_str(),snd.word.c_str());
		}
		fclose(fp);
	}
}
//输出错误的结果
void Lexer::printWrong() {
	if(!lineWrong.empty()) {
		FILE*fp = fopen("error.txt", "w");
	for(const auto &[fst, snd] : lineWrong) {
		fprintf(fp, "%d %c\n",fst,snd);
	}
	fclose(fp);
	}
}



//读字符过程
void Lexer::getChar() {
	Char = *charPtr++;
}
//清空token
void Lexer::clearToken() {
	string str="";
	token.swap(str);
	tokenType="";
}

//每次调用把Char字符与token字符数组中的字符串连接
void Lexer::catToken() {
	token += Char;
}
//字符指针回退一个
void Lexer::retract() {
	--charPtr;
}
//查找token是不是保留字
//有个小漏洞：变量名是Ident，IntConst等
int Lexer::reserver() {
	return (tokenTypeMap.count(token) || token == "Ident" || token == "IntConst" || token == "StringConst" || token == "CharConst") ? 1 : 0;
}
//将token数字字符串转成整数
long long Lexer::transNum(const string &token) {
	return stoll(token);
}
//错误处理
void Lexer::handleError() {
	lineWrong[lineCount] = 'a';
}
//---------------------------------------------------------
//封装的修改程序
// 判断是否是空白字符
bool Lexer::isWhitespace(char c) {
    return c == ' ' || c == '\t' || c == '\r';
}

// 处理标识符或保留字
void Lexer::handleIdentifierOrKeyword() {
    while (isLetterOrDigi()) {
        catToken();
        getChar();
    }
    retract();
    int resultValue = reserver(); //判断是否为保留字或标识符
	//token为当前处理的单词
    tokenType = (resultValue) ? tokenTypeMap[token] : tokenTypeMap["Ident"];
}

// 处理数字
void Lexer::handleNumber() {
    while (isDigit(Char)) {
        catToken();
        getChar();
    }
    retract();
    num = transNum(token);
    tokenType = tokenTypeMap["IntConst"];
}

// 处理字符串常量
void Lexer::handleString(char quote) {
    catToken();
    getChar();
    while (Char != quote || (*(charPtr - 2)) == '\\' && (*(charPtr - 3)) != '\\') {
        catToken();
        getChar();
    }
    catToken();
    getChar();
    retract();
    tokenType = (quote == '"') ? tokenTypeMap["StringConst"] : tokenTypeMap["CharConst"];
}

// 处理等号
void Lexer::handleEqualityOperator() {
    catToken();
    getChar();
    if (Char == '=') {
        catToken();
        tokenType = tokenTypeMap["=="];
    } else {
        retract();
        tokenType = tokenTypeMap["="];
    }
}

// 处理不等号
void Lexer::handleInequalityOperator() {
    catToken();
    getChar();
    if (Char != '=') {
    	retract();
    	tokenType = tokenTypeMap["!"];

    } else {
    	catToken();
    	tokenType = tokenTypeMap["!="];
    }
}

// 处理小于号
void Lexer::handleLessThanOperator() {
    catToken();
    getChar();
    if (Char != '=') {
    	retract();
    	tokenType = tokenTypeMap["<"];
    } else {
    	catToken();
    	tokenType = tokenTypeMap["<="];
    }
}

// 处理大于号
void Lexer::handleGreaterThanOperator() {
    catToken();
    getChar();
    if (Char == '=') {
        catToken();
        tokenType = tokenTypeMap[">="];
    } else {
        retract();
        tokenType = tokenTypeMap[">"];
    }
}

// 辅助函数：判断是否是字母开头
bool Lexer::isLetterStart(char c) {
	bool flagg = c == '_' || (c >= 'A' && c <= 'Z')||(c >= 'a' && c <= 'z') ;
    return flagg;
}

// 辅助函数：判断是否是数字
bool Lexer::isDigit(char c) {
	if (c >= '0' && c <= '9') {return true;}
	return false;
	//return c >= '0' && c <= '9';
}


// 处理逻辑运算符
bool Lexer::handleLogicalOperators() {
	if (Char == '&' || Char == '|') {
		char op = Char;
		catToken();
		getChar();
		if (Char == op) {
			catToken();
			tokenType = (op == '&') ? tokenTypeMap["&&"] : tokenTypeMap["||"];
			return true;
		}
		retract();
		handleError();
		addToken((op == '&') ? "&&" : "||");
	}
	return false;
}

// 处理注释
bool Lexer::handleComments() {
	if (Char != '/') {
		return false; // 没有注释处理
	}

	catToken();
	getChar();

	CommentState state = NONE;

	if (Char == '/') {
		state = LINE_COMMENT;
	} else if (Char == '*') {
		state = BLOCK_COMMENT;
	} else {
		retract();
		tokenType = tokenTypeMap["/"]; // 不是注释，当前单词为除号
		return true;
	}

	// 处理行注释
	if (state == LINE_COMMENT) {
		while (Char != '\n' && (*charPtr) != 0) {
			getChar();
		}
		if (Char == '\n') {
			retract(); // 处理换行
		}
	}
	// 处理块注释
	else if (state == BLOCK_COMMENT) {
		while (true) {
			while (Char != '*' && (*charPtr) != 0) {
				if (Char == '\n') {
					lineCount++;
				}
				getChar();
			}
			if (Char == '*') {
				getChar();
				if (Char == '/') {
					break; // 结束多行注释
				}
			}
		}
	}

	return true; // 注释处理完毕
}

// 添加到tokenMap
void Lexer::addToken(const std::string& tokenStr) {
	Word word = Word(tokenTypeMap[tokenStr], tokenStr, lineCount);
	tokenMap[i++] = word;
}


// 处理单个字符运算符
bool Lexer::handleSingleCharacterOperators() {
	const char* operators = "+-*%(){}[];,";
	unordered_map<char, std::string> operatorMap = {
		{'+', "+"}, {'-', "-"}, {'*', "*"}, {'%', "%"},
		{'(', "("}, {')', ")"}, {'{', "{"}, {'}', "}"},
		{'[', "["}, {']', "]"}, {',', ","}, {';', ";"}
	};

	if (strchr(operators, Char) == nullptr) {
		return false; // 没有处理到
	}

	catToken(); // 记录当前字符
	tokenType = tokenTypeMap[operatorMap[Char]]; // 获取对应的token类型
	return true; // 处理成功
}

//---------------------------------------------------------
//---------------------------
bool Lexer :: isLetterOrDigi() {
	bool flage =  (Char >= 'a' && Char <= 'z') || (Char >= 'A' && Char <= 'Z') || Char == '_'||(Char >= '0' && Char <= '9');
	return flage;
}

//词法分析程序
int Lexer::handle_next() {//返回类别码
	clearToken();
	getChar();//读入第一个字符
	TokenType* tokenTypeN = TokenType::getTokenTypeInstance();
	tokenTypeMap = tokenTypeN->getTokenTypeMap();
	// 跳过空白字符
	while (isWhitespace(Char)) {
		getChar();
	}
	if (Char == '\n') {
		lineCount++;
	}

	// 处理标识符或保留字
	if (isLetterStart(Char)) {
		handleIdentifierOrKeyword();
	}
	
	// 处理字符串常量（双引号）
	else if (Char == '\"') {
		handleString('"');
	}
	// 处理字符常量（单引号）
	else if (Char == '\'') {
		handleString('\'');
	}
	// 处理数字
	else if (isDigit(Char)) {
		handleNumber();
	}
	// 处理运算符
	else if (Char == '!') {
		handleInequalityOperator();
	}
	else if (Char == '=') {
		handleEqualityOperator();
	}
	else if (Char == '>') {
		handleGreaterThanOperator();
	}
	else if (Char == '<') {
		handleLessThanOperator();
	}
	// 处理注释
	else if (handleComments()) {
		return 0;
	}
	//------------------------------------------------------
	//处理逻辑运算符
	//------------------------------------------------------
	else if (handleLogicalOperators()) {
		return 0;
	}

	//-------------------------------------------------------------------
	//单分界符的情况
	//---------------------------------------------------------------

	// 处理运算符和符号
	else if (handleSingleCharacterOperators()) {
		return 0;
	}
	//----------------------------------------------------
	//处理//和/
	//----------------------------------------------------

	
	return 0;
}