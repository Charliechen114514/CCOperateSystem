#ifndef __USERPROG_TSS_H
#define __USERPROG_TSS_H
#include "thread.h"
void update_tss_esp(TaskStruct* pthread);
void tss_init(void);
#endif
