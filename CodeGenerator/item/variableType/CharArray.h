//
// Created by 郑慕函 on 24-10-17.
//

#ifndef CHARARRAY_H
#define CHARARRAY_H
class CharArray {
public:
    int dim ;
    char * values;

    explicit CharArray(int dim):dim(dim) {
        values = new char[dim + 1];
    }

    ~CharArray() {
        delete[] values;
        delete values;
        values = nullptr;
    }

};
#endif //CHARARRAY_H
