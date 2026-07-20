#include <semaphore.h>
#include <limits.h>
#ifdef __wasm__
#include "syscall.h"
#include "wasm_sem.h"
#endif

int sem_getvalue(sem_t *restrict sem, int *restrict valp)
{
#ifdef __wasm__
	if (__wasm_sem_is_named(sem)) {
		int value = __syscall_ret(__syscall(SYS_wasm_sem_getvalue,
			__wasm_sem_fd(sem)));

		if (value < 0) return -1;
		*valp = value;
		return 0;
	}
#endif
	int val = sem->__val[0];
	*valp = val & SEM_VALUE_MAX;
	return 0;
}
