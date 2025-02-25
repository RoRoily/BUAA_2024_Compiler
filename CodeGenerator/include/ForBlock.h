//
// Created by 郑慕函 on 24-12-1.
//

#ifndef FORBLOCK_H
#define FORBLOCK_H


#include "../item/quadrupleGenerator/include/QuadItemLabel.h"
class ForBlock {
public:
    ForBlock *parent;
    ForBlock *child;
    const int depth;

    QuadItemLabel *forStartLabel;
    QuadItemLabel *forBodyLabel;
    QuadItemLabel *forBodyEndLabel;
    QuadItemLabel *forEndLabel;

    ForBlock(ForBlock *parent, QuadItemLabel *start, QuadItemLabel*body,QuadItemLabel*bodyend,QuadItemLabel *end);

    ForBlock();  // 最开始的 root，设置 depth = 0，表示不在 while 里

    bool inWhile() const;
};



#endif //FORBLOCK_H
