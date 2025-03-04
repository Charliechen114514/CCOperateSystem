#ifndef THREAD_H
#define THREAD_H
#include "include/library/types.h"
#include "include/thread/thread_settings.h"
#include "include/library/list.h"
#include "include/memory/memory.h"
#define TASK_NAME_ARRAY_SZ (16) // Maximum size for task name
#define TASK_MAGIC (0x11451419) // Magic number for stack integrity check

/* External lists for thread management */
extern list thread_ready_list; // List of ready threads
extern list thread_all_list;   // List of all threads

/**
 * @brief Function pointer type for thread execution.
 *
 * A thread function should accept a single void pointer argument.
 */
typedef void (*TaskFunction)(void *);

/**
 * @brief Enumeration for thread status.
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
 * @brief Interrupt stack structure.
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
 * @brief Thread stack structure.
 *
 * This structure defines the stack layout for a thread.
 * It is used for context switching.
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
     * When the thread is first scheduled, this function is called with
     * the specified function and argument.
     */
    void (*eip)(TaskFunction func, void *func_arg);

    /* These fields are used only when the thread is first scheduled. */
    void(*_retaddr);       // Placeholder return address
    TaskFunction function; // Function to be executed by the thread
    void *func_arg;        // Argument to be passed to the function
} ThreadStack;

/**
 * @brief Task control block structure.
 *
 * This structure represents a thread and stores its execution context.
 */
typedef struct __cctaskstruct
{
    uint32_t *self_kstack;         // Kernel stack pointer for the thread
    TaskStatus status;             // Current status of the thread
    char name[TASK_NAME_ARRAY_SZ]; // Thread name
    uint8_t priority;              // Thread priority level
    uint8_t ticks;
    uint32_t elapsed_ticks;
    list_elem general_tag;
    list_elem all_list_tag;
    uint32_t *pg_dir;
    VirtualAddressMappings  userprog_vaddr;
    uint32_t stack_magic; // Magic value for stack overflow detection
} TaskStruct;


/**
 * @brief Get the currently running thread.
 *
 * Returns a pointer to the thread currently executing.
 *
 * @return Pointer to the currently running TaskStruct.
 */
TaskStruct *current_thread(void);

void create_thread(TaskStruct *pthread, TaskFunction function, void *func_arg);
void init_thread(TaskStruct *pthread, char *name, int priority);
/**
 * @brief Start a new thread.
 *
 * This function creates and initializes a new thread, sets up its stack,
 * and prepares it for execution.
 *
 * @param name The name of the thread.
 * @param priority The priority level of the thread.
 * @param function The function to be executed by the thread.
 * @param func_arg The argument to be passed to the function.
 * @return Pointer to the created TaskStruct.
 */
TaskStruct *thread_start(
    char *name, int priority,
    TaskFunction function, void *func_arg);

/* Unblock a thread */
void thread_unblock(TaskStruct *pthread);
/* Block the current thread, marking its status as stat */
void thread_block(TaskStatus stat);

void schedule(void);
void thread_subsystem_init(void);
#endif