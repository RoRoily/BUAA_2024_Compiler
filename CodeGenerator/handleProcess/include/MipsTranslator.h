//
// Created by 郑慕函 on 24-11-27.
//

#ifndef MIPSTRANS_H
#define MIPSTRANS_H
#ifndef MIPS_TRANSLATOR_H
#define MIPS_TRANSLATOR_H


#include "ICGenerator.h"
#include "../../item/mipsGenerator/include/Reg.h"

class MipsTranslator {
public:
    const int tempStackAddressBase = 268500992;  // 0x10010000
    int tempStackAddressTop = 268500992;
    int tempFuncStackOffsetTop;
    int tempFuncStackOffsetBase = 0;
    /****
     * 强制类型转换用
     */
    std::vector <std::string > tagTable;
    /**
     * main函数专用：存局部变量
     */
    std::map<int, int> localVarId2mem;  // id 为负

    std::map<int, int> tempVarId2mem;  // id 为正

    std::map<int, int> localArrayId2mem;  // id 为正

    std::map<int, int> tempArrayId2mem;  // id 为负

    /**
     * 自定义函数专用：存局部变量, offset 相对于当前函数栈的 $sp
     */
    std::map<int, int> localVarId2offset;  // id 为负

    std::map<int, int> tempVarId2offset;  // id 为正

    std::map<int, int> localArrayId2offset;  // id 为正

    std::map<int, int> tempArrayId2offset;  // id 为负

    //---------------------

    std::map<Reg, bool> regUsage;

    std::map<Reg, int> reg2id;

    std::map<int, int> funcFVarParamId2offset;

    std::map<int, int> funcFArrayParamId2offset;

    ICGenerator *icTranslator;

    explicit MipsTranslator(ICGenerator *icTranslator);

    void translate();

    void translate_FuncDef(QuadItemFunc *func);

    void translate_GlobalVarOrArrayDef(QuadEntry *);

    void pushParams(const std::vector<QuadItem *> *params);

    // recursiveFormerOffset: 当在递归调用的压栈过程，从 $sp lw时要加1000
    void lw(Reg reg, QuadItemInt *var, bool whenPushingParamsRecursively = false);

    void lw(Reg dst, int offset, Reg base);

    void sw(Reg reg, QuadItemInt *dst);

    void sw(Reg src, int offset, Reg base);

    void li(Reg reg, int imm);

    void move(Reg dst, Reg src);

    void pushTempReg();

    void popTempReg();

    void addiu(Reg dst, Reg src, int i);

    void jal(const QuadItemFunc *calledFunc);

    void j(QuadItemLabel *label);

    void beqz(Reg cond, QuadItemLabel *label);

    void bnez(Reg cond, QuadItemLabel *label);

    void exit();

    void getint(QuadItemInt *dst);

    void getchar(QuadItemChar *dst);

    void printStr(int strId);

    void la(Reg reg, const std::string &label);

    static std::string strId2label(int strId);

    void syscall();

    void printInt(QuadItemInt *var);

    void printChar(QuadItemChar *var);

    void addu(Reg rd, Reg rs, Reg rt);

    void subu(Reg rd, Reg rs, Reg rt);

    void mul(Reg rd, Reg rs, Reg rt);

    void mul(Reg dst, Reg srcReg, int srcNum);

    void div(Reg rs, Reg rt);

    void mfhi(Reg reg);

    void mflo(Reg reg);

    void jr();

    void sll(Reg dst, Reg src, int bits);

    bool isFuncFParam(QuadItemInt *var);

    bool isFuncFParam(QuadItemChar *var);

    bool isFuncFParam(QuadItemIntArray *array);

    bool isFuncFParam(QuadItemCharArray *array);

    void sne(Reg rd, Reg rs, Reg rt);

    void seq(Reg rd, Reg rs, Reg rt);

    void sle(Reg rd, Reg rs, Reg rt);

    void slt(Reg rd, Reg rs, Reg rt);

    void sge(Reg rd, Reg rs, Reg rt);

    void sgt(Reg rd, Reg rs, Reg rt);

    void _or(Reg rd, Reg rs, Reg rt);

    void _and(Reg rd, Reg rs, Reg rt);

    void insertLabel(QuadItemLabel *label);

    void clearLocalAndTempInFunc();

    void sw(Reg reg, QuadItemChar *dst);

    void lb(Reg reg, QuadItemChar *var, bool whenPushingParamsRecursively =false);

    void lb(Reg dst, int offset, Reg base);

    void lw(Reg reg, QuadItemChar *var, bool whenPushingParamsRecursively = false);

    void StackPrint();

    void TagPrint();

    std::string toStringWithCheck2(int VarId);

    std::string toStringWithCheck(int VarId, bool isTemp, bool isGlobal, bool isConst);

    std::string toStringWithCheck_Array(int arrayId, bool isTemp, bool isGlobal, bool isConst);

    bool contains(const std::vector<std::string> &vec, const std::string *element);
};


#endif //MIPS_TRANSLATOR_H

#endif //MIPSTRANS_H
