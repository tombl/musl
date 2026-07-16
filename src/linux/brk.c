#define _BSD_SOURCE
#include <unistd.h>
#include <errno.h>
#include "syscall.h"

#ifdef __wasm__
#include "pthread_impl.h"

extern void __heap_end; // synthesised by linker
static void *heap_limit;

static void *current = &__heap_end;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static int _brk(void *end) {
	if (!heap_limit)
		heap_limit = (void *)(__builtin_wasm_memory_size(0) * PAGESIZE);

	if (end < current) goto fail;

	if (end <= heap_limit) {
		current = end;
		return 0;
	}

	size_t bytes = (uintptr_t)end - (uintptr_t)heap_limit;
	size_t pages = (bytes + PAGESIZE - 1) / PAGESIZE;

	if (__builtin_wasm_memory_grow(0, pages) == SIZE_MAX) goto fail;
	heap_limit = (void *)(__builtin_wasm_memory_size(0) * PAGESIZE);
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
