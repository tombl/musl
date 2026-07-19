#define _GNU_SOURCE
#include <stdarg.h>
#include <unistd.h>
#include <sched.h>
#include "pthread_impl.h"
#include "syscall.h"
#include "lock.h"
#include "fork_impl.h"

struct clone_start_args {
	int (*func)(void *);
	void *arg;
	sigset_t sigmask;
};

static int clone_start(void *arg)
{
	struct clone_start_args *csa = arg;
	__post_Fork(0);
	__restore_sigs(&csa->sigmask);
	return csa->func(csa->arg);
}

int clone(int (*func)(void *), void *stack, int flags, void *arg, ...)
{
	struct clone_start_args csa;
	va_list ap;
	pid_t *ptid = 0, *ctid = 0;
	void  *tls = 0;

	/* Flags that produce an invalid thread/TLS state are disallowed. */
	int badflags = CLONE_THREAD | CLONE_SETTLS | CLONE_CHILD_CLEARTID;

	if ((flags & badflags) || !stack)
		return __syscall_ret(-EINVAL);

	va_start(ap, arg);
	if (flags & (CLONE_PIDFD | CLONE_PARENT_SETTID))
	 	ptid = va_arg(ap, pid_t *);
	if (flags & CLONE_SETTLS)
		tls = va_arg(ap, void *);
	if (flags & (CLONE_CHILD_SETTID | CLONE_CHILD_CLEARTID))
		ctid = va_arg(ap, pid_t *);
	va_end(ap);

	if (flags & CLONE_VM) return __syscall_ret(
		__clone(func, stack, flags, arg, ptid, tls, ctid));
	__block_all_sigs(&csa.sigmask);
	LOCK(__abort_lock);

	csa.func = func;
	csa.arg = arg;
	int ret = __clone(clone_start, stack, flags, &csa, ptid, tls, ctid);

	__post_Fork(ret);
	__restore_sigs(&csa.sigmask);
	return __syscall_ret(ret);
}
