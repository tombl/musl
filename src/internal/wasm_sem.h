#ifndef WASM_SEM_H
#define WASM_SEM_H

#include <semaphore.h>

#define WASM_SEM_TAG 0x73656d66

static inline int __wasm_sem_is_named(const sem_t *sem)
{
	return sem->__val[3] == WASM_SEM_TAG;
}

static inline int __wasm_sem_fd(const sem_t *sem)
{
	return sem->__val[0];
}

#endif
