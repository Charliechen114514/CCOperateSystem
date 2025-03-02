#include "include/syscall/syscall.h"

/* System call with no parameters */
#define _syscall0(NUMBER)                                                  \
    ({                                                                     \
        int retval;                                                        \
        asm volatile("int $0x80" : "=a"(retval) : "a"(NUMBER) : "memory"); \
        retval;                                                            \
    })

/* System call with one parameter */
#define _syscall1(NUMBER, ARG1)               \
    ({                                        \
        int retval;                           \
        asm volatile("int $0x80"              \
                     : "=a"(retval)           \
                     : "a"(NUMBER), "b"(ARG1) \
                     : "memory");             \
        retval;                               \
    })

/* System call with two parameters */
#define _syscall2(NUMBER, ARG1, ARG2)                    \
    ({                                                   \
        int retval;                                      \
        asm volatile("int $0x80"                         \
                     : "=a"(retval)                      \
                     : "a"(NUMBER), "b"(ARG1), "c"(ARG2) \
                     : "memory");                        \
        retval;                                          \
    })

/* System call with three parameters */
#define _syscall3(NUMBER, ARG1, ARG2, ARG3)                         \
    ({                                                              \
        int retval;                                                 \
        asm volatile("int $0x80"                                    \
                     : "=a"(retval)                                 \
                     : "a"(NUMBER), "b"(ARG1), "c"(ARG2), "d"(ARG3) \
                     : "memory");                                   \
        retval;                                                     \
    })

/* Returns the current task's PID */
uint32_t getpid() {
    return _syscall0(SYS_GETPID);
}

/* Writes 'count' characters from 'buf' to file descriptor 'fd' */
uint32_t write(int32_t fd, const void *buf, uint32_t count) {
    return _syscall3(SYS_WRITE, fd, buf, count);
}

/* Allocates 'size' bytes of memory and returns the result */
void *malloc(uint32_t size) {
    return (void *)_syscall1(SYS_MALLOC, size);
}

/* Frees the memory pointed to by 'ptr' */
void free(void *ptr) {
    _syscall1(SYS_FREE, ptr);
}

/* Creates a child process and returns the child process's PID */
pid_t fork(void) {
    return _syscall0(SYS_FORK);
}