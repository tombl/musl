#include <features.h>
#include "process_args.h"

int __main_argc_argv_envp(int, char **, char **);

weak int __main_argc_argv(int argc, char **argv)
{
	return __main_argc_argv_envp(argc, argv, __wasm_process_args->envp);
}
