#include <bits/setjmp.h>

void longjmp(__jmp_buf env, int val) {
    __builtin_trap();
    // __builtin_longjmp(env, val);
}