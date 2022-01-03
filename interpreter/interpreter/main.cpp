//
//  main.cpp
//  interpreter
//
//  Created by 徐浩博 on 2022/1/3.
//

#include <iostream>
#include <memory>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>
using namespace std;

int token; //当前token
char *src, *old_src; //被编译代码的指针
int poolsize; //代码段/数据段/栈段的大小
int line; //当前行

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
    return 0;
}

int main(int argc, char **argv) {
    
    int i, fd;
    
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
    
    if ((i = (int)read(fd, src, poolsize-1)) <= 0) {
        cout << "read() returned " << i << endl;
        return -1;
    }
    
    src[i] = 0; //EOF
    
    close(fd);
    
    program();
    
    return eval();
}
