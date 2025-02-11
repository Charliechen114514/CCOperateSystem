#ifndef __USERPROG_EXEC_H
#define __USERPROG_EXEC_H
#include "include/library/types.h"
#include "include/utils/fast_utils.h"
int32_t sys_execv(const char* path, const char*  argv[]);
#endif
