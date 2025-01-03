__asm__(".globaltype __stack_pointer, i32\n");
static inline void *__get_stack_pointer(void)
{
	void* ptr;
	__asm__ volatile("global.get __stack_pointer\n"
			 "local.set %0"
			 : "=r"(ptr));
	return ptr;
}

#ifdef START_is_dlstart
hidden void _dlstart_c(size_t *sp, size_t *dynv);
hidden void _dlstart(void) {
	_dlstart_c(__get_stack_pointer(), 0);
}
#endif

#ifdef START_is_start
hidden void _start_c(long *p);
hidden void _start(void) {
	_start_c(__get_stack_pointer());
}
#endif