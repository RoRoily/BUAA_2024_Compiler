//
// Created by ROILYY on 24-12-27.
//

#ifndef WHILEBLOCK_H
#define WHILEBLOCK_H
#include "../item/quadrupleGenerator/include/QuadItemLabel.h"

class WhileBlock {
public:
    WhileBlock *parent;
    WhileBlock *child;
    const int depth;

    QuadItemLabel *whileStartLabel;
    QuadItemLabel *whileEndLabel;

    WhileBlock(WhileBlock *parent, QuadItemLabel *start, QuadItemLabel *end);

    WhileBlock();  // 最开始的 root，设置 depth = 0，表示不在 while 里

    bool inWhile() const;
};
#endif //WHILEBLOCK_H
