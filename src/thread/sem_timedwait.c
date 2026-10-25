#include <semaphore.h>
#include <limits.h>
#include "pthread_impl.h"
#ifdef __wasm__
#include "syscall.h"
#include "wasm_sem.h"
#endif

static void cleanup(void *p)
{
	a_dec(p);
}

int sem_timedwait(sem_t *restrict sem, const struct timespec *restrict at)
{
	pthread_testcancel();

#ifdef __wasm__
	if (__wasm_sem_is_named(sem)) {
		long result;

		if (at) {
			long long kernel_time[2] = { at->tv_sec, at->tv_nsec };

			result = __syscall_cp(SYS_wasm_sem_timedwait,
				__wasm_sem_fd(sem), kernel_time);
		} else {
			result = __syscall_cp(SYS_wasm_sem_wait,
				__wasm_sem_fd(sem));
		}
		return __syscall_ret(result);
	}
#endif

	if (!sem_trywait(sem)) return 0;

	int spins = 100;
	while (spins-- && !(sem->__val[0] & SEM_VALUE_MAX) && !sem->__val[1])
		a_spin();

	while (sem_trywait(sem)) {
		int r, priv = sem->__val[2];
		a_inc(sem->__val+1);
		a_cas(sem->__val, 0, 0x80000000);
		pthread_cleanup_push(cleanup, (void *)(sem->__val+1));
		r = __timedwait_cp(sem->__val, 0x80000000, CLOCK_REALTIME, at, priv);
		pthread_cleanup_pop(1);
		if (r) {
			errno = r;
			return -1;
		}
	}
	return 0;
}
