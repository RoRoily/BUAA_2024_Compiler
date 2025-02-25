//
// Created by 郑慕函 on 24-12-1.
//
#include "../include/ForBlock.h"
ForBlock::ForBlock(ForBlock *parent, QuadItemLabel *start, QuadItemLabel*body,QuadItemLabel*bodyend,QuadItemLabel *end) :
        parent(parent),
        depth(parent->depth + 1),
        child(nullptr),
        forStartLabel(start),
        forBodyLabel(body),
        forBodyEndLabel(bodyend),
        forEndLabel(end) {}

bool ForBlock::inWhile() const {
    return depth;
}

ForBlock::ForBlock() :
        parent(nullptr),
        depth(0),
        child(nullptr),
        forStartLabel(nullptr),
        forBodyLabel(nullptr),
        forBodyEndLabel(nullptr),
        forEndLabel(nullptr) {}
