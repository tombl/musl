CC = clang
CFLAGS += --target=wasm32 -matomics -mbulk-memory
LDFLAGS =
CROSS_COMPILE = llvm-
LIBCC =

# longjmp is implemented with LLVM's Wasm SjLj lowering. __wasm_longjmp uses
# __builtin_wasm_throw, and any musl translation unit that itself calls longjmp
# (siglongjmp) must be compiled with the pass enabled so the call is rewritten.
# Scoped to these objects so the rest of libc stays free of the Wasm
# exception-handling feature; a guest only requires it if it uses setjmp.
#
# Only wasm_longjmp.c (the __builtin_wasm_throw helper) and siglongjmp.c (which
# calls longjmp) go through the pass. setjmp.c and longjmp.c are excluded: the
# setjmp/longjmp helpers are plain memory writes, and running the pass over the
# real setjmp/longjmp definitions rejects the _setjmp/_longjmp weak aliases as
# "indirect use of setjmp" or collides with the __wasm_longjmp symbol.
WASM_SJLJ_OBJS = \
	obj/src/setjmp/wasm32/wasm_longjmp.o obj/src/setjmp/wasm32/wasm_longjmp.lo \
	obj/src/signal/siglongjmp.o obj/src/signal/siglongjmp.lo
$(WASM_SJLJ_OBJS): CFLAGS_ALL += -mllvm -wasm-enable-sjlj
