#define _GNU_SOURCE
#include "pthread_impl.h"
#include "sys/syscall.h"
#include <sched.h>
#include <stdarg.h>
#include <stdint.h>

__asm__(".globaltype __stack_pointer, i32\n");
static inline void set_stack_pointer(void *ptr)
{
	__asm__ volatile("local.get %0\n"
			 "global.set __stack_pointer" ::"r"(ptr));
}

struct clone_entry_arg {
  void *stack;
  int (*func)(void *);
  void *arg;
};

static int clone_entry(void *arg_) {
  struct clone_entry_arg arg = *(struct clone_entry_arg *)arg_;
  set_stack_pointer(arg.stack);
  return arg.func(arg.arg);
}

int __clone(int (*func)(void *), void *stack, int flags, void *arg, ...) {
  pid_t *parent_tid = 0;
  void *tls = 0;
  pid_t *child_tid = 0;

  if (flags & (CLONE_PARENT_SETTID | CLONE_PIDFD | CLONE_SETTLS |
               CLONE_CHILD_SETTID | CLONE_CHILD_CLEARTID)) {
    va_list ap;
    va_start(ap, arg);

    parent_tid = va_arg(ap, pid_t *);
    tls = va_arg(ap, void *);
    child_tid = va_arg(ap, pid_t *);

    va_end(ap);
  }

  uintptr_t child_stack = (uintptr_t)stack & -16;
  child_stack -= (sizeof(struct clone_entry_arg) + 15) & -16;

  struct clone_entry_arg *args = (void *)child_stack;
  *args = (struct clone_entry_arg) {
    .stack = (void *)child_stack,
    .func = func,
    .arg = arg,
  };

  return __syscall(SYS_clone, clone_entry, args, flags, parent_tid, child_tid, tls);
}
