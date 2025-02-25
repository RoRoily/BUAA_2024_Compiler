//
// Created by ROILYY on 24-10-27.
//

#ifndef CONSTVALUE_H
#define CONSTVALUE_H

#include <vector>
#include <string>

union Value {
    int intVar;
    int *intArray;
    char charVar;
    std::string *charArray;
};


class ConstValue {
public:
    int dimension;
    int d;
    int intVar;
    int *intArray;
    char charVar;
    std::string *charArray ;
    ConstValue() : dimension(), d(),intVar(), intArray(nullptr), charVar() {
        charArray = new std::string("");
     }
};

#endif //CONSTVALUE_H
