#define _GNU_SOURCE
#include <features.h>
#include <signal.h>
#include <stdint.h>

__attribute__((import_module("linux"), import_name("copy_siginfo")))
int __wasm_copy_siginfo(siginfo_t *);

hidden void __restore(void)
{
}

/* Signal delivery is a synchronous host callback, so there is no native
 * signal frame to restore. Instead, the kernel calls this restorer as the
 * userspace trampoline. It materializes the public objects before invoking
 * the application's three-argument handler. */
hidden void __restore_rt(uintptr_t fn, int sig)
{
	void (*handler)(int, siginfo_t *, void *) = (void *)fn;
	siginfo_t info;

	/* Register state is not representable for a callback made at the wasm
	 * syscall boundary. Keep the context valid and intentionally minimal;
	 * siginfo carries the source information applications depend on. */
	ucontext_t context = {0};

	if (__wasm_copy_siginfo(&info)) return;

	handler(sig, &info, &context);
}
