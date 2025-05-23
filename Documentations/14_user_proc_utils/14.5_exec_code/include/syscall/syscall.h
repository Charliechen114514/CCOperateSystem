#ifndef __LIB_USER_SYSCALL_H
#define __LIB_USER_SYSCALL_H

#include "include/thread/thread.h"
#include "include/library/types.h"
#include "include/filesystem/filesystem.h"

enum SYSCALL_NR
{
    SYS_GETPID,
    SYS_WRITE,
    SYS_MALLOC,
    SYS_FREE,
    SYS_FORK,
    SYS_READ,
    SYS_PUTCHAR,
    SYS_CLEAR,
    SYS_GETCWD,
    SYS_OPEN,
    SYS_CLOSE,
    SYS_LSEEK,
    SYS_UNLINK,
    SYS_MKDIR,
    SYS_OPENDIR,
    SYS_CLOSEDIR,
    SYS_CHDIR,
    SYS_RMDIR,
    SYS_READDIR,
    SYS_REWINDDIR,
    SYS_STAT,
    SYS_PS,
};

uint32_t getpid(void);
uint32_t write(int32_t fd, const void *buf, uint32_t count);
void *malloc(uint32_t size);
void free(void *ptr);
pid_t fork(void);
int32_t read(int32_t fd, void *buf, uint32_t count);
/* Clears the screen */
void clear(void);
/* Outputs a character */
void putchar(char char_asci);
char *getcwd(char *buf, uint32_t size);
int32_t open(char *pathname, uint8_t flag);
int32_t close(int32_t fd);
int32_t lseek(int32_t fd, int32_t offset, uint8_t whence);
int32_t unlink(const char *pathname);
int32_t mkdir(const char *pathname);
Dir *opendir(const char *name);
int32_t closedir(Dir *dir);
int32_t rmdir(const char *pathname);
DirEntry *readdir(Dir *dir);
void rewinddir(Dir *dir);
int32_t stat(const char *path, Stat*buf);
int32_t chdir(const char *path);
void ps(void);
#endif
