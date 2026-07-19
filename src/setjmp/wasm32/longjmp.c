#include <setjmp.h>

// The real longjmp symbol. With the Wasm SjLj pass every longjmp() call is
// rewritten to __wasm_longjmp() (see wasm_longjmp.c), so this body only runs if
// a caller was compiled without the pass, where there is no catchpad to unwind
// to. Kept out of the pass so the _longjmp weak alias below is not rejected as
// an indirect use of longjmp.
_Noreturn void longjmp(jmp_buf env, int val)
{
	(void)env;
	(void)val;
	__builtin_trap();
}

_Noreturn void _longjmp(jmp_buf, int) __attribute__((__weak__, __alias__("longjmp")));
