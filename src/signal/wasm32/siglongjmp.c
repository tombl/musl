#include <setjmp.h>
#include <signal.h>
#include "syscall.h"

_Noreturn void siglongjmp(sigjmp_buf jb, int ret)
{
	if (jb->__fl && __syscall(SYS_rt_sigprocmask, SIG_SETMASK, jb->__ss,
	    0, _NSIG/8) < 0)
		__builtin_trap();
	longjmp(jb, ret);
}
