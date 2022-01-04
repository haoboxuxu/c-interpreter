//
//  main.cpp
//  interpreter
//
//  Created by 徐浩博 on 2022/1/3.
//

/*
 +------------------+
 |    stack   |     |      high address
 |    ...     v     |
 |                  |
 |                  |
 |                  |
 |                  |
 |    ...     ^     |
 |    heap    |     |
 +------------------+
 | bss  segment     |
 +------------------+
 | data segment     |
 +------------------+
 | text segment     |      low address
 +------------------+
 */

#include <iostream>
#include <memory>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
using namespace std;

#define armint long long // arm / x86_64 fix

armint token; //当前token
char *src, *old_src; //被编译代码的指针
armint poolsize; //代码段/数据段/栈段的大小
armint line; //当前行

armint *text_seg; //代码段
armint *old_text_seg; //代码段
armint *stack_seg; //栈段
char *data_seg; //数据段（只存放字符串）

/*
 pc: 计数器，下一指令
 sp: 栈顶，push时sp--
 bp: 基址指针，指向栈
 ax: 通用寄存器，存指令执行后的结果
 */
armint *pc, *bp, *sp, ax, cycle; //寄存器


// 指令集
enum {
    LEA,   // sub_func_call.txt
    IMM,   // IMM <num> 将 <num> 放入寄存器 ax 中
    JMP,   //
    CALL,  // CALL<addr>与RET指令，跳转到地址为<addr>的子函数，RET从子函数中返回
    JZ,    // ax=0则pc跳
    JNZ,   // ax!=0则pc跳
    ENT,   // ENT<size>，保存当前的栈指针，同时在栈上保留一定的空间，用以存放局部变量
    ADJ,   // ADJ<size> ‘remove arguments from frame’。在将调用子函数时压入栈中的数据清除
    LEV,   // 替代RET
    LI,    // LI 将对应地址中的整数载入 ax 中，要求 ax 中存放地址
    LC,    // LC 将对应地址中的字符载入 ax 中，要求 ax 中存放地址
    SI,    // SI 将 ax 中的数据作为整数存放入地址中，要求栈顶存放地址
    SC,    // SC 将 ax 中的数据作为字符存放入地址中，要求栈顶存放地址
    PUSH,  //
    OR,    //
    XOR,   //
    AND,   //
    EQ,    //
    NE,    //
    LT,    //
    GT,    //
    LE,    //
    GE,    //
    SHL,   //
    SHR,   //
    ADD,   //
    SUB,   //
    MUL,   //
    DIV,   //
    MOD,   //
    OPEN,  //
    READ,  //
    CLOS,  //
    PRTF,  //
    MALC,  //
    MSET,  //
    MCMP,  //
    EXIT   //
};

//词法分析，获取下一个token，将自动忽略空白字符
void next() {
    token = *src++;
}

//用于解析一个表达式
void expression(int level) {
    
}

//语法分析的入口，分析整个c语言程序
void program() {
    next();
    while (token > 0) {
        cout << "token = " << token << endl;
        next();
    }
}

//虚拟机的入口，用于解释目标代码
int eval() {
    armint op, *tmp; //oc: 指令
    while (1) {
        op = *pc++; //读取下一条指令地址
        if (op == IMM)       {ax = *pc++;} // pc指向的立即数载入ax，pc++
        else if (op == LC)   {ax = *(char *)ax;} // LC对应地址中的字符载入ax中
        else if (op == LI)   {ax = *(armint *)ax;} // LI将对应地址中的整数载入ax中
        else if (op == SC)   {ax = *(char *)*sp++ = ax;} // 栈顶元素char地址载入ax，栈pop
        else if (op == SI)   {*(armint *)*sp++ = ax;} // 栈顶元素int地址载入ax，栈pop
        else if (op == PUSH) {*--sp = ax;} // ax入栈push
        else if (op == JMP)  {pc = (armint *)*pc;} //pc跳
        else if (op == JZ)   {pc = ax ? pc + 1 : (armint *)*pc;} //ax=0则pc跳
        else if (op == JNZ)  {pc = ax ? (armint *)*pc : pc + 1;}  //ax!=0则pc跳
        else if (op == CALL) {*--sp = *(pc+1); pc = (armint *)*pc;} //调用子函数 __BUG
        //else if (op == RET)  {pc = (int *)*sp++;} //子函数return;
        else if (op == ENT)  {*--sp = *bp; bp = sp; sp = sp - *pc++;} // make new stack frame __BUG
        else if (op == ADJ)  {sp = sp + *pc++;}  // add esp, <size>
        else if (op == LEV)  {sp = bp; bp = (armint *)*sp++; pc = (armint *)*sp++;}  // RET，恢复调用帧和pc
        else if (op == LEA)  {ax = *(bp + *pc++);} // load address for arguments. __BUG
        
        
        else if (op == OR)  ax = *sp++ | ax;
        else if (op == XOR) ax = *sp++ ^ ax;
        else if (op == AND) ax = *sp++ & ax;
        else if (op == EQ)  ax = *sp++ == ax;
        else if (op == NE)  ax = *sp++ != ax;
        else if (op == LT)  ax = *sp++ < ax;
        else if (op == LE)  ax = *sp++ <= ax;
        else if (op == GT)  ax = *sp++ >  ax;
        else if (op == GE)  ax = *sp++ >= ax;
        else if (op == SHL) ax = *sp++ << ax;
        else if (op == SHR) ax = *sp++ >> ax;
        else if (op == ADD) ax = *sp++ + ax;
        else if (op == SUB) ax = *sp++ - ax;
        else if (op == MUL) ax = *sp++ * ax;
        else if (op == DIV) ax = *sp++ / ax;
        else if (op == MOD) ax = *sp++ % ax;
        
        else if (op == EXIT) { printf("exit(%d)", *sp); return *sp;}
        else if (op == OPEN) { ax = open((char *)sp[1], sp[0]); }
        else if (op == CLOS) { ax = close(*sp);}
        else if (op == READ) { ax = read(sp[2], (char *)sp[1], *sp); }
        else if (op == PRTF) { tmp = sp + pc[1]; ax = printf((char *)tmp[-1], tmp[-2], tmp[-3], tmp[-4], tmp[-5], tmp[-6]); }
        else if (op == MALC) { ax = (armint)malloc(*sp);}
        else if (op == MSET) { ax = (armint)memset((char *)sp[2], sp[1], *sp);}
        else if (op == MCMP) { ax = memcmp((char *)sp[2], (char *)sp[1], *sp);}
        else {
            printf("unknown instruction:%d\n", op);
            return -1;
        }
    }
    return 0;
}

int main(int argc, char **argv) {
    
    // 文件流 & 初始化
    armint i, fd;
    
    argc--;
    argv++;
    
    poolsize = 256 * 1024;
    
    if ((fd = open(*argv, 0)) < 0) {
        cout << "could not open" << *argv << endl;
        return -1;
    }
    
    if (!(src = old_src = (char*)malloc(poolsize))) {
        cout << "could not malloc" << poolsize << "for source area" << endl;
        return -1;
    }
    
    if ((i = (armint)read(fd, src, poolsize-1)) <= 0) {
        cout << "read() returned " << i << endl;
        return -1;
    }
    
    src[i] = 0; //EOF
    
    close(fd);
    
    
    // 虚拟机申请虚拟内存 & 初始化
    if (!(text_seg = old_text_seg = (armint*)malloc(poolsize))) {
        printf("could not malloc(%d) for text area\n", poolsize);
        return -1;
    }
    if (!(data_seg = (char*)malloc(poolsize))) {
        printf("could not malloc(%d) for data area\n", poolsize);
        return -1;
    }
    if (!(stack_seg = (armint*)malloc(poolsize))) {
        printf("could not malloc(%d) for stack area\n", poolsize);
        return -1;
    }
    
    memset(text_seg, 0, poolsize);
    memset(data_seg, 0, poolsize);
    memset(stack_seg, 0, poolsize);
    
    //初始化寄存器
    bp = sp = (armint *)((armint)stack_seg + poolsize);
    ax = 0;
    
    // 测试
    i = 0;
    text_seg[i++] = IMM;
    text_seg[i++] = 10;
    text_seg[i++] = PUSH;
    text_seg[i++] = IMM;
    text_seg[i++] = 20;
    text_seg[i++] = ADD;
    text_seg[i++] = PUSH;
    text_seg[i++] = EXIT;
    pc = text_seg;
    // end测试
    
    program();
    
    return eval();
}
