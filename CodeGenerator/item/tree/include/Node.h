//
// Created by ROILYY on 24-10-9.
//

#ifndef NODE_H
#define NODE_H
#include <vector>
#include "../../../include/Lexer.h"

#include "../../../include/GrammarItem.h"

class Node {
    std::vector<Node*> children;
    Node* parent;
    int depth;



public:
    //输出结果相关
    Word *token;

    GrammarItem grammarItem;

    bool isLeaf;

    std::vector<Node *> getAllChildren();

    bool needOutput() const;

    //构建语法树相关
    void addChild(Node* child);

    void setParent(Node* parent);

    Node(GrammarItem grammarItem,Node * parent,int depth);

    Node(GrammarItem grammarItem,int depth);

    Node(Word *word,Node *parent,int depth);

    Node(Word *word,int depth);

    Node * getFirstChild();

    Node * getLastChild();

    unsigned long long getChildNum() const ;

    bool equals(GrammarItem grammar_item) const;

    ~Node();

    Node * getChildAt(int pos);

};
#endif //NODE_H
