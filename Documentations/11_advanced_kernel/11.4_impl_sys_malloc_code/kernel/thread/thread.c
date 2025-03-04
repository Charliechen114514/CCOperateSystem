#include "include/thread/thread.h"
#include "include/library/string.h"
#include "include/kernel/interrupt.h"
#include "include/memory/memory.h"
#include "include/library/ccos_print.h"
#include "include/library/kernel_assert.h"
#include "include/memory/memory_settings.h"
#include "include/user/process/process.h"
#include "include/kernel/lock.h"

// Pointer to the main thread structure
static TaskStruct *main_thread;
// Pointer to the thread tag used in scheduling
static list_elem *thread_tag;
// List of threads that are ready to run
list thread_ready_list;
// List of all threads in the system
list thread_all_list;

CCLocker pid_lock;  // Lock for pid allocation

// Function declaration for switching between tasks
extern void switch_to(TaskStruct *cur, TaskStruct *next);

/**
 * @brief Retrieve the currently running thread.
 * This function gets the current thread structure by reading the stack pointer (ESP)
 * and aligning it to the page boundary.
 *
 * @return Pointer to the currently running TaskStruct.
 */
TaskStruct *current_thread(void)
{
    uint32_t esp;
    asm volatile("mov %%esp, %0" : "=g"(esp));
    return (TaskStruct*)(esp & PG_FETCH_OFFSET);
}


static pid_t allocate_pid(void) { 
    static pid_t next_pid = 0; 
    lock_acquire(&pid_lock); 
    next_pid++; 
    lock_release(&pid_lock); 
    return next_pid; 
} 

/**
 * @brief Wrapper function to enable interrupts and execute a kernel thread function.
 * 
 * This function ensures that interrupts are enabled before calling the specified
 * thread function with its argument.
 * 
 * @param function The function to be executed in the kernel thread.
 * @param func_arg Argument passed to the function.
 */
static void kernel_thread(TaskFunction function, void *func_arg)
{
    set_intr_status(INTR_ON); // Enable interrupts
    function(func_arg); // Execute the function
}

/**
 * @brief Initialize and register the main thread.
 * 
 * This function sets up the main thread, initializes it, and adds it to the global
 * thread list.
 */
static void make_main_thread(void)
{
    main_thread = current_thread(); // Retrieve the current running thread
    init_thread(main_thread, "main", 31); // Initialize the main thread with priority 31
    KERNEL_ASSERT(!elem_find(&thread_all_list, &main_thread->all_list_tag)); // Ensure it is not already in the list
    list_append(&thread_all_list, &main_thread->all_list_tag); // Add the main thread to the thread list
}

/**
 * @brief Set up the initial stack structure for a new thread.
 *
 * This function prepares the stack of the given thread by reserving space for
 * an interrupt stack and a thread stack. It sets up the thread stack structure
 * to execute the kernel_thread function when the thread starts.
 *
 * @param pthread Pointer to the thread structure.
 * @param function The function the thread will execute.
 * @param func_arg Argument to be passed to the function.
 */
void create_thread(TaskStruct *pthread, TaskFunction function, void *func_arg)
{
    pthread->self_kstack -= sizeof(Interrupt_Stack); // Reserve space for interrupt stack
    pthread->self_kstack -= sizeof(ThreadStack); // Reserve space for thread stack
    ThreadStack *kthread_stack = (ThreadStack *)pthread->self_kstack; // Set thread stack pointer
    kthread_stack->eip = kernel_thread; // Set entry point to kernel_thread
    kthread_stack->function = function; // Set function to execute
    kthread_stack->func_arg = func_arg; // Set function argument
    kthread_stack->ebp = 0;
    kthread_stack->ebx = 0;
    kthread_stack->esi = 0;
    kthread_stack->edi = 0; // Initialize general-purpose registers to 0
}

/**
 * @brief Initialize a thread structure with default values.
 * 
 * This function sets up a new thread, including its name, status, priority,
 * stack, and other necessary fields.
 * 
 * @param pthread Pointer to the thread structure.
 * @param name Name of the thread.
 * @param priority Priority of the thread.
 */
void init_thread(TaskStruct *pthread, char *name, int priority)
{
    k_memset(pthread, 0, sizeof(TaskStruct)); // Clear the thread structure
    k_strcpy(pthread->name, name); // Set thread name
    pthread->pid = allocate_pid();
    if (pthread == main_thread)
    {
        pthread->status = TASK_RUNNING; // Main thread is always running
    }
    else
    {
        pthread->status = TASK_READY; // Other threads start as ready
    }

    pthread->priority = priority; // Set thread priority
    pthread->ticks = priority; // Set time slice based on priority
    pthread->elapsed_ticks = 0; // Reset elapsed ticks
    pthread->pg_dir = NULL; // No separate page directory for threads
    pthread->self_kstack = (uint32_t *)((uint32_t)pthread + PG_SIZE); // Set initial stack pointer
    pthread->stack_magic = TASK_MAGIC; // Set stack magic number for stack overflow detection
}

/**
 * @brief Task scheduling function.
 * 
 * This function implements simple task scheduling by selecting the next thread
 * to run from the ready list and performing a context switch.
 */
void schedule(void)
{
    KERNEL_ASSERT(get_intr_status() == INTR_OFF); // Ensure interrupts are disabled

    TaskStruct *cur = current_thread(); // Get the current running thread
    if (cur->status == TASK_RUNNING)
    {
        KERNEL_ASSERT(!elem_find(&thread_ready_list, &cur->general_tag)); // Ensure it's not already in the list
        list_append(&thread_ready_list, &cur->general_tag); // Add it to the ready list
        cur->ticks = cur->priority; // Reset thread's time slice based on priority
        cur->status = TASK_READY; // Set thread status to ready
    }

    KERNEL_ASSERT(!list_empty(&thread_ready_list)); // Ensure there is a next thread to run
    thread_tag = NULL; // Clear the thread tag
    
    // Pop the next thread from the ready list
    thread_tag = list_pop(&thread_ready_list);
    TaskStruct *next = elem2entry(TaskStruct, general_tag, thread_tag);
    next->status = TASK_RUNNING; // Set the next thread as running
    process_activate(next);
    switch_to(cur, next); // Perform context switch to the next thread
}

/* Block the current thread, marking its status as stat */
void thread_block(TaskStatus stat) {
    /* stat must be one of TASK_BLOCKED, TASK_WAITING, or TASK_HANGING */
    KERNEL_ASSERT(((stat == TASK_BLOCKED) || (stat == TASK_WAITING) ||
            (stat == TASK_HANGING)));
    
    // Disable interrupts and save the previous interrupt status
    Interrupt_Status old_status = set_intr_status(INTR_OFF);
    
    // Get the current running thread
    TaskStruct *cur_thread = current_thread();
    
    // Set the current thread's status to the given stat
    cur_thread->status = stat;  
    
    // Schedule another thread to run
    schedule();                 
    
    // Restore the previous interrupt state
    set_intr_status(old_status); 
}

/* Unblock a thread */
void thread_unblock(TaskStruct *pthread) {
    // Disable interrupts and save the previous interrupt status
    Interrupt_Status old_status = set_intr_status(INTR_OFF);
    
    // Ensure the thread is currently blocked, waiting, or hanging
    KERNEL_ASSERT(((pthread->status == TASK_BLOCKED) ||
            (pthread->status == TASK_WAITING) ||
            (pthread->status == TASK_HANGING)));
    
    // If the thread is not already ready, move it to the ready list
    if (pthread->status != TASK_READY) {
        KERNEL_ASSERT(!elem_find(&thread_ready_list, &pthread->general_tag)); // Ensure the thread is not already in the ready list
        
        // Ensure the thread is not in the ready list, if it is, panic
        if (elem_find(&thread_ready_list, &pthread->general_tag)) {
            KERNEL_PANIC_SPIN("thread_unblock: blocked thread in ready_list\n");
        }
        
        // Add the thread to the front of the ready list
        list_push(&thread_ready_list, &pthread->general_tag); 
        
        // Set the thread status to ready
        pthread->status = TASK_READY;     
    }
    
    // Restore the previous interrupt state
    set_intr_status(old_status); 
}

/* Start a new thread */
TaskStruct *thread_start(
    char *name, int priority,
    TaskFunction function, void *func_arg)
{
    // Allocate memory for the thread and initialize it
    TaskStruct *thread = get_kernel_pages(PCB_SZ_PG_CNT);
    init_thread(thread, name, priority);
    
    // Create the thread and set up its stack
    create_thread(thread, function, func_arg);
    
    // Ensure the thread is not already in the ready list
    KERNEL_ASSERT(!elem_find(&thread_ready_list, &thread->general_tag));
    
    // Add the thread to the ready list
    list_append(&thread_ready_list, &thread->general_tag);
    
    // Ensure the thread is not already in the all list
    KERNEL_ASSERT(!elem_find(&thread_all_list, &thread->all_list_tag));
    
    // Add the thread to the all thread list
    list_append(&thread_all_list, &thread->all_list_tag);
    
    return thread;  // Return the created thread
}

/* Initialize the thread subsystem */
void thread_subsystem_init(void)
{
    ccos_puts("Thread SubSystem initing...\n");
    
    // Initialize the ready and all thread lists
    list_init(&thread_ready_list);
    list_init(&thread_all_list);
    lock_init(&pid_lock);
    // Create the main thread
    make_main_thread();
    
    ccos_puts("Thread SubSystem init done\n");
}
