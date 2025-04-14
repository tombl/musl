#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <malloc.h>
#include <unistd.h>
#include "syscall.h"

#ifdef __wasm__
const char unsigned *__map_file(const char *pathname, size_t *size)
{
	struct stat st;
	unsigned char *buf = 0;
	int fd = sys_open(pathname, O_RDONLY|O_CLOEXEC);
	if (fd < 0) return 0;
	if (!__fstat(fd, &st)) {
		buf = malloc(st.st_size);
		if (!buf) return 0;

		ssize_t n = read(fd, buf, st.st_size);
		if (n != st.st_size)  {
			free(buf);
			buf = 0;
		}

		*size = st.st_size;
	}
	__syscall(SYS_close, fd);
	return buf;
}
void __unmap_file(const char *map, size_t size)
{
	free((void *)map);
}
#else
const char unsigned *__map_file(const char *pathname, size_t *size)
{
	struct stat st;
	const unsigned char *map = MAP_FAILED;
	int fd = sys_open(pathname, O_RDONLY|O_CLOEXEC|O_NONBLOCK);
	if (fd < 0) return 0;
	if (!__fstat(fd, &st)) {
		map = __mmap(0, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
		*size = st.st_size;
	}
	__syscall(SYS_close, fd);
	return map == MAP_FAILED ? 0 : map;
}
void __unmap_file(const char *map, size_t size)
{
	__munmap(map, size);
}
#endif


