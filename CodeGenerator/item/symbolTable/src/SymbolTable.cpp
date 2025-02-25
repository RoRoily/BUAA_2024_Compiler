//
// Created by 郑慕函 on 24-10-17.
//
#include <iostream>
#include "../include/SymbolTable.h"
#include "../../tree/include/Node.h"


//新建符号表
SymbolTable::SymbolTable(SymbolTable *parent, bool isRoot,int count) : parent(parent), isRoot(isRoot) ,identifier(count){}

//检查当前表中是否有重名对象
bool SymbolTable::nameExistedInCurrentTable(Node *node) const {
    return this->name2symbolTableEntry.count(node->token->word) != 0;
}

//向父节点查询
bool SymbolTable::nameExistedInAllTables(Node *node) {
    SymbolTable *table = this;
    while (table != nullptr) {
        if (table->nameExistedInCurrentTable(node)) return true;
        table = table->parent;
    }
    return false;
}

QuadItem *SymbolTable::getQuadItemByNameFromAllTables(Node *node) {
    SymbolTable *table = this;
    while (table != nullptr) {
        if (table->nameExistedInCurrentTable(node)) {
            return table->name2QuadItem.find(node->token->word)->second;
        }
        table = table->parent;
    }
    return nullptr;
}

//递归查询（没写完）
bool SymbolTable::nameExistedInAllTables(SymbolTable * cur) {
    if(cur == nullptr)return false;
    else {
        return false;
    }
}


//根据名称从所有符号表中获取对应的符号表条目
SymbolTableEntry *SymbolTable::getEntryByNameFromAllTables(Node *node) {
    // 先调用 nameExistedInAllTables 保证有
    SymbolTable *table = this;
    while (table != nullptr) {
        if (table->nameExistedInCurrentTable(node)) {
            return table->name2symbolTableEntry.find(node->token->word)->second;
        }
        table = table->parent;
    }
    return nullptr;
}

//在指定符号表中添加条目
void SymbolTable::addEntry(const std::string& name, SymbolTableEntry *entry) {
    if(name2symbolTableEntry.count(name) != 0) {
        printf("Error in SymbolTable::addEntry : name2symbolTableEntry.count(name)!=0");
    }
    printf("success to add entry %s\n",name.c_str());
    this->name2symbolTableEntry.insert({name, entry});
    this->insertionOrder.push_back(name);
}

//添加子表
void SymbolTable::addChildTable(SymbolTable *child) {
    this->children.push_back(child);
}


//输出所有的变量
void SymbolTable::printAllNames() {
    static int i = 1;
    std::cout << "\nTimes for printing: " << i << "\n";
    for (auto& it : name2symbolTableEntry) {
        std::cout << it.first << " ";
    }
    i++;
    std::cout << std::endl;
}


void SymbolTable::addQuadItem(const std::string &name, QuadItem *icItem) {
    printf("SymbolTable::addQuadItem\n");
    icItem->Name = name;
    name2QuadItem.insert({name, icItem});
}


