#include "include/kernel/interrupt.h"
#include "include/library/types.h"
#include "include/library/ccos_print.h"
#include "include/settings.h"
#include "include/property/selectors.h"
#include "include/device/pci.h"
#include "include/library/bit_tools.h"

#define IDT_DESC_CNT (0x21) // The number of interrupt descriptors in the IDT

typedef void *intr_functor; // Define a function pointer type for interrupt handlers

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
static char *interrpt_name[IDT_DESC_CNT];

static void make_idt(GateDescriptor *desp, uint8_t attr, intr_functor func);

/*
    IDT Tables here, we soon load this into the idt registers
*/
static GateDescriptor idt_table[IDT_DESC_CNT];

// internal intr functors
intr_functor internal_exception_callback[IDT_DESC_CNT];
// see interrupt.S for details
extern intr_functor asm_intr_vector_table[IDT_DESC_CNT];

// Function to set up a default callback for exception interrupts
static void __def_exception_callback(uint8_t nvec)
{
    if(nvec == 0x27 || nvec == 0x2f) {  // Ignore certain interrupts
        return;
    }
    ccos_puts("receive interrupt: 0x");  // Print received interrupt
    __ccos_display_int(nvec);  // Display interrupt vector
    __ccos_putchar('\n');  // Print a newline
}

// Initialize the exceptions and set default callback functions
static void init_exceptions(void)
{
    uint16_t i = 0;
    for(i = 0; i < ARRAY_SIZE(INTERRUPT_NAME); i++){
        internal_exception_callback[i] = __def_exception_callback;  // Set default callback
        interrpt_name[i] = (char*)INTERRUPT_NAME[i];  // Set the exception name
    }

    // For undefined interrupts, set to a generic "Unknown Exception"
    for(i = ARRAY_SIZE(INTERRUPT_NAME); i < IDT_DESC_CNT; i++){
        internal_exception_callback[i] = __def_exception_callback;  // Set default callback
        interrpt_name[i] = "#Unknown Exception";  // Set unknown exception name
    }
}

// Helper function to populate the IDT GateDescriptor for an interrupt handler
static void make_idt(GateDescriptor* desp, uint8_t attr, intr_functor func)
{
    desp->func_offset_low_word = LOW_16BITS(func);  // Set the lower 16 bits of the function pointer
    desp->func_offset_high_word = HIGH_16BITS(func);  // Set the higher 16 bits of the function pointer
    desp->dcount = 0;  // No arguments, so dcount is 0
    desp->attribute = attr;  // Set the interrupt descriptor attributes
    desp->selector = SELECTOR_K_CODE;  // Set the code segment selector for kernel code
}

// Function to initialize the IDT descriptor table
static void idt_desp_init(void)
{
    verbose_ccputs("idt descriptor is initing as expected...\n");  // Debugging message
    for(uint16_t i = 0; i < IDT_DESC_CNT; i++){
        make_idt(&idt_table[i], IDT_DESC_ATTR_DPL0, asm_intr_vector_table[i]);  // Initialize each entry
    }
    verbose_ccputs("idt descriptor is initing done!\n");  // Debugging message
}

// Macro to compute the value for the LIDT instruction
#define COMPOSE_LIDT_VALUE() (\
    ((uint64_t)(uint32_t)idt_table << 16) | /* Place the address of the IDT table */ \
    (sizeof(idt_table) - 1) /* The size of the IDT table */\
)

// Function to load the IDT into the CPU's IDT register
static void load_idt(void)
{
    uint64_t idt_op = COMPOSE_LIDT_VALUE();  // Prepare the LIDT value
    asm volatile("lidt %0"::"m"(idt_op));  // Load the IDT register with the LIDT value
}

// Optional debug function to view the memory mappings of the IDT structure
#ifdef VIEW_IDT_STRUTURE
static void view_idt_struct_memory_mappings(void)
{
    verbose_ccputs("    so idt's should be like this:)\n");
    verbose_ccputs("        IDT_Base_ADDR: ");verbose_ccint((uint32_t)&idt_table[0]);verbose_ccputchar('\n');
    verbose_ccputs("        IDT_Base_low_func: ");verbose_ccint((uint32_t)&(idt_table[0].func_offset_low_word));verbose_ccputchar('\n');
    verbose_ccputs("        IDT_Base_selector: ");verbose_ccint((uint32_t)&(idt_table[0].selector));verbose_ccputchar('\n');
    verbose_ccputs("        IDT_Base_dcnt: ");verbose_ccint((uint32_t)&(idt_table[0].dcount));verbose_ccputchar('\n');
    verbose_ccputs("        IDT_Base_attr: ");verbose_ccint((uint32_t)&(idt_table[0].attribute));verbose_ccputchar('\n');
    verbose_ccputs("        IDT_Base_high_func: ");verbose_ccint((uint32_t)&(idt_table[0].func_offset_high_word));verbose_ccputchar('\n');
    verbose_ccputs("    done dumped!:)\n");
}
#endif

// Initialize the global interrupt system
void init_idt(void)
{
    verbose_ccputs("global interrupt is initing\n");  // Debugging message
    init_exceptions();  // Initialize exception handlers
    idt_desp_init();  // Set up the IDT descriptors
    pci_init();  // Initialize PCI devices (if applicable)
    load_idt();  // Load the IDT into the CPU
#ifdef VIEW_IDT_STRUTURE
    view_idt_struct_memory_mappings();  // Optional: View the IDT memory layout
#endif
    verbose_ccputs("global interrupt done!\n");  // Debugging message
}

// Enable interrupts globally by setting the IF flag in the EFLAGS register
void sti(void)
{
    asm volatile("sti");  // Enable interrupts
}

// Disable interrupts globally by clearing the IF flag in the EFLAGS register
void cli(void)
{
    asm volatile("cli");  // Disable interrupts
}