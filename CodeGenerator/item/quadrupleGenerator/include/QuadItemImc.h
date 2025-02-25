//
// Created by 郑慕函 on 24-11-20.
//

#ifndef QUADITEMIMC_H
#define QUADITEMIMC_H
#include "QuadItem.h"
class QuadItemImc : public QuadItem {
public:
    char value;  // 立即数

    explicit QuadItemImc(char value = '\0') : QuadItem(QuadItemType::Imc), value(value) {}
};

#endif //QUADITEMIMC_H

