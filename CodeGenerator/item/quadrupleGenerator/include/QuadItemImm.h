//
// Created by 郑慕函 on 24-11-12.
//

#ifndef QUADIMM_H
#define QUADIMM_H

#include "QuadItem.h"

class QuadItemImm : public QuadItem {
public:
    int value;  // 立即数

    explicit QuadItemImm(int value = 0) : QuadItem(QuadItemType::Imm), value(value) {}
};

#endif //QUADIMM_H
