#include "thread.h"   //函数声明 各种结构体
#include "stdint.h"   //前缀
#include "string.h"   //memset
#include "global.h"   //不清楚
#include "memory.h"   //分配页需要
#include "debug.h"
#include "interrupt.h"
#include "print.h"
#include "list.h"

#define PG_SIZE 4096

struct task_struct* main_thread;   // 主线程PCB
struct list thread_ready_list;     // 就绪队列
struct list thread_all_list;       // 所有任务队列
static struct list_elem* thread_tag;// 用于保存队列中的线程结点

extern void switch_to(struct task_struct* cur,struct task_struct* next);

/* 获取当前pcb指针 */
struct task_struct* running_thread(){
    uint32_t esp;
    asm("mov %%esp,%0":"=g"(esp));
    /* 取asm整数部分，即pcb起始地址 */
    return (struct task_struct*)(esp & 0xfffff000);
}

/* 由kernel_thread去执行function(func_arg) */
void kernel_thread(thread_func* function,void* func_arg){
    /* 执行function前要开中断，避免后面的时钟中断被屏蔽，而无法调度其它进程 */
    intr_enable();
    function(func_arg);
}

/* 初始化线程栈thread_stack，将待执行的函数和参数方法到thread_stack中相应的位置 */
void thread_create(struct task_struct* pthread,thread_func function,void* func_arg){
    /* 先预留中断使用栈的空间，可见thread.h中定义的结构 */
    pthread->self_kernel_stack-=sizeof(struct intr_stack);

    /* 再留出线程栈空间，可见thread.h中定义 */
    pthread->self_kernel_stack-=sizeof(struct thread_stack);
    struct thread_stack* kthread_stack=(struct thread_stack*)pthread->self_kernel_stack;
    kthread_stack->eip=kernel_thread;
    kthread_stack->function=function;
    kthread_stack->func_arg=func_arg;
    kthread_stack->ebp=kthread_stack->ebx=kthread_stack->esi=kthread_stack->edi=0;
}

/* 初始化线程基本信息 */
void init_thread(struct task_struct* pthread,char* name,int prio){
    memset(pthread,0,sizeof(*pthread));
    strcpy(pthread->name,name);

    if (pthread==main_thread){
    /* 由于把main函数也封装成一个线程，并且它一直是运行的，故将其直接设为TASK_RUNNING */
    pthread->status=TASK_RUNNING;
    }else{
    pthread->status=TASK_READY;
    }

    /* self_kernel_stack是线程自己在内核态下使用的栈顶地址 */
    pthread->self_kernel_stack=(uint32_t*)((uint32_t)pthread+PG_SIZE);
    pthread->priority=prio;
    pthread->ticks=prio;
    pthread->elapsed_ticks=0;
    pthread->pgdir=NULL;
    pthread->stack_magic=STACK_MAGIC;   // 自定义魔数
}

/* 创建一个优先级为prio的线程，线程名为name，线程所执行的函数是funciton(func_arg) */
struct task_struct* thread_start(char* name,int prio,thread_func function,void* func_arg){
    /* pcb都位于内核空间，包括用户进程pcb也是在内核空间 */
    struct task_struct* thread=get_kernel_pages(1);

    init_thread(thread,name,prio);
    thread_create(thread,function,func_arg);

    /* 确保之前不再队列中 */
    ASSERT(!elem_find(&thread_ready_list,&thread->general_tag));
    /* 加入就绪队列 */
    list_append(&thread_ready_list,&thread->general_tag);

    /* 确保之前不再队列中 */
    ASSERT(!elem_find(&thread_all_list,&thread->all_list_tag));
    list_append(&thread_all_list,&thread->all_list_tag);

    return thread;
}

/* 将kernel中的main函数完善为主线程 */
static void make_main_thread(void){
    /* 因为main线程早已运行，咱们在loader.S中进入内核时的mov esp,0xc009f000,
     * 就是为其预留pcb的，因此pcb地址为0xc009e000，不需要get_kernel_page另分配一页 */
    main_thread=running_thread();
    init_thread(main_thread,"main",31);
    put_str("\n");
    put_int(main_thread);
    put_str("\n");
    put_int(&main_thread->all_list_tag);
    put_str("\n");
    /* main函数是当前线程，当前线程不在thread_ready_list中，
     * 所以只将其加在thead_all_list中 */
    ASSERT(!elem_find(&thread_all_list,&main_thread->all_list_tag));
    list_append(&thread_all_list,&main_thread->all_list_tag);
}

/* 实现任务调度 */
void schedule(){
    ASSERT(intr_get_status()==INTR_OFF);
    struct task_struct* cur=running_thread();
    if (cur->status==TASK_RUNNING){
    // 若此线程只是cpu时间片到了，将其加入就绪队列队尾
    ASSERT(!elem_find(&thread_ready_list,&cur->general_tag));
    list_append(&thread_ready_list,&cur->general_tag);
    cur->ticks=cur->priority;
    // 重新将当前进程的ticks重置为priority
    cur->status=TASK_READY;
    }else{
    /* 若此线程需要某事件发生后才能继续上cpu运行，
     * 不需要将其加入队列，因为当前不在就绪队列中 */
    }

    ASSERT(!list_empty(&thread_ready_list));
    thread_tag=NULL;   // thread_tag清空
    /* 将thread_ready_list队列中的第一个就绪线程弹出，准备将其调度上cpu */
    thread_tag=list_pop(&thread_ready_list);
    struct task_struct* next=elem2entry(struct task_struct,general_tag,thread_tag);
    next->status=TASK_RUNNING;
    switch_to(cur,next);
}

/* 初始化线程环境 */
void thread_init(void){
    put_str("thread_init start\n");
    list_init(&thread_ready_list);
    list_init(&thread_all_list);
    /* 将当前main函数创建为线程 */
    make_main_thread();
    put_str("thread_init done\n");
}