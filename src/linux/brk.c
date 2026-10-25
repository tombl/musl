#define _BSD_SOURCE
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include "syscall.h"

#ifdef __wasm__
#include "pthread_impl.h"

extern void __heap_end; // synthesised by linker
static size_t heap_pages;

static void *current = &__heap_end;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static int _brk(void *end) {
	// Count pages rather than bytes: a full 4 GiB memory is 2^32 bytes,
	// which does not fit in a 32-bit pointer.
	if (!heap_pages)
		heap_pages = __builtin_wasm_memory_size(0);

	if (end < current) goto fail;

	uint64_t limit = (uint64_t)heap_pages * PAGESIZE;
	if ((uintptr_t)end <= limit) {
		current = end;
		return 0;
	}

	size_t bytes = (uintptr_t)end - (size_t)limit;
	size_t pages = (bytes + PAGESIZE - 1) / PAGESIZE;

	size_t old = __builtin_wasm_memory_grow(0, pages);
	if (old == SIZE_MAX) goto fail;
	// Everything from the cached limit up belongs to whoever grew it. If
	// that was not us, the memory we would hand out is already in use.
	if (old != heap_pages) a_crash();
	heap_pages = old + pages;
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
