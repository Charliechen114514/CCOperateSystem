#ifndef __KERNEL_SHELL_H
#define __KERNEL_SHELL_H
#include "include/filesystem/filesystem.h"
void print_prompt(void);
void ccshell(void);
extern char final_path[MAX_PATH_LEN];
#endif
