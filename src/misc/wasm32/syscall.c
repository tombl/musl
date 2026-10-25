#define _BSD_SOURCE
#include <unistd.h>
#include "syscall.h"
#include <stdarg.h>

#undef syscall

/*
 * The kernel's argument slots are 64 bits wide, but callers of syscall()
 * pass ints, longs and pointers, which wasm32 varargs store as 4-byte values.
 * Reading syscall_arg_t here would consume two of them per argument. Read each
 * argument as a long instead, as on every ILP32 port, and sign-extend it into
 * its slot. A 64-bit value cannot pass through syscall(), because the callee
 * cannot tell how wide each variadic argument was. Use the libc wrapper
 * (pread, ftruncate, fallocate, posix_fadvise, ...) instead.
 */
long syscall(long n, ...)
{
	va_list ap;
	long a,b,c,d,e,f;
	va_start(ap, n);
	a=va_arg(ap, long);
	b=va_arg(ap, long);
	c=va_arg(ap, long);
	d=va_arg(ap, long);
	e=va_arg(ap, long);
	f=va_arg(ap, long);
	va_end(ap);
	return __syscall_ret(__syscall(n,a,b,c,d,e,f));
}
