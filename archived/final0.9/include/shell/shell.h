#ifndef __KERNEL_SHELL_H
#define __KERNEL_SHELL_H
#include "include/filesystem/fs.h"
void print_prompt(void);
void ccos_baseshell(void);
extern char final_path[MAX_PATH_LEN];
#endif
