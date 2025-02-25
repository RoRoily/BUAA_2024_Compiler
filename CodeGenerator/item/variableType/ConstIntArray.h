//
// Created by 郑慕函 on 24-10-17.
//

#ifndef CONSTINTARRAY_H
#define CONSTINTARRAY_H
class ConstIntArray {
public:
    const int d;  // 维度
    int *values;

    explicit ConstIntArray(int d, int *values)
            : d(d), values(values) {}

    ~ConstIntArray() {
        delete values;
    }
};
#endif //CONSTINTARRAY_H


