#include <sys/mman.h>
#include "syscall.h"

#ifndef __wasm__
int mlockall(int flags)
{
	return syscall(SYS_mlockall, flags);
}
#endif
