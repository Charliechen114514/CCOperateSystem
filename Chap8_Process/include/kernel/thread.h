#ifndef THREAD_H
#define THREAD_H
#include "include/library/types.h"
#include "include/library/list.h"
#define     TASK_NAME_ARRAY_SZ      (16)
#define     TASK_MAGIC              (0x11451419)

typedef void(*TaskFunction)(void*);

typedef enum {
    TASK_RUNNING,
    TASK_READY,
    TASK_BLOCKED,
    TASK_WAITING,
    TASK_HANGING,
    TASK_DIED
}TaskStatus;

typedef struct {
    uint32_t    vec_no;	
    uint32_t    edi;
    uint32_t    esi;
    uint32_t    ebp;
    uint32_t    esp_dummy;	 
    uint32_t    ebx;
    uint32_t    edx;
    uint32_t    ecx;
    uint32_t    eax;
    uint32_t    gs;
    uint32_t    fs;
    uint32_t    es;
    uint32_t    ds;

/* from low to high level */
    uint32_t    err_code;		 // err_code会被压入在eip之后
    void (*eip) (void);
    uint32_t    cs;
    uint32_t    eflags;
    void*       esp;
    uint32_t    ss;
}Interrupt_Stack;

typedef struct {
    uint32_t ebp;
    uint32_t ebx;
    uint32_t edi;
    uint32_t esi;
 
 /* 线程第一次执行时,eip指向待调用的函数kernel_thread 
 其它时候,eip是指向switch_to的返回地址*/
    void (*eip) (TaskFunction func, void* func_arg);
 
 /*****   以下仅供第一次被调度上cpu时使用   ****/
 /* 参数unused_ret只为占位置充数为返回地址 */
    void (*unused_retaddr);
    TaskFunction function;     // 由Kernel_thread所调用的函数名
    void* func_arg;             // 由Kernel_thread所调用的函数所需的参数
}ThreadStack;

typedef struct __TaskStruct{
    uint32_t*   kstack;
    TaskStatus  current_status;
    uint8_t     priority;
    char        taskname[TASK_NAME_ARRAY_SZ];
    uint8_t     ticks_in_cpu;
    uint32_t    elapsed_ticks;

    list_elem   general_tag;
    list_elem   all_tags;

    uint32_t*   pg_dir;

    uint32_t    stack_magic;    // detecting stack segment fault
}TaskStruct;

// for the kernel initializations
void thread_init(void);

void init_thread(TaskStruct* pthread, char* name, int prio);
void create_thread(TaskStruct* pthread, TaskFunction func, void* args);
TaskStruct* thread_start(
    char* name, int prio, TaskFunction function, void* func_arg);

TaskStruct* running_thread(void);

// thread self block and unblocks
void thread_block(TaskStatus stat);
void thread_unblock(TaskStruct* pthread);

// other part call this for the schedule
void schedule(void);

#endif