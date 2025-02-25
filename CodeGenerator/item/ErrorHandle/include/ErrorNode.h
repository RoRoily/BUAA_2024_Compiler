//
// Created by ROILYY on 24-10-27.
//

#ifndef ERRORNODE_H
#define ERRORNODE_H
#include <string>
#include "../../tree/include/Node.h"
#include "ErrorType.h"

class ErrorNode : public Node {
public:
    const ErrorType errorType;
    const int lineNum;  // 这个行号为要求输出的行号

    ErrorNode(ErrorType errorType, int lineNum)
            : Node(), errorType(errorType), lineNum(lineNum) {}

    std::string error() const {
        return errorType2string.find(this->errorType)->second;
    }
};

#endif //ERRORNODE_H
