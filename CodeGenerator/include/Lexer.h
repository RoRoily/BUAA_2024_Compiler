//
// Created by Peng Xinyang on 24-9-20.
//

#ifndef LEXER_H
#define LEXER_H
#include <algorithm>
#include <map>
#include <sstream>
#include <string>
#include <utility>
using namespace std;
struct Word {
    std::string word;      // 单词名字 token的名字
    int line_num;         // 行号
    std::string word_type; // 单词类型 token的值
    int tokenPos;

    // 默认构造函数
    Word() : word(""), line_num(0), word_type("") {}

    // 带参数构造函数
    Word(std::string word_type, std::string word, int line_num)
        : word(std::move(word)), word_type(std::move(word_type)), line_num(line_num) {}

    // 复制构造函数
    Word(const Word& other)
        : word(other.word), line_num(other.line_num), word_type(other.word_type) {}

    // 赋值运算符
    Word& operator=(const Word& other) {
        if (this != &other) {
            word = other.word;
            line_num = other.line_num;
            word_type = other.word_type;
        }
        return *this;
    }

    // 转换为字符串的表示
    std::string toString() const {
        std::ostringstream oss;
        oss << "Word: " << word << ", Type: " << word_type << ", Line: " << line_num;
        return oss.str();
    }

    // 生成哈希值
    std::size_t hash() const {
        return std::hash<std::string>()(word) ^ std::hash<std::string>()(word_type) ^ std::hash<int>()(line_num);
    }

    // 从字符串创建 Word 对象
    static Word fromString(const std::string& str) {
        std::istringstream iss(str);
        std::string word, word_type;
        int line_num;
        iss >> word >> word_type >> line_num; // 假设格式为 "word type line_num"
        return Word(word_type, word, line_num);
    }

    // 比较两个 Word 对象
    bool operator==(const Word& other) const {
        return word == other.word && line_num == other.line_num && word_type == other.word_type;
    }

    // 重载输出运算符
    friend std::ostream& operator<<(std::ostream& os, const Word& w) {
        os << w.toString();
        return os;
    }
};

class Lexer {
public:
    //行号+错误码
    map<int,char> lineWrong;
    static Lexer* getLexerInstance();
    static void deleteLexerInstance();

    map<int,Word> tokenMap;
    //初始化，让指针指向源代码source
    void setCharPtr(char*source);
    //处理下一个单词
    Word handleNext();
    //处理的具体过程
    int handle_next();
    //完全处理所有的单词
    void handleSource();
    //获取读取的单词
    string getToken();
    //输出正确的内容
    void printRight();
    //输出错误的内容
    void printWrong();
    //错误处理
    void lexerError();

    bool isWhitespace(char c);

    void handleIdentifierOrKeyword();

    void handleNumber();

    void handleString(char quote);

    void handleEqualityOperator();

    void handleInequalityOperator();

    void handleLessThanOperator();

    void handleGreaterThanOperator();

    bool isLetterStart(char c);

    bool isDigit(char c);

    bool handleLogicalOperators();

    bool handleComments();

    void addToken(const std::string &tokenStr);

    bool handleOperators();

    bool handleSingleCharacterOperators();

    static bool isLetterOrDigi();

private:
    static Lexer* lexerInstance;
    //存储单词类别和单词值


    string token;//当前处理的单词
    //char Char = ' ';//存取当前读进的字符
    long long num = 0;//存入当前读入的整型变量
    char* charPtr = nullptr;//字符指针
    int lineCount = 1;//行数
    string tokenType;



    //读字符过程
    void getChar();
    //清空token
    void clearToken();
    //判断是否为空格
    void catToken();
    //字符指针回退一个
    void retract();
    //查找token是不是保留字
    //有个小漏洞：变量名是Ident，IntConst等
    int reserver();
    //将token数字字符串转成整数
    long long transNum(const string &token);

    void handleError();
};



#endif //LEXER_H
