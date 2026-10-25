/*
 * linux.syscall is (i64 nr, i64 a..f) -> i64 on every address width. Each
 * argument occupies one slot, so a 64-bit value is passed whole rather than
 * as a pair of longs.
 *
 * The kernel ABI extends each argument according to its C type in the
 * kernel's prototype: pointers, size_t and other unsigned types zero-extend,
 * and signed types sign-extend. __scc converts each argument to long long by
 * the type musl passes, which implements that rule wherever the two types
 * agree in signedness. Clang converts a wasm32 pointer to a wider integer by
 * zero extension. Where they disagree (an int fd for an unsigned int fd),
 * only a value with the sign bit set differs, and the kernel truncates every
 * slot to its parameter's width, so the call still behaves the same.
 *
 * The result is the kernel's long, sign-extended, so truncating it to a
 * long is exact and __syscall_ret needs no change.
 */
#define __SYSCALL_LL_E(x) (x)
#define __SYSCALL_LL_O(x) (x)

#define __scc(X) ((syscall_arg_t) (X))
typedef long long syscall_arg_t;

__attribute__((import_module("linux"), import_name("syscall")))
long long __wasm_syscall(long long n, syscall_arg_t a, syscall_arg_t b, syscall_arg_t c, syscall_arg_t d, syscall_arg_t e, syscall_arg_t f);

static inline long __syscall0(long n)                                                                                                   { return __wasm_syscall(n, 0, 0, 0, 0, 0, 0); }
static inline long __syscall1(long n, syscall_arg_t a)                                                                                  { return __wasm_syscall(n, a, 0, 0, 0, 0, 0); }
static inline long __syscall2(long n, syscall_arg_t a, syscall_arg_t b)                                                                 { return __wasm_syscall(n, a, b, 0, 0, 0, 0); }
static inline long __syscall3(long n, syscall_arg_t a, syscall_arg_t b, syscall_arg_t c)                                                { return __wasm_syscall(n, a, b, c, 0, 0, 0); }
static inline long __syscall4(long n, syscall_arg_t a, syscall_arg_t b, syscall_arg_t c, syscall_arg_t d)                               { return __wasm_syscall(n, a, b, c, d, 0, 0); }
static inline long __syscall5(long n, syscall_arg_t a, syscall_arg_t b, syscall_arg_t c, syscall_arg_t d, syscall_arg_t e)              { return __wasm_syscall(n, a, b, c, d, e, 0); }
static inline long __syscall6(long n, syscall_arg_t a, syscall_arg_t b, syscall_arg_t c, syscall_arg_t d, syscall_arg_t e, syscall_arg_t f) { return __wasm_syscall(n, a, b, c, d, e, f); }
