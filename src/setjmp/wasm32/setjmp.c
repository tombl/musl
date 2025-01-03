#include <bits/setjmp.h>

int setjmp(__jmp_buf env) {
    __builtin_trap();
    // return __bultin_setjmp(env);
}