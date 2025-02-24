#include <sys/mman.h>
#include "syscall.h"

#ifndef __wasm__
int msync(void *start, size_t len, int flags)
{
	return syscall_cp(SYS_msync, start, len, flags);
}
#endif
