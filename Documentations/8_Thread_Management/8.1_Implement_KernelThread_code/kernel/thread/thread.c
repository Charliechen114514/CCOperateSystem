#include "include/thread/thread.h"
#include "include/library/string.h"
#include "include/memory/memory.h"
#include "include/memory/memory_settings.h"

/**
 * @brief Kernel thread entry function.
 * 
 * This function serves as the entry point for a new kernel thread.
 * It directly calls the provided task function with its argument.
 * 
 * @param function The task function to be executed by the thread.
 * @param func_arg The argument to be passed to the task function.
 */
static void kernel_thread(TaskFunction function, void *func_arg)
{
    function(func_arg);
}

/**
 * @brief Create a new kernel thread.
 * 
 * This function initializes the thread stack and prepares the execution
 * environment for the new thread.
 * 
 * @param pthread Pointer to the TaskStruct representing the thread.
 * @param function The function to be executed by the thread.
 * @param func_arg The argument to be passed to the function.
 */
static void create_thread(TaskStruct *pthread, TaskFunction function, void *func_arg)
{
    // Allocate space for the interrupt stack
    pthread->self_kstack -= sizeof(Interrupt_Stack);

    // Allocate space for the thread stack
    pthread->self_kstack -= sizeof(ThreadStack);

    // Set up the thread stack
    ThreadStack *kthread_stack = (ThreadStack *)pthread->self_kstack;
    kthread_stack->eip = kernel_thread;  // Entry point for the thread
    kthread_stack->function = function;  // Task function
    kthread_stack->func_arg = func_arg;  // Argument for the function
    kthread_stack->ebp = 0;
    kthread_stack->ebx = 0;
    kthread_stack->esi = 0;
    kthread_stack->edi = 0;
}

/**
 * @brief Initialize a thread structure.
 * 
 * This function initializes the thread control block with the provided
 * parameters and sets up the default values.
 * 
 * @param pthread Pointer to the TaskStruct to be initialized.
 * @param name The name of the thread.
 * @param priority The priority level of the thread.
 */
static void init_thread(TaskStruct *pthread, char *name, int priority)
{
    k_memset(pthread, 0, sizeof(TaskStruct));  // Clear the structure
    k_strcpy(pthread->name, name);  // Set thread name
    pthread->status = TASK_RUNNING;  // Set the status as running
    pthread->priority = priority;  // Assign priority
    pthread->self_kstack = (uint32_t *)((uint32_t)pthread + PG_SIZE);  // Set stack pointer
    pthread->stack_magic = TASK_MAGIC;  // Assign magic value for stack integrity check
}

/**
 * @brief Start a new thread.
 * 
 * This function allocates a new kernel thread, initializes it, and sets up
 * the stack for execution. It then switches to the new thread context.
 * 
 * @param name The name of the thread.
 * @param priority The priority level of the thread.
 * @param function The function to be executed by the thread.
 * @param func_arg The argument to be passed to the function.
 * @return Pointer to the created TaskStruct.
 */
TaskStruct *thread_start(
    char *name, int priority,
    TaskFunction function, void *func_arg)
{
    // Allocate memory for the thread control block
    TaskStruct* thread = get_kernel_pages(PCB_SZ_PG_CNT);
    // Initialize the thread structure
    init_thread(thread, name, priority);

    // Set up the thread stack
    create_thread(thread, function, func_arg);
    asm volatile ("movl %0, %%esp; \
        pop %%ebp; \
        pop %%ebx; \
        pop %%edi; \
        pop %%esi; \
        ret": : "g" (thread->self_kstack) : "memory");
    return thread;
}


