#include "pthread_impl.h"
#include "sys/syscall.h"
#include <stdarg.h>

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

__attribute__((__noinline__))
static int clone_entry_inner(struct clone_entry_arg *arg)
{
  void *user_arg = arg->arg;
  int (*user_func)(void *) = arg->func;

  free(arg);
  user_func(user_arg);
}

static int clone_entry(void *arg_) {
  struct clone_entry_arg *arg = arg_;
  set_stack_pointer(arg->stack);
  clone_entry_inner(arg);
}

int __clone(int (*func)(void *), void *stack, int flags, void *arg, ...) {
  va_list ap;
  va_start(ap, arg);

  pid_t parent_tid = va_arg(ap, pid_t);
  void *tls = va_arg(ap, void *);
  pid_t child_tid = va_arg(ap, pid_t);

  va_end(ap);

  struct clone_entry_arg *args = malloc(sizeof(*args));
  args->stack = stack;

  return __syscall(SYS_clone, func, arg, flags, parent_tid, child_tid, tls);
}
