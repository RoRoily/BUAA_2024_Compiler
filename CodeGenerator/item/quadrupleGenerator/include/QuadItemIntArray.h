//
// Created by 郑慕函 on 24-11-11.
//

#ifndef QUADITEMARRAY_H
#define QUADITEMARRAY_H
#include <vector>
#include "QuadItem.h"
#include "QuadItemLabel.h"
#include "QuadItemInt.h"
#include "../../symbolTable/include/SymbolTableEntry.h"

#include "QuadItemCharArray.h"
inline extern int Global_Count_For_Array = 1;
inline extern int Temp_Global_Count_For_Array = -1;
class QuadItemIntArray : public QuadItem {
private:
    static int generateArrayId();

    static int generateTempArrayId();

public:
    /* 用于存储临时变量 */
    const bool isTemp;
    const int tempArrayId;
    std::vector<QuadItemInt *> *itemsToInitArray;  // 只有给数组赋值时会用到
    /* 用于非临时变量 */
    const std::string *originalName;
    const SymbolTableEntry *symbolTableEntry;
    const bool isGlobal;
    const bool isConst;
    const int arrayId;
    int *value;  // const 特有
    int length;  // 一维数组的长度  TODO: 注意，二维数组全部转成一维！
    OriginType originType;

    QuadItemIntArray(const std::string *originalName, SymbolTableEntry *symbolTableEntry,
                bool isGlobal, bool isConst, int length = 0, int *value = nullptr);

    explicit QuadItemIntArray(bool isGlobal, int *value = nullptr);

    ~QuadItemIntArray() override;

    std::string toString() const;

    void setOriginType(int d, int length1, int length2);
};
#endif //QUADITEMARRAY_H
