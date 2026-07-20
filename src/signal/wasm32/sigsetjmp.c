#include <setjmp.h>
#include <signal.h>
#include "syscall.h"

#undef sigsetjmp

struct __jmp_buf_tag *__wasm_sigsetjmp_prepare(sigjmp_buf jb, int savesigs)
{
	jb->__fl = savesigs != 0;
	if (jb->__fl && __syscall(SYS_rt_sigprocmask, SIG_SETMASK, 0,
	    jb->__ss, _NSIG/8) < 0)
		__builtin_trap();
	return jb;
}

/* Calls through the ABI symbol cannot provide a caller-side catchpad. */
int sigsetjmp(sigjmp_buf jb, int savesigs)
{
	(void)jb;
	(void)savesigs;
	__builtin_trap();
}
