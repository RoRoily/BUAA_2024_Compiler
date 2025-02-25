//
// Created by 郑慕函 on 24-10-17.
//

#ifndef INTARRAY_H
#define INTARRAY_H

//整数型变量数组
class IntArray {
public:
    int dim; //-1表示为形参
    int *values;

    explicit IntArray(int dim):dim(dim) {
        values = new int[dim + 1];
    }

    ~IntArray() {
        delete[] values;
        delete values;
        values = nullptr;
    }

};
#endif //INTARRAY_H
