//
//  hello.c
//  interpreter
//
//  Created by 徐浩博 on 2022/1/4.
//

#include <stdio.h>

int fib(int a) {
    if (i <= 1) {
        return 1;
    }
    return fib(a-1) + fib(a-2);
}

int main() {
    int i;
    i = 0;
    while (i <= 10) {
        printf("fib(%2d) = %d\n", i, fib(i));
        i = i + 1;
    }
    return 0;
}
