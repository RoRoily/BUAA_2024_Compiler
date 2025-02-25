//
// Created by 郑慕函 on 24-10-17.
//

#ifndef FUNCPARAM_H
#define FUNCPARAM_H
class FuncParam {
public:
    const int type;  // 0:int 1:char 2:int[] 3：char[]、
    std::string *name;

    explicit FuncParam(std::string *name,int type) :name(name),type(type) {}
};

#endif //FUNCPARAM_H
