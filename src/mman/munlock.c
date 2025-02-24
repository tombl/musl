#include <sys/mman.h>
#include "syscall.h"

#ifndef __wasm__
int munlock(const void *addr, size_t len)
{
	return syscall(SYS_munlock, addr, len);
}
#endif