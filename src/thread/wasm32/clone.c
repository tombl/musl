#define _GNU_SOURCE
#include "pthread_impl.h"
#include "sys/syscall.h"
#include <sched.h>
#include <stdarg.h>
#include <stdint.h>

__asm__(".globaltype __stack_pointer, i32\n");
__asm__(".globaltype __tls_base, i32\n");

static inline void set_stack_pointer(void *ptr)
{
	__asm__ volatile("local.get %0\n"
			 "global.set __stack_pointer" ::"r"(ptr));
}

static inline void *get_tls_base(void)
{
	void *ptr;
	__asm__ volatile("global.get __tls_base\n"
			 "local.set %0" : "=r"(ptr));
	return ptr;
}

static inline void set_tls_base(void *ptr)
{
	__asm__ volatile("local.get %0\n"
			 "global.set __tls_base" :: "r"(ptr));
}

struct clone_entry_arg {
  void *stack;
  void *tls_base;
  int (*func)(void *);
  void *arg;
};

__attribute__((__noreturn__, __noinline__))
static void clone_entry_inner(int (*func)(void *), void *arg) {
  int status = func(arg);
  __syscall(SYS_exit, status);
  __builtin_unreachable();
}

static int clone_entry(void *arg_) {
  struct clone_entry_arg *arg = arg_;
  void *stack = arg->stack;
  void *tls_base = arg->tls_base;
  int (*func)(void *) = arg->func;
  void *user_arg = arg->arg;

  set_stack_pointer(stack);
  set_tls_base(tls_base);
  clone_entry_inner(func, user_arg);
  __builtin_unreachable();
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
    .tls_base = get_tls_base(),
    .func = func,
    .arg = arg,
  };

  return __syscall(SYS_clone, clone_entry, args, flags, parent_tid, child_tid, tls);
}
