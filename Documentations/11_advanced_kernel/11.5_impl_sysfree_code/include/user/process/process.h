#ifndef __USERPROG_PROCESS_H 
#define __USERPROG_PROCESS_H 
#include "include/thread/thread.h"
#include "include/library/types.h"
#include "include/tools/math_tools.h"

#define DEFAULT_PRIO 31
#define USER_STACK3_VADDR  (0xc0000000 - PG_SIZE)
void process_execute(void* filename, char* name);
void start_process(void* filename_);
void process_activate(TaskStruct* p_thread);
void page_dir_activate(TaskStruct* p_thread);
uint32_t* create_page_dir(void);
void create_user_vaddr_bitmap(TaskStruct* user_prog);
#endif
