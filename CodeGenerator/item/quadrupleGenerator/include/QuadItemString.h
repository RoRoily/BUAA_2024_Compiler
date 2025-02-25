//
// Created by 郑慕函 on 24-11-12.
//

#ifndef QUADITEMSTRING_H
#define QUADITEMSTRING_H
#include <map>
#include <vector>
#include "QuadItem.h"
#include "QuadItemInt.h"

class QuadItemString : public QuadItem {
public:
    class StrItem {
    public:
        const bool isString;
        const bool isChar;
        const std::string *stringItem;
        const QuadItem *varItem;
        const int pureStringId; //ID

        //生成字符串ID
        static int generatePureStringId(bool isString) {
            static int pureStringId = 0;
            return ++pureStringId;
        }

        StrItem(const bool isString,const bool isChar, const std::string *stringItem, const QuadItem *intItem)
                : isString(isString), isChar(isChar),stringItem(stringItem),
                  varItem(intItem), pureStringId(generatePureStringId(isString)) {}

        ~StrItem() {
            delete stringItem;
            delete varItem;
        }

    };
    //存储所有打印内容
    std::vector<StrItem *> *stringItems;
    std::map<int, std::string *> *id2pureString;

    QuadItemString() : QuadItem(QuadItemType::String) {
        stringItems = new std::vector<StrItem *>;
        id2pureString = new std::map<int, std::string *>;
    }

    ~QuadItemString() override {
        for (const auto *pItem: *stringItems) delete pItem;
        delete stringItems;
        for (const auto &pItem: *id2pureString) delete pItem.second;
        delete id2pureString;
    }

    //FIXME: 分配的内存泄漏？
    void addIntItem(QuadItem *varItem) const {
        printf("addIntItem \n");
        auto *intType = new StrItem(false, false,nullptr, varItem);
        stringItems->push_back(intType);
    }

    void addIntItem(QuadItem *varItem,bool isChar) const {
        printf("addIntItem \n");
        auto *intType = new StrItem(false, true,nullptr, varItem);
        stringItems->push_back(intType);
    }

    void addCharItem(QuadItem *varItem) const {
        printf("addIntItem \n");
        auto *intType = new StrItem(false,true ,nullptr, varItem);
        stringItems->push_back(intType);
    }


    /**
     * @param s
     * @return pureStringId
     */
    int addStringItem(std::string *s) const {
        printf("addStringItem %s\n",s->c_str());
        auto *stringItem = new StrItem(true,false, s, nullptr);
        stringItems->push_back(stringItem);
        id2pureString->insert({stringItem->pureStringId, s});
        return stringItem->pureStringId;
    }

};
#endif //QUADITEMSTRING_H
