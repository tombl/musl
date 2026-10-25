#define _GNU_SOURCE
#include "pthread_impl.h"
#include "libc.h"
#include <sys/mman.h>

int pthread_getattr_np(pthread_t t, pthread_attr_t *a)
{
	*a = (pthread_attr_t){0};
	a->_a_detach = t->detach_state>=DT_DETACHED;
	a->_a_guardsize = t->guard_size;
	if (t->stack) {
		a->_a_stackaddr = (uintptr_t)t->stack;
		a->_a_stacksize = t->stack_size;
	} else {
#ifdef __wasm__
		/* The main thread runs on the stack wasm-ld places in linear
		 * memory (-z stack-size), and the linker defines its bounds
		 * for static links, which are all this port builds. A child
		 * cloned from the main thread runs on another stack with the
		 * main thread's descriptor, so check that the bounds apply. */
		extern char __stack_low[], __stack_high[];
		char here;
		if (t == __pthread_self() &&
		    ((uintptr_t)&here < (uintptr_t)__stack_low ||
		     (uintptr_t)&here >= (uintptr_t)__stack_high))
			return ENOSYS;
		a->_a_stackaddr = (uintptr_t)__stack_high;
		a->_a_stacksize = __stack_high - __stack_low;
#else
		// char *p = (void *)libc.auxv;
		// size_t l = PAGE_SIZE;
		// p += -(uintptr_t)p & PAGE_SIZE-1;
		// a->_a_stackaddr = (uintptr_t)p;
		// while (mremap(p-l-PAGE_SIZE, PAGE_SIZE, 2*PAGE_SIZE, 0)==MAP_FAILED && errno==ENOMEM)
		// 	l += PAGE_SIZE;
		// a->_a_stacksize = l;
		a_crash();
#endif
	}
	return 0;
}
