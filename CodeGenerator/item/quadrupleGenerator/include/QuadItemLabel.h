//
// Created by 郑慕函 on 24-11-11.
//

#ifndef QUADITEMLABEL_H
#define QUADITEMLABEL_H
#include "QuadItem.h"

//中间代码条目的唯一标签
class QuadItemLabel : public QuadItem {
    static int generateLabelId() {
        static int i = 0;
        return ++i;
    }

public:
    const int labelId;

    QuadItemLabel() : QuadItem(QuadItemType::Label), labelId(generateLabelId()) {}

    std::string toString() const {
        return "Label_" + std::to_string(abs(labelId));
    }

};
#endif //QUADITEMLABEL_H
