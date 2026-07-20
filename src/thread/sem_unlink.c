#include <semaphore.h>
#include <sys/mman.h>
#ifdef __wasm__
#include <limits.h>
#include "syscall.h"
#endif

int sem_unlink(const char *name)
{
#ifdef __wasm__
	char mapped[NAME_MAX+10];

	if (!(name = __shm_mapname(name, mapped))) return -1;
	return syscall(SYS_wasm_sem_unlink, name+9);
#else
	return shm_unlink(name);
#endif
}
