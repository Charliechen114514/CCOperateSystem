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
uint32_t getpid()
{
    return _syscall0(SYS_GETPID);
}

/* Writes 'count' characters from 'buf' to file descriptor 'fd' */
uint32_t write(int32_t fd, const void *buf, uint32_t count)
{
    return _syscall3(SYS_WRITE, fd, buf, count);
}

/* Allocates 'size' bytes of memory and returns the result */
void *malloc(uint32_t size)
{
    return (void *)_syscall1(SYS_MALLOC, size);
}

/* Frees the memory pointed to by 'ptr' */
void free(void *ptr)
{
    _syscall1(SYS_FREE, ptr);
}

/* Creates a child process and returns the child process's PID */
pid_t fork(void)
{
    return _syscall0(SYS_FORK);
}

int32_t read(int32_t fd, void *buf, uint32_t count)
{
    return _syscall3(SYS_READ, fd, buf, count);
}


/* Outputs a character */
void putchar(char char_asci) {
    _syscall1(SYS_PUTCHAR, char_asci);
}

/* Clears the screen */
void clear(void) {
    _syscall0(SYS_CLEAR);
}

/* Gets the current working directory */
char *getcwd(char *buf, uint32_t size) {
    return (char *)_syscall2(SYS_GETCWD, buf, size);
}

/* Opens the file at 'pathname' with 'flag' */
int32_t open(char *pathname, uint8_t flag) {
    return _syscall2(SYS_OPEN, pathname, flag);
}

/* Closes the file descriptor 'fd' */
int32_t close(int32_t fd) {
    return _syscall1(SYS_CLOSE, fd);
}

/* Sets the file offset */
int32_t lseek(int32_t fd, int32_t offset, uint8_t whence) {
    return _syscall3(SYS_LSEEK, fd, offset, whence);
}

/* Deletes the file at 'pathname' */
int32_t unlink(const char *pathname) {
    return _syscall1(SYS_UNLINK, pathname);
}

/* Creates a directory at 'pathname' */
int32_t mkdir(const char *pathname) {
    return _syscall1(SYS_MKDIR, pathname);
}

/* Opens the directory 'name' */
Dir *opendir(const char *name) {
    return (Dir *)_syscall1(SYS_OPENDIR, name);
}

/* Closes the directory 'dir' */
int32_t closedir(Dir *dir) {
    return _syscall1(SYS_CLOSEDIR, dir);
}

/* Deletes the directory at 'pathname' */
int32_t rmdir(const char *pathname) {
    return _syscall1(SYS_RMDIR, pathname);
}

/* Reads an entry from the directory 'dir' */
DirEntry *readdir(Dir *dir) {
    return (DirEntry *)_syscall1(SYS_READDIR, dir);
}

/* Rewinds the directory pointer 'dir' */
void rewinddir(Dir *dir) {
    _syscall1(SYS_REWINDDIR, dir);
}

/* Gets the attributes of 'path' and stores them in 'buf' */
int32_t stat(const char *path, Stat *buf) {
    return _syscall2(SYS_STAT, path, buf);
}

/* Changes the current working directory to 'path' */
int32_t chdir(const char *path) {
    return _syscall1(SYS_CHDIR, path);
}

/* Displays the task list */
void ps(void) {
    _syscall0(SYS_PS);
}