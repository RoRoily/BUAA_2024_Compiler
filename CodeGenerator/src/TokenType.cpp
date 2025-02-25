#include <map>
#include <string>

class TokenType {
public:
    TokenType();
    static TokenType* getTokenTypeInstance();
    static void deleteTokenTypeInstance();
    std::map<std::string, std::string> getTokenTypeMap();
    std::string getTokenType(const std::string& token);

private:
    static TokenType* g_pTokenType;
    std::map<std::string, std::string> tokenTypeMap;
};

// 在构造函数中初始化
TokenType::TokenType() {
    tokenTypeMap = {
        {"Ident", "IDENFR"}, {"IntConst", "INTCON"}, {"StringConst", "STRCON"},
        {"CharConst", "CHRCON"}, {"main", "MAINTK"}, {"const", "CONSTTK"},
        {"int", "INTTK"}, {"char", "CHARTK"}, {"break", "BREAKTK"},
        {"continue", "CONTINUETK"}, {"if", "IFTK"}, {"else", "ELSETK"},
        {"!", "NOT"}, {"&&", "AND"}, {"||", "OR"}, {"for", "FORTK"},
        {"getint", "GETINTTK"}, {"getchar", "GETCHARTK"},
        {"printf", "PRINTFTK"}, {"return", "RETURNTK"},
        {"+", "PLUS"}, {"-", "MINU"}, {"void", "VOIDTK"},
        {"*", "MULT"}, {"/", "DIV"}, {"%", "MOD"},
        {"<", "LSS"}, {"<=", "LEQ"}, {">", "GRE"},
        {">=", "GEQ"}, {"==", "EQL"}, {"!=", "NEQ"},
        {"=", "ASSIGN"}, {";", "SEMICN"}, {",", "COMMA"},
        {"(", "LPARENT"}, {")", "RPARENT"}, {"[", "LBRACK"},
        {"]", "RBRACK"}, {"{", "LBRACE"}, {"}", "RBRACE"}
    };
}

TokenType* TokenType::g_pTokenType = nullptr;

TokenType* TokenType::getTokenTypeInstance() {
    if (g_pTokenType == nullptr) {
        g_pTokenType = new TokenType();
    }
    return g_pTokenType;
}

void TokenType::deleteTokenTypeInstance() {
    delete g_pTokenType;
    g_pTokenType = nullptr;
}

std::map<std::string, std::string> TokenType::getTokenTypeMap() {
    return tokenTypeMap;
}

std::string TokenType::getTokenType(const std::string& token) {
    auto it = tokenTypeMap.find(token);
    if (it != tokenTypeMap.end()) {
        return it->second;
    }
    return ""; // 或者抛出异常
}

