#include "include/kernel/thread.h"
#include "include/kernel/interrupt.h"
#include "include/memory/memory.h"
#include "include/library/string.h"
#include "include/library/ccos_print.h"
#include "include/library/types.h"
#include "include/library/assert.h" // for ASSERT
#include "include/memory/memory_settings.h"

TaskStruct* main_thread;    // 主线程PCB
list thread_ready_list;	    // 就绪队列
list thread_all_list;	    // 所有任务队列
list_elem* thread_tag;      // 用于保存队列中的线程结点


// // defines in asms
extern void switch_to(TaskStruct* cur, TaskStruct* next);

// fetch current running task
TaskStruct* running_thread(void) {
    uint32_t esp; 
    asm ("mov %%esp, %0" \
       : "=g" (esp));
    /* 取esp整数部分即pcb起始地址 */
    return (TaskStruct*)(esp & 0xfffff000);
}

static void __kernel_thread_executor(TaskFunction func, void* args){
    enable_intr();
    func(args);
}


void create_thread(TaskStruct* pthread, TaskFunction func, void* args){
    pthread->kstack -= sizeof(Interrupt_Stack);
    pthread->kstack -= sizeof(ThreadStack);

    ThreadStack* kthread_stack = (ThreadStack*)(pthread->kstack);
    kthread_stack->eip = __kernel_thread_executor;
    kthread_stack->function = func;
    kthread_stack->func_arg = args;
    kthread_stack->ebp = \
        kthread_stack->ebx = \
        kthread_stack->esi = \
        kthread_stack->edi = 0;
}

void init_thread(TaskStruct* pthread, char* name, int prio) {
    memset(pthread, 0, sizeof(*pthread));
    strcpy(pthread->taskname, name);
    
    pthread->current_status = 
        (pthread == main_thread ? TASK_RUNNING : TASK_READY); 
    
    pthread->kstack = (uint32_t*)((uint32_t)pthread + PGSIZE);
    pthread->priority = prio;

    pthread->ticks_in_cpu   = prio;
    pthread->elapsed_ticks  = 0;
    pthread->pg_dir = NULL;

    pthread->stack_magic = TASK_MAGIC;	  // 自定义的魔数
 }
 
 /* 创建一优先级为prio的线程,线程名为name,线程所执行的函数是function(func_arg) */
TaskStruct* thread_start(
    char* name, int prio, TaskFunction function, void* func_arg) {
 /* pcb都位于内核空间,包括用户进程的pcb也是在内核空间 */
    TaskStruct* thread = get_kernel_pages(1);
    init_thread(thread, name, prio);
    create_thread(thread, function, func_arg);
    // test the direct run
    // asm volatile (  "movl %0, %%esp; \
    //                 pop %%ebp; \
    //                 pop %%ebx; \
    //                 pop %%edi; \
    //                 pop %%esi; \
    //                 ret" :: "g"(thread->kstack) : "memory");
    /* 确保之前不在队列中 */
    ASSERT(!elem_find(&thread_ready_list, &thread->general_tag));
    /* 加入就绪线程队列 */
    list_append(&thread_ready_list, &thread->general_tag);

    /* 确保之前不在队列中 */
    ASSERT(!elem_find(&thread_all_list, &thread->all_tags));
    /* 加入全部线程队列 */
    list_append(&thread_all_list, &thread->all_tags);
    return thread;
}

static void create_main_thread(void)
{
    main_thread = running_thread();
    init_thread(main_thread, "main", 31);
    ASSERT(!elem_find(&thread_all_list, &main_thread->all_tags));
    list_append(&thread_all_list, &main_thread->all_tags);
}

void schedule(void)
{
    ASSERT(get_intr_state() == INTR_OFF);

    TaskStruct* cur = running_thread(); 
    if (cur->current_status == TASK_RUNNING) { // 若此线程只是cpu时间片到了,将其加入到就绪队列尾
        ASSERT(!elem_find(&thread_ready_list, &cur->general_tag));
        list_append(&thread_ready_list, &cur->general_tag);
        cur->ticks_in_cpu = cur->priority;     // 重新将当前线程的ticks再重置为其priority;
        cur->current_status = TASK_READY;
    }

 
    ASSERT(!list_empty(&thread_ready_list));
    thread_tag = NULL;	  // thread_tag清空

    thread_tag = list_pop(&thread_ready_list);   
    TaskStruct t;
    TaskStruct* next = elem2entry(TaskStruct, general_tag, thread_tag);
    next->current_status = TASK_RUNNING;
 
    switch_to(cur, next);    
}

// block the thread himself
void thread_block(TaskStatus stat) {
    // hey! status must be at followings!
    ASSERT(((stat == TASK_BLOCKED) || (stat == TASK_WAITING) || (stat == TASK_HANGING)));
    Interrupt_Status old_status = disable_intr();
    TaskStruct* cur_thread = running_thread();
    cur_thread->current_status = stat;
    schedule();		      
    set_intr_state(old_status);
}
    
// unblock by using other threads
void thread_unblock(TaskStruct* pthread) {
    Interrupt_Status old_status = disable_intr();
    // easy to say, the status should be followings...
    ASSERT(((pthread->current_status == TASK_BLOCKED) 
            || (pthread->current_status == TASK_WAITING) 
            || (pthread->current_status == TASK_HANGING)));
    if (pthread->current_status != TASK_READY) {
        ASSERT(!elem_find(&thread_ready_list, &pthread->general_tag));
        if (elem_find(&thread_ready_list, &pthread->general_tag)) {
            KERNEL_PANIC("thread_unblock: blocked thread in ready_list\n");
        }
        list_push(&thread_ready_list, &pthread->general_tag);    // 放到队列的最前面,使其尽快得到调度
        pthread->current_status = TASK_READY;
    } 
    set_intr_state(old_status);
}

#include "include/library/ccos_print.h"
/* 初始化线程环境 */
void thread_init(void) {
    ccos_puts("thread schedule and init start!\n");
    list_init(&thread_ready_list);
    list_init(&thread_all_list);
    create_main_thread();
    ccos_puts("thread schedule and init done!\n");
}


