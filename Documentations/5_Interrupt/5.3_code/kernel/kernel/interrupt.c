#include "include/kernel/interrupt.h"
#include "include/library/types.h"
#include "include/library/ccos_print.h"
#include "include/settings.h"
#include "include/property/selectors.h"
#include "include/device/pci.h"
#include "include/library/bit_tools.h"

// The number of entries in the IDT (Interrupt Descriptor Table)
#define IDT_DESC_CNT (0x21)

typedef void *intr_functor; // Type for interrupt function pointers

// GateDescriptor structure represents the interrupt descriptor entry
// The structure defines how the interrupt descriptor is laid out in memory
#pragma pack(push, 1)
typedef struct
{
    uint16_t func_offset_low_word;        // Low 16 bits of the function address
    uint16_t selector;                    // Selector for code segment
    uint8_t dcount;                       // Number of arguments for the function
    uint8_t attribute;                    // Attributes (DPL, type of gate)
    uint16_t func_offset_high_word;       // High 16 bits of the function address
} __attribute__((packed)) GateDescriptor; // Ensure the structure is packed (no padding)
#pragma pack(pop)

// Declare a function to initialize an interrupt descriptor
static void make_idt(GateDescriptor *desp, uint8_t attr, intr_functor func);

// Declare an IDT table to store the interrupt descriptors
static GateDescriptor idt_table[IDT_DESC_CNT];

// Declare the vector table of interrupt functions (defined in interrupt.S)
extern intr_functor asm_intr_vector_table[IDT_DESC_CNT];

// Function to initialize a single IDT descriptor
static void make_idt(GateDescriptor *desp, uint8_t attr, intr_functor func)
{
    // Fill in the gate descriptor with the function address and other attributes
    desp->func_offset_low_word = LOW_16BITS(func);   // Lower 16 bits of function address
    desp->func_offset_high_word = HIGH_16BITS(func); // Upper 16 bits of function address
    desp->dcount = 0;                                // No arguments for the interrupt function
    desp->attribute = attr;                          // Set the gate attributes (e.g., privilege level)
    desp->selector = SELECTOR_K_CODE;                // Selector for the kernel code segment
}

// Function to initialize all IDT descriptors
static void idt_desp_init(void)
{
    verbose_ccputs("idt descriptor is initing as expected...\n");

    // Loop through all interrupt descriptors and initialize them
    for (uint16_t i = 0; i < IDT_DESC_CNT; i++)
    {
        make_idt(&idt_table[i], IDT_DESC_ATTR_DPL0, asm_intr_vector_table[i]);
    }

    verbose_ccputs("idt descriptor is initing done!\n");
}

// Macro to compose the value to load into the IDT register
#define COMPOSE_LIDT_VALUE() (                                          \
    ((uint64_t)(uint32_t)idt_table << 16) | /* IDT base address */      \
    (sizeof(idt_table) - 1)                 /* Size of the IDT table */ \
)

// Function to load the IDT into the IDT register (LIDT instruction)
static void load_idt(void)
{
    uint64_t idt_op = COMPOSE_LIDT_VALUE();
    asm volatile("lidt %0" ::"m"(idt_op)); // Load the IDT
}

// Conditional compilation for viewing the IDT structure in memory
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

// Function to initialize the global interrupt descriptor table (IDT)
void init_idt(void)
{
    verbose_ccputs("global interrupt is initing\n");

    // Initialize the exception handlers
    idt_desp_init();

    // Initialize PCI devices (used for interrupt handling in some cases)
    pci_init();

    // Load the IDT into the IDT register
    load_idt();

    // Optionally, view the structure of the IDT in memory (for debugging)
#ifdef VIEW_IDT_STRUTURE
    view_idt_struct_memory_mappings();
#endif

    verbose_ccputs("global interrupt done!\n");
}

// Function to enable interrupts globally (using the STI instruction)
void sti(void)
{
    asm volatile("sti"); // Set the interrupt flag (enable interrupts)
}

// Function to disable interrupts globally (using the CLI instruction)
void cli(void)
{
    asm volatile("cli"); // Clear the interrupt flag (disable interrupts)
}
