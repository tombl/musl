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

// Keep at least one page of backed linear memory beyond the program break.
//
// On a conventional target sbrk hands out page-granular memory and the bytes
// between the break and the end of its page stay accessible, so the small
// over-reads that C code performs just past a heap allocation (word-at-a-time
// and page-rounded scans in libc and in programs like GNU coreutils) are
// harmless. wasm linear memory has no such slack of its own: an access at or
// beyond memory.size traps. Because malloc here draws from sbrk and packs
// allocations directly against the break, a break landing on a 64KiB memory
// boundary leaves the top allocation abutting unbacked memory, and an otherwise
// benign over-read traps with "memory access out of bounds". Reserving a guard
// page past the break restores the page-granular read slack those programs
// rely on.
#define BRK_GUARD PAGESIZE

static int _brk(void *end) {
	if (!heap_limit)
		heap_limit = (void *)(__builtin_wasm_memory_size(0) * PAGESIZE);

	if (end < current) goto fail;

	// The break itself is `end`; grow memory so the guard page past it is
	// backed too.
	uintptr_t want = (uintptr_t)end + BRK_GUARD;

	if (want <= (uintptr_t)heap_limit) {
		current = end;
		return 0;
	}

	size_t bytes = want - (uintptr_t)heap_limit;
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
