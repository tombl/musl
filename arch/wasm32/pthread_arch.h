__attribute__((import_module("linux"), import_name("get_thread_area")))
hidden uintptr_t __get_tp();

#define MC_PC gregs[0]
