#include <bits/setjmp.h>

int setjmp(__jmp_buf env) {
    // return __bultin_setjmp(env);
    return 0;
}

int sigsetjmp(__jmp_buf env, int savesigs) {
    return 0;
}
