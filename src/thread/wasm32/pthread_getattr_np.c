#define _GNU_SOURCE
#include "pthread_impl.h"

/* The linker places the main thread's stack (-z stack-size) in linear
 * memory and defines its bounds, for static links, which are all this
 * port builds. There is no mremap to probe the stack with. */
extern char __stack_low[], __stack_high[];

int pthread_getattr_np(pthread_t t, pthread_attr_t *a)
{
	*a = (pthread_attr_t){0};
	a->_a_detach = t->detach_state>=DT_DETACHED;
	a->_a_guardsize = t->guard_size;
	if (t->stack) {
		a->_a_stackaddr = (uintptr_t)t->stack;
		a->_a_stacksize = t->stack_size;
	} else {
		/* A child cloned from the main thread without CLONE_VM keeps
		 * its descriptor but runs on the stack its parent supplied. */
		char here;
		if (t == __pthread_self() &&
		    ((uintptr_t)&here < (uintptr_t)__stack_low ||
		     (uintptr_t)&here >= (uintptr_t)__stack_high))
			return ENOSYS;
		a->_a_stackaddr = (uintptr_t)__stack_high;
		a->_a_stacksize = __stack_high - __stack_low;
	}
	return 0;
}
