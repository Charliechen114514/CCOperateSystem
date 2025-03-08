#ifndef __USERPROG_PROCESS_H 
#define __USERPROG_PROCESS_H 
#include "include/thread/thread.h"
#include "include/library/types.h"
#include "include/utils/fast_utils.h"
#define DEFAULT_PRIO (31)
#define USER_STACK3_VADDR  (KERNEL_V_START - PG_SIZE)

void create_process(void* filename, char* name);

void activate_process_settings(TaskStruct* p_thread);
void page_dir_activate(TaskStruct* p_thread);
uint32_t* create_page_dir(void);

#endif
