#include "include/device/console_tty.h"
#include "include/kernel/interrupt.h"
#include "include/kernel/lock.h"
#include "include/library/ccos_print.h"
#include "include/library/kernel_assert.h"
#include "include/library/list.h"
#include "include/library/string.h"
#include "include/library/types.h"
#include "include/memory/memory.h"
#include "include/memory/memory_settings.h"
#include "include/thread/thread.h"
#include "include/user/program/process.h"
#include "include/user/stdio/stdio.h"

/* Bitmap for pid, supporting up to 1024 pids */
uint8_t pid_bitmap_bits[128] = {0};

/* pid pool structure */
CCLocker pid_lock;  // Lock for pid allocation

static TaskStruct *main_thread;      // Main thread PCB (Process Control Block)
static TaskStruct *idle_thread;      // Idle thread
list thread_ready_list;       // Ready thread list
list thread_all_list;         // All threads list
static list_elem *thread_tag; // Thread node in the list

extern void switch_to(TaskStruct *cur, TaskStruct *next);
extern void init(void);

/* Idle thread, runs when the system is idle */
static void idle(void *arg) {
    (void)arg; // maybe unused
    while (1) {
        thread_block(TASK_BLOCKED);
        // Ensure interrupts are enabled before executing hlt instruction
        asm volatile("sti; \
          hlt"
                     :
                     :
                     : "memory");
    }
}

/* Get the current thread's PCB pointer */
TaskStruct *running_thread() {
    uint32_t esp;
    asm("mov %%esp, %0" : "=g"(esp));
    /* Extract the integer part of esp, which is the starting address of the PCB
     */
    return (TaskStruct *)(esp & 0xfffff000);
}

/* Kernel thread to execute function(func_arg) */
static void kernel_thread(TaskFunction function, void *func_arg) {
    /* Enable interrupts before executing the function to avoid blocking clock
     * interrupts */
    set_intr_status(INTR_ON);
    function(func_arg);
}


/* Allocate a new pid */
static pid_t allocate_pid(void) { 
    static pid_t next_pid = 0; 
    lock_acquire(&pid_lock); 
    next_pid++; 
    lock_release(&pid_lock); 
    return next_pid; 
} 

/* Initialize the thread stack for a new thread, putting the function and
 * arguments in the stack */
void thread_create(TaskStruct *pthread, TaskFunction function, void *func_arg) {
    /* Reserve space for the interrupt stack */
    pthread->self_kstack -= sizeof(Interrupt_Stack);

    /* Reserve space for the thread stack */
    pthread->self_kstack -= sizeof(ThreadStack);
    ThreadStack *kthread_stack = (ThreadStack *)pthread->self_kstack;
    kthread_stack->eip = kernel_thread;
    kthread_stack->function = function;
    kthread_stack->func_arg = func_arg;
    kthread_stack->ebp = kthread_stack->ebx = kthread_stack->esi =
        kthread_stack->edi = 0;
}

/* Initialize basic thread information */
void init_thread(TaskStruct *pthread, char *name, int prio) {
    k_memset(pthread, 0, sizeof(*pthread));
    pthread->pid = allocate_pid(); // Assign a new pid
    k_strcpy(pthread->name, name);

    if (pthread == main_thread) {
        /* Main thread is set to TASK_RUNNING since it's always running */
        pthread->status = TASK_RUNNING;
    } else {
        pthread->status = TASK_READY;
    }

    /* Initialize the thread's kernel stack */
    pthread->self_kstack = (uint32_t *)((uint32_t)pthread + PGSIZE);
    pthread->priority = prio;
    pthread->ticks = prio; // Set the number of ticks based on priority
    pthread->elapsed_ticks = 0;
    pthread->pgdir = NULL;

    pthread->stack_magic =
        TASK_MAGIC; // Custom magic number for stack validation
}

/* Create a new thread with priority prio, name name, and function
 * function(func_arg) */
TaskStruct *thread_start(char *name, int prio, TaskFunction function,
                         void *func_arg) {
    /* PCBs are located in kernel space, including the user process's PCB */
    TaskStruct *thread =
        get_kernel_pages(1); // Allocate kernel memory for the thread
    init_thread(thread, name,
                prio); // Initialize thread with the given name and priority
    thread_create(thread, function,
                  func_arg); // Create thread and set up the stack for execution

    /* Ensure the thread is not already in the ready list */
    KERNEL_ASSERT(!elem_find(&thread_ready_list, &thread->general_tag));
    /* Add thread to the ready list */
    list_append(&thread_ready_list, &thread->general_tag);

    /* Ensure the thread is not already in the all threads list */
    KERNEL_ASSERT(!elem_find(&thread_all_list, &thread->all_list_tag));
    /* Add thread to the all threads list */
    list_append(&thread_all_list, &thread->all_list_tag);

    return thread; // Return the created thread
}

/* Initialize the main thread */
static void make_main_thread(void) {
    /* The main thread's PCB has already been allocated at 0xc009e000, no need
     * to allocate a new page */
    main_thread = running_thread();
    init_thread(main_thread, "main", 31); // Initialize the main thread

    /* The main thread is already running, add it to the all threads list */
    KERNEL_ASSERT(!elem_find(&thread_all_list, &main_thread->all_list_tag));
    list_append(&thread_all_list, &main_thread->all_list_tag);
}

/* Task scheduling function */
void schedule() {
    KERNEL_ASSERT(get_intr_status() == INTR_OFF);

    TaskStruct *cur = running_thread(); // Get the current running thread
    if (cur->status == TASK_RUNNING) { // If the current thread is still running
        KERNEL_ASSERT(!elem_find(&thread_ready_list, &cur->general_tag));
        list_append(&thread_ready_list,
                    &cur->general_tag); // Add it to the ready list
        cur->ticks =
            cur->priority;        // Reset the thread's ticks based on priority
        cur->status = TASK_READY; // Set the thread status to ready
    }

    /* If there are no tasks in the ready queue, unblock the idle thread */
    if (list_empty(&thread_ready_list)) {
        thread_unblock(idle_thread);
    }

    KERNEL_ASSERT(!list_empty(&thread_ready_list));
    thread_tag = NULL; // Clear the thread_tag
    /* Pop the first thread from the ready list to schedule */
    thread_tag = list_pop(&thread_ready_list);
    TaskStruct *next = elem2entry(TaskStruct, general_tag, thread_tag);
    next->status = TASK_RUNNING;

    /* Activate the process (e.g., set page tables, etc.) */
    process_activate(next);

    switch_to(cur, next); // Switch to the next thread
}

/* Block the current thread, marking its status as stat */
void thread_block(TaskStatus stat) {
    /* stat must be one of TASK_BLOCKED, TASK_WAITING, or TASK_HANGING */
    KERNEL_ASSERT(((stat == TASK_BLOCKED) || (stat == TASK_WAITING) ||
            (stat == TASK_HANGING)));
    Interrupt_Status old_status = set_intr_status(INTR_OFF);
    TaskStruct *cur_thread = running_thread();
    cur_thread->status = stat;  // Set the current thread's status
    schedule();                 // Schedule another thread
    set_intr_status(old_status); // Restore interrupt state
}

/* Unblock a thread */
void thread_unblock(TaskStruct *pthread) {
    Interrupt_Status old_status = set_intr_status(INTR_OFF);
    KERNEL_ASSERT(((pthread->status == TASK_BLOCKED) ||
            (pthread->status == TASK_WAITING) ||
            (pthread->status == TASK_HANGING)));
    if (pthread->status != TASK_READY) {
        KERNEL_ASSERT(!elem_find(&thread_ready_list, &pthread->general_tag));
        if (elem_find(&thread_ready_list, &pthread->general_tag)) {
            KERNEL_PANIC_SPIN("thread_unblock: blocked thread in ready_list\n");
        }
        list_push(&thread_ready_list,
                  &pthread->general_tag); // Push to the front of the ready list
        pthread->status = TASK_READY;     // Set the thread status to ready
    }
    set_intr_status(old_status); // Restore interrupt state
}

/* Yield the CPU voluntarily to another thread */
void thread_yield(void) {
    TaskStruct *cur = running_thread();
    Interrupt_Status old_status = set_intr_status(INTR_OFF);
    KERNEL_ASSERT(!elem_find(&thread_ready_list, &cur->general_tag));
    list_append(&thread_ready_list,
                &cur->general_tag); // Add the current thread to the ready list
    cur->status = TASK_READY;       // Set the current thread's status to ready
    schedule();                     // Schedule another thread
    set_intr_status(old_status);     // Restore interrupt state
}

/* Recycle the PCB and page table of a finished thread and remove it from the
 * scheduling queue */
void thread_exit(TaskStruct *thread_over, bool need_schedule) {
    /* Ensure interrupts are disabled before calling schedule */
    set_intr_status(INTR_OFF);
    thread_over->status = TASK_DIED; // Mark thread as died

    /* If thread is in the ready list, remove it */
    if (elem_find(&thread_ready_list, &thread_over->general_tag)) {
        list_remove(&thread_over->general_tag);
    }

    /* If the thread has a page table, free it */
    if (thread_over->pgdir) {
        mfree_page(PF_KERNEL, thread_over->pgdir, 1);
    }

    /* Remove the thread from the all threads list */
    list_remove(&thread_over->all_list_tag);

    /* Free the thread's PCB if it's not the main thread */
    if (thread_over != main_thread) {
        mfree_page(PF_KERNEL, thread_over, 1);
    }

    /* If needed, schedule another thread */
    if (need_schedule) {
        schedule();
        KERNEL_PANIC_SPIN(
            "thread_exit: should not be here\n"); // Panic if the control
                                                  // reaches here
    }
}

/* Compare the task's PID */
static bool pid_check(list_elem *pelem, int32_t pid) {
    TaskStruct *pthread = elem2entry(TaskStruct, all_list_tag, pelem);
    if (pthread->pid == pid) {
        return true; // Return true if the PID matches
    }
    return false;
}

/* Find a thread's PCB by PID. Return NULL if not found. */
TaskStruct *pid2thread(int32_t pid) {
    list_elem *pelem = list_traversal(&thread_all_list, pid_check, pid);
    if (pelem == NULL) {
        return NULL; // Return NULL if no thread is found with the specified PID
    }
    TaskStruct *thread = elem2entry(TaskStruct, all_list_tag, pelem);
    return thread; // Return the thread's PCB
}

/* Initialize the thread environment */
void thread_init(void) {
    verbose_ccputs("thread_init start\n");

    list_init(&thread_ready_list); // Initialize the ready list
    list_init(&thread_all_list);   // Initialize the all threads list
    lock_init(&pid_lock);

    /* Create the main thread */
    make_main_thread();

    /* Create the idle thread */
    idle_thread = thread_start("idle", 10, idle, NULL);

    verbose_ccputs("thread_init done\n");
}
