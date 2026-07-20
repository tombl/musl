#include "process_args.h"

__attribute__((import_module("linux"), import_name("get_args_length")))
int wasm_get_args_length(void);
__attribute__((import_module("linux"), import_name("get_args")))
int wasm_get_args(struct wasm_process_args *buf);

// TODO: malloc this once mmap is fixed
static char args_buf[65536 * 4];
hidden struct wasm_process_args *__wasm_process_args = (void*)args_buf;

#ifdef START_is_dlstart
hidden void _dlstart_c(size_t *sp, size_t *dynv);
hidden void _dlstart(void) {
	_dlstart_c(0, 0);
}
#endif

#ifdef START_is_start
void __wasm_call_ctors(void);
int __main_void(void);
hidden void _start_c(long *p);
hidden void _start(void) {
	int len = wasm_get_args_length();
	if (len < 0) __builtin_trap();
	if (len > sizeof(args_buf)) __builtin_trap();

	if (wasm_get_args(__wasm_process_args) < 0) __builtin_trap();

	__init_libc(__wasm_process_args->envp, __wasm_process_args->argv[0]);
	__libc_start_init();
	__wasm_call_ctors();

	exit(__main_void());
}
#endif
