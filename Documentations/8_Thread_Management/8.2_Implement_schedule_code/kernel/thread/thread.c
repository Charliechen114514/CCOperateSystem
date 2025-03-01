#include "include/thread/thread.h"
#include "include/library/string.h"
#include "include/kernel/interrupt.h"
#include "include/memory/memory.h"
#include "include/library/ccos_print.h"
#include "include/library/kernel_assert.h"
#include "include/memory/memory_settings.h"

static TaskStruct *main_thread;
static list_elem *thread_tag;
list thread_ready_list;
list thread_all_list;

extern void switch_to(TaskStruct *cur, TaskStruct *next);

TaskStruct *running_thread(void)
{
    uint32_t esp;
    asm volatile("mov %%esp, %0" : "=g"(esp));
    return (TaskStruct*)(esp & PG_FETCH_OFFSET);
}

static void kernel_thread(TaskFunction function, void *func_arg)
{
    set_intr_status(INTR_ON);
    function(func_arg);
}

static void make_main_thread(void)
{
    main_thread = running_thread();
    init_thread(main_thread, "main", 31);
    KERNEL_ASSERT(!elem_find(&thread_all_list, &main_thread->all_list_tag));
    list_append(&thread_all_list, &main_thread->all_list_tag);
}

void create_thread(TaskStruct *pthread, TaskFunction function, void *func_arg)
{
    pthread->self_kstack -= sizeof(Interrupt_Stack);
    pthread->self_kstack -= sizeof(ThreadStack);
    ThreadStack *kthread_stack = (ThreadStack *)pthread->self_kstack;
    kthread_stack->eip = kernel_thread;
    kthread_stack->function = function;
    kthread_stack->func_arg = func_arg;
    kthread_stack->ebp = 0;
    kthread_stack->ebx = 0;
    kthread_stack->esi = 0;
    kthread_stack->edi = 0;
}

void init_thread(TaskStruct *pthread, char *name, int priority)
{
    k_memset(pthread, 0, sizeof(TaskStruct));
    k_strcpy(pthread->name, name);

    if (pthread == main_thread)
    {
        pthread->status = TASK_RUNNING;
    }
    else
    {
        pthread->status = TASK_READY;
    }

    pthread->priority = priority;
    pthread->ticks = priority;
    pthread->elapsed_ticks = 0;
    pthread->pg_dir = NULL;
    pthread->self_kstack = (uint32_t *)((uint32_t)pthread + PG_SIZE);
    pthread->stack_magic = TASK_MAGIC;
}

/* Task scheduling function */
void schedule(void)
{
    KERNEL_ASSERT(get_intr_status() == INTR_OFF);

    TaskStruct *cur = running_thread(); // Get the current running thread
    if (cur->status == TASK_RUNNING)
    { // If the current thread is still running
        KERNEL_ASSERT(!elem_find(&thread_ready_list, &cur->general_tag));
        list_append(&thread_ready_list,
                    &cur->general_tag); // Add it to the ready list
        cur->ticks =
            cur->priority;        // Reset the thread's ticks based on priority
        cur->status = TASK_READY; // Set the thread status to ready
    }

    KERNEL_ASSERT(!list_empty(&thread_ready_list));
    thread_tag = NULL; // Clear the thread_tag
    /* Pop the first thread from the ready list to schedule */
    thread_tag = list_pop(&thread_ready_list);
    TaskStruct *next = elem2entry(TaskStruct, general_tag, thread_tag);
    next->status = TASK_RUNNING;

    switch_to(cur, next); // Switch to the next thread
}

TaskStruct *thread_start(
    char *name, int priority,
    TaskFunction function, void *func_arg)
{
    TaskStruct *thread = get_kernel_pages(PCB_SZ_PG_CNT);
    init_thread(thread, name, priority);
    create_thread(thread, function, func_arg);
    KERNEL_ASSERT(!elem_find(&thread_ready_list, &thread->general_tag));
    list_append(&thread_ready_list, &thread->general_tag);
    KERNEL_ASSERT(!elem_find(&thread_all_list, &thread->all_list_tag));
    list_append(&thread_all_list, &thread->all_list_tag);
    return thread;
}

void thread_init(void)
{
    ccos_puts("Thread SubSystem initing...\n");
    list_init(&thread_ready_list);
    list_init(&thread_all_list);
    make_main_thread();
    ccos_puts("Thread SubSystem init done\n");
}