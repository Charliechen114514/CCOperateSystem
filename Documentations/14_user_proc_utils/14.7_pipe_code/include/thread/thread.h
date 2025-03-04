#ifndef THREAD_H
#define THREAD_H

#include "include/library/list.h"
#include "include/library/types.h"
#include "include/memory/memory.h"
#include "include/memory/memory_settings.h"

#define TASK_NAME_ARRAY_SZ (16)     // Maximum length of a thread name
#define TASK_MAGIC (0x11451419)     // Magic number for stack integrity check
#define MAX_FILES_OPEN_PER_PROC (8) // Maximum number of open files per process

/**
 * @brief Function pointer type for thread execution.
 *
 * A thread function should accept a single void pointer argument.
 */
typedef void (*TaskFunction)(void *);

/**
 * @brief Type definition for process IDs.
 */
typedef int16_t pid_t;

/**
 * @brief Enumeration of thread states.
 *
 * Represents the possible states a thread can be in during execution.
 */
typedef enum
{
    TASK_RUNNING, // Thread is currently executing
    TASK_READY,   // Thread is ready to execute but not running
    TASK_BLOCKED, // Thread is blocked and waiting for an event
    TASK_WAITING, // Thread is waiting for a resource
    TASK_HANGING, // Thread is suspended
    TASK_DIED     // Thread has terminated
} TaskStatus;

/**
 * @brief Structure for the interrupt stack.
 *
 * This structure stores the register values when an interrupt occurs.
 * It is used to save and restore the thread's execution context.
 */
typedef struct
{
    uint32_t vec_no; // Interrupt vector number
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp_dummy; // Placeholder for alignment
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t gs;
    uint32_t fs;
    uint32_t es;
    uint32_t ds;

    /* From low to high privilege level */
    uint32_t err_code; // Error code pushed by the processor
    void (*eip)(void); // Instruction pointer at the time of interrupt
    uint32_t cs;       // Code segment
    uint32_t eflags;   // CPU flags
    void *esp;         // Stack pointer
    uint32_t ss;       // Stack segment
} Interrupt_Stack;

/**
 * @brief Structure representing a thread's stack.
 *
 * Defines the stack layout for context switching.
 */
typedef struct
{
    uint32_t ebp;
    uint32_t ebx;
    uint32_t edi;
    uint32_t esi;

    /**
     * @brief Entry point for the thread.
     *
     * When the thread is first scheduled, this function is called
     * with the specified function and argument.
     */
    void (*eip)(TaskFunction func, void *func_arg);

    /* These fields are used only when the thread is first scheduled. */
    void(*_retaddr);       // Placeholder return address
    TaskFunction function; // Function to be executed by the thread
    void *func_arg;        // Argument to be passed to the function
} ThreadStack;

/**
 * @brief Process Control Block (PCB) structure for threads and processes.
 *
 * This structure stores the execution context, scheduling information,
 * and resources associated with a thread or process.
 */
typedef struct
{
    uint32_t *self_kstack;         // Pointer to the kernel stack of the thread
    pid_t pid;                     // Process ID
    TaskStatus status;             // Current status of the thread
    char name[TASK_NAME_ARRAY_SZ]; // Name of the thread
    uint8_t priority;              // Thread priority level
    uint8_t ticks;                 // Time slices allocated per execution cycle
    uint32_t elapsed_ticks;        // Total CPU time consumed

    /**
     * @brief General list element for scheduling queues.
     *
     * Used to manage the thread in general scheduling queues.
     */
    list_elem general_tag;

    /**
     * @brief List element for all threads.
     *
     * Used to track all threads in the system.
     */
    list_elem all_list_tag;

    uint32_t *pg_dir;                   // Virtual address of process page directory
    VirtualMemoryHandle userprog_vaddr; // User process virtual memory space
    MemoryBlockDescriptor
        u_block_desc[DESC_CNT];                // User process memory block descriptors
    int32_t fd_table[MAX_FILES_OPEN_PER_PROC]; // File descriptors of opened files
    uint32_t cwd_inode_nr;                     // Inode number of the current working directory
    pid_t parent_pid;      // Parent process ID
    int8_t exit_status;    // Exit status set by the process when terminating
    uint32_t stack_magic;                      // Stack boundary marker for overflow detection
} TaskStruct;

/* External lists for thread management */
extern list thread_ready_list; // List of ready threads
extern list thread_all_list;   // List of all threads

/**
 * @brief Create a new thread.
 *
 * Initializes the stack and sets up execution context for a new thread.
 *
 * @param pthread Pointer to the thread structure.
 * @param function Function to be executed by the thread.
 * @param func_arg Argument to be passed to the function.
 */
void thread_create(TaskStruct *pthread, TaskFunction function, void *func_arg);

/**
 * @brief Initialize a thread structure.
 *
 * Prepares a thread structure for execution.
 *
 * @param pthread Pointer to the thread structure.
 * @param name Name of the thread.
 * @param prio Priority of the thread.
 */
void init_thread(TaskStruct *pthread, char *name, int prio);

/**
 * @brief Start a new thread.
 *
 * Allocates memory, initializes, and starts a new thread.
 *
 * @param name Name of the thread.
 * @param prio Priority of the thread.
 * @param function Function to be executed.
 * @param func_arg Argument to be passed to the function.
 * @return Pointer to the created TaskStruct.
 */
TaskStruct *thread_start(char *name, int prio, TaskFunction function,
                         void *func_arg);

/**
 * @brief Get the currently running thread.
 *
 * Returns a pointer to the thread currently executing.
 *
 * @return Pointer to the currently running TaskStruct.
 */
TaskStruct *current_thread(void);

/**
 * @brief Schedule the next thread.
 *
 * Selects the next ready thread to execute.
 */
void schedule(void);

/**
 * @brief Initialize threading system.
 *
 * Sets up necessary structures and starts the first thread.
 */
void thread_init(void);

/**
 * @brief Block the current thread.
 *
 * Puts the current thread in a blocked state.
 *
 * @param stat New state of the thread (must be a blocking state).
 */
void thread_block(TaskStatus stat);

/**
 * @brief Unblock a thread.
 *
 * Moves a blocked thread to the ready state.
 *
 * @param pthread Pointer to the thread to be unblocked.
 */
void thread_unblock(TaskStruct *pthread);

/**
 * @brief Yield CPU execution.
 *
 * Temporarily stops execution of the current thread and schedules another.
 */
void thread_yield(void);

/**
 * @brief Generate a new process ID.
 *
 * Allocates and returns a unique process ID.
 *
 * @return New process ID.
 */
pid_t fork_pid(void);

/**
 * @brief Display process and thread information.
 *
 * Prints the status of all threads and processes.
 */
void sys_ps(void);

/**
 * @brief Terminate a thread.
 *
 * Ends a thread's execution and optionally schedules a new one.
 *
 * @param thread_over Pointer to the thread to be terminated.
 * @param need_schedule Whether to invoke the scheduler immediately.
 */
void thread_exit(TaskStruct *thread_over, bool need_schedule);

/**
 * @brief Convert process ID to thread structure.
 *
 * Retrieves the TaskStruct corresponding to a given process ID.
 *
 * @param pid Process ID to look up.
 * @return Pointer to the TaskStruct, or NULL if not found.
 */
TaskStruct *pid2thread(int32_t pid);


/* Release a pid */
void release_pid(pid_t pid);
#endif
