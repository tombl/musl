CC = clang
CFLAGS += --target=wasm32 -matomics -mbulk-memory
LDFLAGS =
CROSS_COMPILE = llvm-
LIBCC =
