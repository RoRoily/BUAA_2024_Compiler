//
// Created by 郑慕函 on 24-10-17.
//

#ifndef CHARFUNC_H
#define CHARFUNC_H
#include "FuncParam.h"
//void函数 并管理参数
class CharFunc {
public:
    //指向函数参数的指针
    std::vector<FuncParam *>* params;


    CharFunc() {
        params = new std::vector<FuncParam *>();
    }

    ~CharFunc() {
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

#endif //CHARFUNC_H
