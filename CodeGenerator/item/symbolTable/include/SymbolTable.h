//
// Created by 郑慕函 on 24-10-17.
//

#ifndef THIS_PROJECT_SYMBOL_TABLE_H
#define THIS_PROJECT_SYMBOL_TABLE_H

#include <map>
#include <list>
#include <vector>
#include "SymbolTableEntry.h"
#include "../../tree/include/Node.h"
#include "../../quadrupleGenerator/include/QuadItem.h"

class SymbolTable {
public:

    bool isRoot;
    int identifier; //编号
    SymbolTable *parent;
    std::vector<SymbolTable *> children;
    //将符号表名称 映射到 符号表条目指针
    std::map<std::string, SymbolTableEntry *> name2symbolTableEntry;
    //以便于按照插入顺序显示
    std::list<std::string> insertionOrder;

    std::map<std::string, QuadItem *> name2QuadItem;

    SymbolTable(SymbolTable *parent, bool isRoot, int count);

    //是否在符号表中存在
    bool nameExistedInCurrentTable(Node *node) const;

    bool nameExistedInAllTables(Node *node);

    bool nameExistedInAllTables(SymbolTable *cur);

    SymbolTableEntry *getEntryByNameFromAllTables(Node *node);

    void addEntry(const std::string&, SymbolTableEntry *);

    QuadItem *getQuadItemByNameFromAllTables(Node *node);

    void addChildTable(SymbolTable *child);

    //中间代码生成
    void addQuadItem(const std::string&, QuadItem *icItem);

    void printAllNames();
};


#endif //THIS_PROJECT_SYMBOL_TABLE_H

