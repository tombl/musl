#include <stdint.h>

// __wasm_longjmp is the target the Wasm SjLj pass lowers every longjmp() call
// site to. It throws a Wasm exception carrying (env, val); the pass-generated
// catchpad in the setjmping frame unwinds to it and matches the buffer with
// __wasm_setjmp_test (see setjmp.c). Kept in its own translation unit compiled
// with -mllvm -wasm-enable-sjlj so the __builtin_wasm_throw below has the
// exception-handling feature enabled, without the pass also seeing a real
// longjmp definition to redirect (which would collide with this symbol).

struct __wasm_longjmp_args {
	void *env;
	int val;
};

// Overlaid on the front of __jmp_buf (see setjmp.c for the layout contract).
struct __jmp_buf_impl {
	void *func_invocation_id;
	uint32_t label;
	struct __wasm_longjmp_args arg;
};

// LLVM uses tag 1 (__c_longjmp) for the longjmp exception; the linker
// synthesises the tag. See llvm/include/llvm/CodeGen/WasmEHFuncInfo.h.
#define WASM_C_LONGJMP 1

void __wasm_longjmp(void *env, int val)
{
	struct __jmp_buf_impl *jb = env;
	// The C standard forbids longjmp from making setjmp return 0.
	if (val == 0)
		val = 1;
	jb->arg.env = env;
	jb->arg.val = val;
	__builtin_wasm_throw(WASM_C_LONGJMP, &jb->arg);
}
