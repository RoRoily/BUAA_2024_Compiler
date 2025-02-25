//
// Created by ROILYY on 24-10-9.
//

#include  "../include/Node.h"
#include "../../../include/GrammarItem.h"

void Node::addChild(Node *child){
  this->children.push_back(child);
}


void Node::setParent(Node *parent) {
  this->parent = parent;
}

std::vector<Node *> Node::getAllChildren() {
  return this->children;
}

//BlockItem,Decl,Btype不用输出
bool Node::needOutput() const {
  return !((!this->isLeaf)&&(this->grammarItem==GrammarItem::BlockItem || this->grammarItem==GrammarItem::Decl || this->grammarItem==GrammarItem::BType));
}


Node::Node(GrammarItem grammarItem, Node *parent, int depth)
        : grammarItem(grammarItem), parent(parent), depth(depth), isLeaf(false) {}

Node::Node(GrammarItem grammarItem, int depth)
        : grammarItem(grammarItem), parent(nullptr), depth(depth), isLeaf(false) {}



Node::Node(Word *token, int depth)
        : token(token), parent(nullptr), depth(depth), isLeaf(true) {}


Node::Node(Word *token, Node *parent, int depth)
        : token(token), parent(parent), depth(depth), isLeaf(true) {}

Node *Node::getFirstChild() {
    return this->children.front();
}

Node *Node::getLastChild() {
    return this->children.back();
}

unsigned long long Node::getChildNum() const {
//如果NODE_DEBUG宏被定义了则执行这段代码
#ifdef NODE_DEBUG
    std::cout << "\nIn Node::getChildrenNum()\nCurrent node: ";
    if (this->isErrorNode) {
        std::cout << "ErrorNode" << "\n";
    } else if (this->isToken()) {
        if (this->token->lineNumber == 31) {
            int a;
        }
        std::cout << "\"" << this->token->value << "\", line: " << this->token->lineNumber << "\n";
    } else {
        std::cout << grammarItem2string.find(this->grammarItem)->second << "\n";
    }
    std::cout << "children: \n\t";
    for (const Node *child: children) {
        std::cout << std::flush;
        if (child->isErrorNode) {
            std::cout << "ErrorNode" << "\n";
        } else if (child->isToken()) {
            if (this->token->lineNumber == 31) {
                int a;
            }
            std::cout << "\"" << child->token->value << "\", line: " << child->token->lineNumber << "\n\t";
        } else {
            std::cout << grammarItem2string.find(child->grammarItem)->second << "\n\t";
        }
    }
    std::cout << std::endl;
#endif
    return this->children.size();
}


Node *Node::getChildAt(int pos) {
    if(pos >= 0)return this->children[pos];
    else return this->children[children.size() + pos];
}

bool Node::equals(GrammarItem grammar_item) const {
    return this->grammarItem == grammar_item;
}
Node::~Node() {
    if (isLeaf) delete token;
    for (const auto child : children) {
        delete child;
    }
}