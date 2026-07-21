#include <features.h>
#include "process_args.h"

int __main_argc_argv(int, char **);

weak int __main_void(void)
{
	return __main_argc_argv(__wasm_process_args->argc,
		__wasm_process_args->argv);
}
