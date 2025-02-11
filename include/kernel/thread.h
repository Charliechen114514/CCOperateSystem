#ifndef THREAD_H
#define THREAD_H
#include "include/library/list.h"
#include "include/library/types.h"
#include "include/memory/memory.h"
#include "include/memory/memory_settings.h"
#define TASK_NAME_ARRAY_SZ          (16)
#define TASK_MAGIC                  (0x11451419)
#define MAX_FILES_OPEN_PER_PROC     (8)

typedef void (*TaskFunction)(void *);
typedef int16_t pid_t;

typedef enum {
    TASK_RUNNING,
    TASK_READY,
    TASK_BLOCKED,
    TASK_WAITING,
    TASK_HANGING,
    TASK_DIED
} TaskStatus;

typedef struct {
    uint32_t vec_no;
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp_dummy;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t gs;
    uint32_t fs;
    uint32_t es;
    uint32_t ds;

    /* from low to high level */
    uint32_t err_code; // err_code会被压入在eip之后
    void (*eip)(void);
    uint32_t cs;
    uint32_t eflags;
    void *esp;
    uint32_t ss;
} Interrupt_Stack;

typedef struct {
    uint32_t ebp;
    uint32_t ebx;
    uint32_t edi;
    uint32_t esi;

    /* 线程第一次执行时,eip指向待调用的函数kernel_thread
    其它时候,eip是指向switch_to的返回地址*/
    void (*eip)(TaskFunction func, void *func_arg);

    /*****   以下仅供第一次被调度上cpu时使用   ****/
    /* 参数_ret只为占位置充数为返回地址 */
    void(*_retaddr);
    TaskFunction function; // 由Kernel_thread所调用的函数名
    void *func_arg;        // 由Kernel_thread所调用的函数所需的参数
} ThreadStack;

/* 进程或线程的pcb,程序控制块 */
typedef struct {
    uint32_t* self_kstack;	 // 各内核线程都用自己的内核栈
    pid_t pid;
    TaskStatus status;
    char name[TASK_NAME_ARRAY_SZ];
    uint8_t priority;
    uint8_t ticks;	   // 每次在处理器上执行的时间嘀嗒数
 /* 此任务自上cpu运行后至今占用了多少cpu嘀嗒数,
  * 也就是此任务执行了多久*/
    uint32_t elapsed_ticks;
 /* general_tag的作用是用于线程在一般的队列中的结点 */
    list_elem general_tag;				    
 /* all_list_tag的作用是用于线程队列thread_all_list中的结点 */
    list_elem all_list_tag;
    uint32_t* pgdir;              // 进程自己页表的虚拟地址
    VirtualMemoryHandle     userprog_vaddr;   // 用户进程的虚拟地址
    MemoryBlockDescriptor   u_block_desc[DESC_CNT];   // 用户进程内存块描述符
    int32_t fd_table[MAX_FILES_OPEN_PER_PROC];	// 已打开文件数组
    uint32_t cwd_inode_nr;	 // 进程所在的工作目录的inode编号
    pid_t parent_pid;		 // 父进程pid
    int8_t  exit_status;         // 进程结束时自己调用exit传入的参数
    uint32_t stack_magic;	 // 用这串数字做栈的边界标记,用于检测栈的溢出
 }TaskStruct;
 
 extern list thread_ready_list;
 extern list thread_all_list;
 
 void thread_create(TaskStruct* pthread, TaskFunction function, void* func_arg);
 void init_thread(TaskStruct* pthread, char* name, int prio);
 TaskStruct* thread_start(char* name, int prio, TaskFunction function, void* func_arg);
 TaskStruct* running_thread(void);
 void schedule(void);
 void thread_init(void);
 void thread_block(TaskStatus stat);
 void thread_unblock(TaskStruct* pthread);
 void thread_yield(void);
 pid_t fork_pid(void);
 void sys_ps(void);
 void thread_exit(TaskStruct* thread_over, bool need_schedule);
 TaskStruct* pid2thread(int32_t pid);
 void release_pid(pid_t pid);

#endif
