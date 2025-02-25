//
// Created by 郑慕函 on 24-10-17.
//

#ifndef CONSTCHARARRAY_H
#define CONSTCHARARRAY_H
class ConstCharArray {
public:
    const int d;  // 维度
    const string * values;

    explicit ConstCharArray(int d, const string * values)
            : d(d), values(values) {}

    ~ConstCharArray() {
        delete values;
    }

};
#endif //CONSTCHARARRAY_H
