typedef int32_t int_fast16_t;
typedef int32_t int_fast32_t;
typedef uint32_t uint_fast16_t;
typedef uint32_t uint_fast32_t;

#define INT_FAST16_MIN  INT32_MIN
#define INT_FAST32_MIN  INT32_MIN

#define INT_FAST16_MAX  INT32_MAX
#define INT_FAST32_MAX  INT32_MAX

#define UINT_FAST16_MAX UINT32_MAX
#define UINT_FAST32_MAX UINT32_MAX

/*
 * size_t, intptr_t and ptrdiff_t are (unsigned) long here, as in the wasm32
 * C ABI (_Addr), not int as generic 32-bit ports have them: their limits
 * must have the same types (as clang's __SIZE_MAX__ etc. do).
 */
#define INTPTR_MIN      (-0x7fffffffL-1)
#define INTPTR_MAX      0x7fffffffL
#define UINTPTR_MAX     0xffffffffUL
#define PTRDIFF_MIN     (-0x7fffffffL-1)
#define PTRDIFF_MAX     0x7fffffffL
#define SIZE_MAX        0xffffffffUL
