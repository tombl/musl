#include "pthread_impl.h"
#include "sys/syscall.h"
#include <stdarg.h>

int __clone(int (*func)(void *), void *stack, int flags, void *arg, ...) {
  va_list ap;
  va_start(ap, arg);

  pid_t parent_tid = va_arg(ap, pid_t);
  void *tls = va_arg(ap, void *);
  pid_t child_tid = va_arg(ap, pid_t);

  va_end(ap);

  return __syscall(SYS_clone, func, arg, flags, parent_tid, child_tid, tls);
}
