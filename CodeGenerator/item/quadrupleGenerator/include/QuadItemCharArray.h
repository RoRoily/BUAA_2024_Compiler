//
// Created by 郑慕函 on 24-11-19.
//

#ifndef QUADITEMCHARARRAY_H
#define QUADITEMCHARARRAY_H
#include <vector>
#include "QuadItem.h"
#include "QuadItemLabel.h"
#include "QuadItemChar.h"
#include "../../symbolTable/include/SymbolTableEntry.h"

#include "string.h"

struct OriginType {
    int d;
    int length1;
    int length2;

    OriginType() = default;

    OriginType(int d, int length1, int length2)
            : d(d), length1(length1), length2(length2) {}
};

class QuadItemCharArray : public QuadItem {
private:
    static int generateArrayId();

    static int generateTempArrayId();

public:
    /* 用于存储临时变量 */
    const bool isTemp;
    const int tempArrayId;
    std::vector<QuadItemChar *> *itemsToInitArray;  // 只有给数组赋值时会用到
    /* 用于非临时变量 */
    const std::string *originalName;
    const SymbolTableEntry *symbolTableEntry;
    const bool isGlobal;
    const bool isConst;
    const int arrayId;
    std::string value;  // const 特有
    int length;  // 一维数组的长度
    OriginType originType;

    QuadItemCharArray(const std::string *originalName, SymbolTableEntry *symbolTableEntry,
                bool isGlobal, bool isConst, int length = 0, string value = "");

    explicit QuadItemCharArray(bool isGlobal, string value = "");


    std::string toString() const;

    void setOriginType(int d, int length1, int length2);

    ~QuadItemCharArray();
};
#endif //QUADITEMCHARARRAY_H
