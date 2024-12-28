#ifndef _THREAD_THREAD_H
#define _THREAD_THREAD_H
#include "stdint.h"
#include "list.h"
#define STACK_MAGIC 0x00114514

typedef void thread_func(void*);

enum task_status{
    TASK_RUNNING,
    TASK_READY,
    TASK_BLOCKED,
    TASK_WAITING,
    TASK_HANGING,
    TASK_DIED
};

// INTR_STACK
struct intr_stack
{
     uint32_t vec_no; //中断号
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
    
    uint32_t err_code;
    void (*eip) (void);        
    uint32_t cs;
    uint32_t eflags;
    void* esp;
    uint32_t ss;
};

/*	线程栈 保护线程环境 */

struct thread_stack
{
    uint32_t ebp;
    uint32_t ebx;
    uint32_t edi;
    uint32_t esi;

    void (*eip) (thread_func* func,void* func_arg); //和下面的相互照应 以ret 汇编代码进入kernel_thread函数调用
    
    void (*unused_retaddr);                         //占位数 在栈顶站住了返回地址的位置 因为是汇编ret 
    
    thread_func* function;                          //进入kernel_thread要调用的函数地址
    
    void* func_arg;				      //参数指针
};

struct task_struct
{
    uint32_t* self_kernel_stack;          //pcb中的 kernel_stack 内核栈
    enum task_status status;              //线程状态
    uint8_t priority;				      //特权级
    uint8_t ticks;
    uint32_t elapsed_ticks;
    char name[16];
    struct list_elem general_tag;
    struct list_elem all_list_tag;
    uint32_t* pgdir;
    uint32_t stack_magic;			      //越界检查  因为我们pcb上面的就是我们要用的栈了 到时候还要越界检查

};

void thread_create(struct task_struct* pThread, thread_func func, void* func_arg);
void init_thread(struct task_struct* pThread, char* name, int priority);
struct task_struct* thread_start(char* name, int priority, thread_func func, void* func_args);
struct task_struct* running_thread(void);
void kernel_thread(thread_func* function,void* func_arg);
static void make_main_thread(void);
void schedule(void);
void thread_init(void);

#endif