#include <stdint.h>
#include <setjmp.h>

// setjmp/longjmp on wasm are implemented with LLVM's Wasm SjLj lowering (the
// WebAssemblyLowerEmscriptenEHSjLj pass, enabled by -mllvm -wasm-enable-sjlj).
// The pass rewrites each setjmp() call site into a call to __wasm_setjmp() and
// lowers longjmp() into a Wasm exception throw caught back in the setjmping
// frame (see longjmp.c). These runtime helpers own the jmp_buf layout; the
// pass only threads the buffer pointer and a per-activation id through them.
// This mirrors emscripten's system/lib/compiler-rt/emscripten_setjmp.c.

struct __wasm_longjmp_args {
	void *env;
	int val;
};

// Overlaid on the front of __jmp_buf, which is unsigned long long[6] (48 bytes,
// 8-byte aligned) on wasm32, so this 16-byte struct fits with room to spare.
struct __jmp_buf_impl {
	void *func_invocation_id;
	uint32_t label;
	struct __wasm_longjmp_args arg;
};

// Called by the pass at each setjmp() call site: record the setjmp label and
// the address identifying this function activation into the buffer.
void __wasm_setjmp(void *env, uint32_t label, void *func_invocation_id)
{
	struct __jmp_buf_impl *jb = env;
	jb->func_invocation_id = func_invocation_id;
	jb->label = label;
}

// Called by the pass in the longjmp catchpad: return the stored label if this
// buffer belongs to the current activation, otherwise 0 so the longjmp is
// rethrown to an outer frame.
uint32_t __wasm_setjmp_test(void *env, void *func_invocation_id)
{
	struct __jmp_buf_impl *jb = env;
	if (jb->func_invocation_id == func_invocation_id)
		return jb->label;
	return 0;
}

// The real setjmp symbol. With the Wasm SjLj pass every setjmp() call is
// rewritten, so this body only runs if a caller was compiled without the pass,
// where there is no working longjmp target and returning 0 (the direct return)
// is the safest fallback.
int setjmp(jmp_buf env)
{
	(void)env;
	return 0;
}

int _setjmp(jmp_buf) __attribute__((__weak__, __alias__("setjmp")));

// sigsetjmp is not recognised by the Wasm SjLj pass, so signal-mask
// save/restore via sigsetjmp/siglongjmp is not yet supported; no-op fallback.
int sigsetjmp(sigjmp_buf env, int savesigs)
{
	(void)env;
	(void)savesigs;
	return 0;
}
