#include <sys/mman.h>
#include "syscall.h"

#ifndef __wasm__
int munlockall(void)
{
	return syscall(SYS_munlockall);
}
#endif
