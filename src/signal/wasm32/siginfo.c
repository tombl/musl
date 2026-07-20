#define _GNU_SOURCE
#include <signal.h>
#include <stdint.h>

/* The wasm kernel cannot place a native signal frame on the userspace stack:
 * handlers run as synchronous host callbacks instead. The runtime calls this
 * exported trampoline with the portable part of kernel_siginfo_t, and this
 * function materializes the public objects in userspace before invoking the
 * application's three-argument handler. */
__attribute__((visibility("default")))
void __wasm_call_siginfo_handler(uintptr_t fn, int sig, int code, pid_t pid,
	uid_t uid, uintptr_t value, int timerid, int overrun)
{
	void (*handler)(int, siginfo_t *, void *) = (void *)fn;
	siginfo_t info = {
		.si_signo = sig,
		.si_code = code,
	};

	/* Register state is not representable for a callback made at the wasm
	 * syscall boundary. Keep the context valid and intentionally minimal;
	 * siginfo carries the source information applications depend on. */
	ucontext_t context = {0};

	if (code == SI_TIMER) {
		info.si_timerid = timerid;
		info.si_overrun = overrun;
		info.si_value.sival_ptr = (void *)value;
	} else {
		info.si_pid = pid;
		info.si_uid = uid;
	}

	handler(sig, &info, &context);
}
