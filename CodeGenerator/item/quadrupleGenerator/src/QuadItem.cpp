//
// Created by 郑慕函 on 24-12-9.
//
#include "../include/QuadItem.h"
QuadItem::QuadItem(QuadItemType type)
        : type(type),
          referenceType(ReferenceType::Unset),
          reference(nullptr),
          typeChanged(false),
          newQuadItem(nullptr) {}

QuadItem::~QuadItem() {
    delete newQuadItem;
    delete reference;
    delete array1_var_index;
}