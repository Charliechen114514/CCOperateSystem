#include "include/kernel/interrupt.h"
#include "include/library/types.h"
#include "include/library/ccos_print.h"
#include "include/settings.h"
#include "include/property/selectors.h"
#include "include/device/pci.h"
#include "include/library/bit_tools.h"
#include "include/library/kernel_assert.h"
#define IDT_DESC_CNT (0x81) // The number of interrupt descriptors in the IDT
#define PAGE_FAULT_NUM (14) // Page Fault interrupt number

/*
    Cautions: Don't use re-arrangement techniques
    to optimize the structures
    As expected, each descriptor needs to be 8 bytes aligned in memory.
*/
#pragma pack(push, 1) // Ensure that the struct is packed with 1-byte alignment
typedef struct
{
    uint16_t func_offset_low_word;        // Lower 16 bits of the interrupt handler function address
    uint16_t selector;                    // Code segment selector for the interrupt handler
    uint8_t dcount;                       // Number of arguments for the interrupt handler (unused here)
    uint8_t attribute;                    // Attribute byte (DPL, present flag, gate type, etc.)
    uint16_t func_offset_high_word;       // Upper 16 bits of the interrupt handler function address
} __attribute__((packed)) GateDescriptor; // Ensure the structure is tightly packed (no padding)
#pragma pack(pop)                         // Restore the previous alignment

extern uint32_t syscall_handler(void); // External syscall handler function

// Exception names array, describing each interrupt type
static const char *INTERRUPT_NAME[] = {
    "#DE Divide Error",
    "#DB Debug Exception",
    "#NMI Interrupt",
    "#BP Breakpoint Exception",
    "#OF Overflow Exception",
    "#BR BOUND Range Exceeded Exception",
    "#UD Invalid Opcode Exception",
    "#NM Device Not Available Exception",
    "#DF Double Fault Exception",
    "Coprocessor Segment Overrun",
    "#TS Invalid TSS Exception",
    "#NP Segment Not Present",
    "#SS Stack Fault Exception",
    "#GP General Protection Exception",
    "#PF Page-Fault Exception",
    "#Intel Reserved Exceptions",
    "#MF x87 FPU Floating-Point Error",
    "#AC Alignment Check Exception",
    "#MC Machine-Check Exception",
    "#XF SIMD Floating-Point Exception"};

// dynamics contains size
static char *interrupt_name[IDT_DESC_CNT];

/*
    IDT Tables here, we soon load this into the idt registers
*/
static GateDescriptor idt_table[IDT_DESC_CNT];

// Function to set up a default callback for exception interrupts
static void __def_exception_callback(uint8_t nvec)
{
    if (nvec == 0x27 || nvec == 0x2f)
    { // Ignore certain interrupts
        return;
    }
    ccos_puts("\n\n");
    ccos_puts("----------- Exceptions occurs! ---------------\n");
    ccos_puts("Man! Exception Occurs! :\n");
    ccos_puts(interrupt_name[nvec]); // Display the name of the interrupt
    ccos_puts("\nSee this fuck shit by debugging your code!\n");
    if (nvec ==
        PAGE_FAULT_NUM)
    { // If it's a page fault, print the missing address
        int page_fault_vaddr = 0;
        asm("movl %%cr2, %0"
            : "=r"(page_fault_vaddr)); // CR2 holds the address causing the page
                                       // fault
        ccos_puts("And this is sweety fuckingly page fault, happened with addr is 0x");
        __ccos_display_int(page_fault_vaddr);
        ccos_puts("\n\n");
    }
    ccos_puts("----------- Exceptions Message End! ---------------\n");

    // Once inside the interrupt handler, the interrupt is disabled,
    // so the following infinite loop cannot be interrupted.
    while (1)
        ;
}
// internal intr functors
intr_func_ptr internal_exception_callback[IDT_DESC_CNT];

void register_intr_handler(uint8_t nvec, intr_func_ptr func)
{
    // Update the IDT entry for the given interrupt vector with the handler
    // function
    internal_exception_callback[nvec] = func;
}

// see interrupt.S for details
extern intr_func_ptr asm_intr_vector_table[IDT_DESC_CNT];
/* Create an interrupt gate descriptor */
static void make_idt_desc(GateDescriptor *p_gdesc, uint8_t attr,
                          intr_func_ptr function)
{
    p_gdesc->func_offset_low_word =
        (uint32_t)function & 0x0000FFFF; // Low 16 bits of the function address
    p_gdesc->selector = SELECTOR_K_CODE; // Selector for kernel code segment
    p_gdesc->dcount = 0;                 // Double word count (not used here)
    p_gdesc->attribute = attr;           // Set attributes (such as privilege level)
    p_gdesc->func_offset_high_word = ((uint32_t)function & 0xFFFF0000) >>
                                     16; // High 16 bits of the function address
}

/*
    GET_EFLAGS(EFLAG_VAR) is a macro used to retrieve the current
    value of the EFLAGS register and store it in the variable EFLAG_VAR.

 Breakdown of the assembly code:

 1. `pushfl` -  This instruction pushes the current value of the
                EFLAGS register onto the stack.
 2. `popl %0` - This instruction pops the top value
                from the stack (which is the value of EFLAGS) into
                the specified variable (EFLAG_VAR in this case).

 The macro uses the `volatile` keyword to prevent the compiler from optimizing
 this code away, ensuring that the assembly code is executed as intended.

 Result: After calling this macro, the EFLAGS register's value will be stored in EFLAG_VAR.
*/
#define GET_EFLAGS(EFLAG_VAR) asm volatile(" \
    pushfl; \
    popl %0" : "=g"(EFLAG_VAR))

/* Initialize the Interrupt Descriptor Table (IDT) */
static void idt_desc_init(void)
{
    verbose_ccputs("   initing idts...\n");
    for (int i = 0; i < IDT_DESC_CNT; i++)
    {
        make_idt_desc(
            &idt_table[i], IDT_DESC_ATTR_DPL0,
            asm_intr_vector_table[i]); // Set up IDT for kernel-level interrupts
    }
    /* Special handling for system calls, which have a DPL of 3 (user-level) */
    make_idt_desc(&idt_table[IDT_DESC_CNT - 1], IDT_DESC_ATTR_DPL3,
                  syscall_handler); // Set up the syscall interrupt handler
    verbose_ccputs("   idt_desc_init done\n");
}

// Initialize the exceptions and set default callback functions
static void init_exceptions(void)
{
    uint16_t i = 0;
    for (i = 0; i < ARRAY_SIZE(INTERRUPT_NAME); i++)
    {
        internal_exception_callback[i] = __def_exception_callback; // Set default callback
        interrupt_name[i] = (char *)INTERRUPT_NAME[i];             // Set the exception name
    }

    // For undefined interrupts, set to a generic "Unknown Exception"
    for (i = ARRAY_SIZE(INTERRUPT_NAME); i < IDT_DESC_CNT; i++)
    {
        internal_exception_callback[i] = __def_exception_callback; // Set default callback
        interrupt_name[i] = "#Unknown Exception";                  // Set unknown exception name
    }
}


// Enable interrupts globally by setting the IF flag in the EFLAGS register
static inline void sti(void)
{
    asm volatile("sti"); // Enable interrupts
}

// Disable interrupts globally by clearing the IF flag in the EFLAGS register
static inline void cli(void)
{
    /*
        add memory as clobber that indicates compilers shouldn't
        optimaze the memory operation order, which is safer
        in multi-thread levels :)
    */
    asm volatile("cli" ::: "memory"); // Disable interrupts
}


/* Enable interrupts and return the previous interrupt status */
static Interrupt_Status enable_intr(void)
{
    Interrupt_Status old_status;
    if (INTR_ON == get_intr_status())
    {
        old_status = INTR_ON; // Interrupts are already enabled
        return old_status;
    }
    else
    {
        old_status = INTR_OFF; // Interrupts are disabled
        sti();   // Enable interrupts with STI instruction
        return old_status;
    }
}

/* Disable interrupts and return the previous interrupt status */
static Interrupt_Status disable_intr(void)
{
    Interrupt_Status old_status;
    if (INTR_ON == get_intr_status())
    {
        old_status = INTR_ON; // Interrupts are already enabled
        cli();
        return old_status;
    }
    else
    {
        old_status = INTR_OFF; // Interrupts are already disabled
        return old_status;
    }
}

/* Set the interrupt state to the specified status */
Interrupt_Status set_intr_status(Interrupt_Status status)
{
    return status & INTR_ON ? enable_intr() : disable_intr();
}


/*
EFLAGS (Status Flags Register) structure:

31      22 21 20 19 18 17 16 15 14 13 12 11 10  9  8
-------------------------------------------------------
| Reserved | IOPL | NT | DF | OF | I | T | S | Z |  A | P | C | Reserved |
-------------------------------------------------------

    EFLAGS_IF (0x00000200) corresponds to the Interrupt Flag (IF),
    which is the 9th bit in EFLAGS.
    When IF is set (1), interrupts are enabled.
    If it is cleared (0), interrupts are disabled.
    This value is used to manipulate the interrupt enable/disable
    state by directly modifying the IF flag.
*/

#define EFLAGS_IF (0x00000200)

/* Get the current interrupt state */
Interrupt_Status get_intr_status()
{
    uint32_t eflags = 0;
    GET_EFLAGS(eflags);
    return (EFLAGS_IF & eflags) ? INTR_ON : INTR_OFF;
}

// Optional debug function to view the memory mappings of the IDT structure
#ifdef VIEW_IDT_STRUTURE
static void view_idt_struct_memory_mappings(void)
{
    verbose_ccputs("    so idt's should be like this:)\n");
    verbose_ccputs("        IDT_Base_ADDR: ");
    verbose_ccint((uint32_t)&idt_table[0]);
    verbose_ccputchar('\n');
    verbose_ccputs("        IDT_Base_low_func: ");
    verbose_ccint((uint32_t)&(idt_table[0].func_offset_low_word));
    verbose_ccputchar('\n');
    verbose_ccputs("        IDT_Base_selector: ");
    verbose_ccint((uint32_t)&(idt_table[0].selector));
    verbose_ccputchar('\n');
    verbose_ccputs("        IDT_Base_dcnt: ");
    verbose_ccint((uint32_t)&(idt_table[0].dcount));
    verbose_ccputchar('\n');
    verbose_ccputs("        IDT_Base_attr: ");
    verbose_ccint((uint32_t)&(idt_table[0].attribute));
    verbose_ccputchar('\n');
    verbose_ccputs("        IDT_Base_high_func: ");
    verbose_ccint((uint32_t)&(idt_table[0].func_offset_high_word));
    verbose_ccputchar('\n');
    verbose_ccputs("    done dumped!:)\n");
}
#endif

// KERNEL_ASSERT checks for the size and offsets of the members
// of GateDescriptor structure
static inline void make_idt_check(void)
{
    // Check the size of the structure. It should be exactly 8 bytes (16 bits * 4 + 8 bits * 1)
    KERNEL_ASSERT(sizeof(GateDescriptor) == 8);

    // Check the offsets of the structure members manually.

    // func_offset_low_word should be at offset 0
    KERNEL_ASSERT((char *)&((GateDescriptor *)0)->func_offset_low_word == (char *)0);

    // selector should be at offset 2 (2 bytes after func_offset_low_word)
    KERNEL_ASSERT((char *)&((GateDescriptor *)0)->selector == (char *)2);

    // dcount should be at offset 4
    KERNEL_ASSERT((char *)&((GateDescriptor *)0)->dcount == (char *)4);

    // attribute should be at offset 5
    KERNEL_ASSERT((char *)&((GateDescriptor *)0)->attribute == (char *)5);

    // func_offset_high_word should be at offset 6
    KERNEL_ASSERT((char *)&((GateDescriptor *)0)->func_offset_high_word == (char *)6);
}

// Macro to compute the value for the LIDT instruction
#define COMPOSE_LIDT_VALUE() (                                                       \
    ((uint64_t)(uint32_t)idt_table << 16) | /* Place the address of the IDT table */ \
    (sizeof(idt_table) - 1)                 /* The size of the IDT table */          \
)

// Function to load the IDT into the CPU's IDT register
static inline void load_idt(void)
{
    uint64_t idt_op = COMPOSE_LIDT_VALUE(); // Prepare the LIDT value
    asm volatile("lidt %0" ::"m"(idt_op));  // Load the IDT register with the LIDT value
}

/* Initialize the interrupt descriptor table (IDT) and related components */
void init_idt()
{
    verbose_ccputs("idt_init start\n");
    idt_desc_init();   // Initialize the interrupt descriptor table
    init_exceptions(); // Initialize exception names and register default check the idt size
    make_idt_check();
    pci_init(); // Initialize the 8259A PIC

    load_idt();
    verbose_ccputs("idt_init done\n");
}
