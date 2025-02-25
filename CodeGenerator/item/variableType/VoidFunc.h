//
// Created by 郑慕函 on 24-10-17.
//

#ifndef VOIDFUNC_H
#define VOIDFUNC_H
#include "./FuncParam.h"
#include <vector>

/*
 * 当 FuncFParam 为数组定义时，其第一维的长度省去（用方括号[ ]表示），而后面的各维则需要用
 * 表达式指明长度，长度是常量。
 */

/*
 * 四种类型参数 param: func(int param,char param, int param[], char param[])
 */

//void函数 并管理参数
class VoidFunc {
public:
    //指向函数参数的指针
    std::vector<FuncParam *>* params;


    VoidFunc() {
        params = new std::vector<FuncParam *>();
    }

    ~VoidFunc() {
        delete params;
    }

    void addIntParam(std::string * name) {
        auto *param = new FuncParam(name,0);
        params->push_back(param);
    }

    void addCharParam(std::string * name) {
        auto *param = new FuncParam(name,1);
        params->push_back(param);
    }

    void addIntArrayParam(std::string * name) {
        auto *param = new FuncParam(name,2);
        params->push_back(param);
    }


    void addCharArrayParam(std::string * name) {
        auto *param = new FuncParam(name,3);
        params->push_back(param);
    }
};
#endif //VOIDFUNC_H
