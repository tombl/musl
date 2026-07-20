#include <semaphore.h>
#include <limits.h>
#include "pthread_impl.h"
#ifdef __wasm__
#include "syscall.h"
#include "wasm_sem.h"
#endif

int sem_trywait(sem_t *sem)
{
#ifdef __wasm__
	if (__wasm_sem_is_named(sem))
		return syscall(SYS_wasm_sem_trywait, __wasm_sem_fd(sem));
#endif
	int val;
	while ((val=sem->__val[0]) & SEM_VALUE_MAX) {
		if (a_cas(sem->__val, val, val-1)==val) return 0;
	}
	errno = EAGAIN;
	return -1;
}
