#define _BSD_SOURCE
#include <unistd.h>
#include <errno.h>
#include "syscall.h"

#ifdef __wasm__
#include "pthread_impl.h"

extern void __heap_end; // synthesised by linker
static void *current = &__heap_end;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static int _brk(void *end) {
	if (end < current) goto fail;

	size_t inc = (uintptr_t)end - (uintptr_t)current;

	// as number of pages, rounding up
	inc = (inc + PAGE_SIZE - 1) / PAGE_SIZE;
	
	if (__builtin_wasm_memory_grow(0, inc) == SIZE_MAX) goto fail;	
	current = end;

	return 0;
fail:
	errno = ENOMEM;
	return -1;
}

int brk(void *end) {
	if (pthread_mutex_lock(&mutex) != 0) a_crash();
	int ret = _brk(end);
	pthread_mutex_unlock(&mutex);
	return ret;
}

void *sbrk(intptr_t inc) {
	if (pthread_mutex_lock(&mutex) != 0) a_crash();
	void *old = current;
	int ret = _brk(current + inc);
	pthread_mutex_unlock(&mutex);
	return ret == 0 ? old : (void *)-1;
}

#else
int brk(void *end)
{
	return __syscall_ret(-ENOMEM);
}
#endif
