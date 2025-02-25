//
// Created by ROILYY on 24-12-27.
//
#include "../include/WhileBlock.h"

WhileBlock::WhileBlock(WhileBlock *parent, QuadItemLabel *start, QuadItemLabel *end) :
        parent(parent),
        depth(parent->depth + 1),
        child(nullptr),
        whileStartLabel(start),
        whileEndLabel(end) {}

bool WhileBlock::inWhile() const {
    return depth;
}

WhileBlock::WhileBlock() :
        parent(nullptr),
        depth(0),
        child(nullptr),
        whileStartLabel(nullptr),
        whileEndLabel(nullptr) {}


