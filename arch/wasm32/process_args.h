#ifndef _WASM32_PROCESS_ARGS_H
#define _WASM32_PROCESS_ARGS_H

struct wasm_process_args {
	int len, envc, argc;
	char **argv, **envp;
	char data[];
};

extern hidden struct wasm_process_args *__wasm_process_args;

#endif
