/* The thread pointer crosses the import as an i64 on every address width. */
__attribute__((import_module("linux"), import_name("get_thread_area")))
hidden unsigned long long __wasm_get_thread_area(void);

static inline uintptr_t __get_tp()
{
	return __wasm_get_thread_area();
}

#define MC_PC gregs[0]
