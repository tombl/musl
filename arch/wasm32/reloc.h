#define LDSO_ARCH "wasm32"

#define CRTJMP(pc, sp) __builtin_trap()