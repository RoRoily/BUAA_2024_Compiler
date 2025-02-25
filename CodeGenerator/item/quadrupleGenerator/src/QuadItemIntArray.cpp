//
// Created by 郑慕函 on 24-11-11.
//
#include "../include/QuadItemIntArray.h"

int QuadItemIntArray::generateArrayId() {
    static int i = 0;
    return ++Global_Count_For_Array;
    return ++i;
}

int QuadItemIntArray::generateTempArrayId() {
    static int i = 0;
    return --Temp_Global_Count_For_Array;
    return ++i;
}

/**
 *
 * @param originalName
 * @param symbolTableEntry
 * @param isGlobal
 * @param isConst
 * @param length
 * @param value
 */
QuadItemIntArray::QuadItemIntArray(const std::string *originalName,
                         SymbolTableEntry *symbolTableEntry, bool isGlobal,
                         bool isConst, int length, int *value) :
        QuadItem(QuadItemType::IntArray),
        isConst(isConst),
        value(value),
        length(length),
        isGlobal(isGlobal),
        arrayId(generateArrayId()),
        tempArrayId(-1),
        symbolTableEntry(symbolTableEntry),
        originalName(originalName),
        isTemp(false),
        itemsToInitArray(nullptr) {}

/**
 *
 * @param isGlobal
 * @param value
 */
QuadItemIntArray::QuadItemIntArray(bool isGlobal, int *value) :
        QuadItem(QuadItemType::IntArray),
        isConst(false),
        value(value),
        length(-1),
        isGlobal(isGlobal),
        arrayId(-1),
        tempArrayId(generateTempArrayId()),
        symbolTableEntry(nullptr),
        originalName(nullptr),
        isTemp(true) {
    if (!isGlobal) {
        // 初始化非全局变量的数组才会用到，数组每一个元素都需要通过临时变量来赋值
        itemsToInitArray = new std::vector<QuadItemInt *>;
    } else {
        itemsToInitArray = nullptr;
    }
}

QuadItemIntArray::~QuadItemIntArray() {
    delete[] value;
    for (const auto *pItem: *itemsToInitArray) delete pItem;
    delete itemsToInitArray;
}

std::string QuadItemIntArray::toString() const {
    if (isTemp) {
        return "temp_Iarr" + std::to_string(abs(tempArrayId));
    } else {
        if (isGlobal) {
            if (isConst) {
                return "global_const_Iarr" + std::to_string(abs(arrayId));
            } else {
                return "global_Iarr" + std::to_string(abs(arrayId));
            }
        } else {
            if (isConst) {
                return "local_const_Iarr" + std::to_string(abs(arrayId));
            } else {
                return "local_Iarr" + std::to_string(abs(arrayId));
            }
        }
    }
}

void QuadItemIntArray::setOriginType(int d, int length1, int length2) {
    originType = OriginType(d, length1, length2);
}
