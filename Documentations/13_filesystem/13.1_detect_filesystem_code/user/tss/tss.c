#include "include/user/tss/tss.h"
#include "include/kernel/kernel_table_property.h"
#include "include/library/ccos_print.h"
#include "include/library/types.h"
#include "include/memory/memory_settings.h"
#include "include/library/string.h"

typedef struct {
    uint16_t limit_low_word;   // Lower 16 bits of limit
    uint16_t base_low_word;    // Lower 16 bits of base address
    uint8_t base_mid_byte;     // Middle 8 bits of base address
    uint8_t attr_low_byte;     // Low 8 bits of attributes
    uint8_t limit_high_attr_high; // Upper 4 bits of limit and high bits of attributes
    uint8_t base_high_byte;    // Upper 8 bits of base address
} GDT_Descriptor;

/* Task State Segment (TSS) structure */
struct tss {
    uint32_t backlink;         // Link to the previous task's TSS
    uint32_t *esp0;            // Stack pointer for privilege level 0
    uint32_t ss0;              // Stack segment for privilege level 0
    uint32_t *esp1;            // Stack pointer for privilege level 1
    uint32_t ss1;              // Stack segment for privilege level 1
    uint32_t *esp2;            // Stack pointer for privilege level 2
    uint32_t ss2;              // Stack segment for privilege level 2
    uint32_t cr3;              // Page directory base address
    uint32_t (*eip)(void);     // Instruction pointer (entry point of the task)
    uint32_t eflags;           // CPU flags
    uint32_t eax;              // General-purpose registers
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;              // Stack pointer for the task
    uint32_t ebp;              // Base pointer
    uint32_t esi;              // Source index
    uint32_t edi;              // Destination index
    uint32_t es;               // Extra segment
    uint32_t cs;               // Code segment
    uint32_t ss;               // Stack segment
    uint32_t ds;               // Data segment
    uint32_t fs;               // Additional segment
    uint32_t gs;               // Additional segment
    uint32_t ldt;              // Local descriptor table selector
    uint32_t trace;            // Trace flag for debugging
    uint32_t io_base;          // Base address for I/O permissions
};
static struct tss tss;

/* Update the esp0 field of the TSS to the stack pointer of the given task (pthread) */
void update_tss_esp(TaskStruct *pthread) {
    tss.esp0 = (uint32_t *)((uint32_t)pthread + PGSIZE);
}

/* Create a GDT descriptor */
static GDT_Descriptor make_gdt_desc(uint32_t *desc_addr, uint32_t limit,
                                    uint8_t attr_low, uint8_t attr_high) {
    uint32_t desc_base = (uint32_t)desc_addr;
    GDT_Descriptor desc;
    desc.limit_low_word = limit & 0x0000ffff;  // Set the lower 16 bits of the limit
    desc.base_low_word = desc_base & 0x0000ffff; // Set the lower 16 bits of the base address
    desc.base_mid_byte = ((desc_base & 0x00ff0000) >> 16); // Set middle 8 bits of the base
    desc.attr_low_byte = (uint8_t)(attr_low);   // Set the low byte of the attributes
    desc.limit_high_attr_high =
        (((limit & 0x000f0000) >> 16) + (uint8_t)(attr_high)); // Set the high byte of the limit and attributes
    desc.base_high_byte = desc_base >> 24;  // Set the high byte of the base address
    return desc;
}

/* Initialize the TSS in the GDT and reload the GDT */
void tss_init() {
    verbose_ccputs("tss_init start\n");
    uint32_t tss_size = sizeof(tss);
    k_memset(&tss, 0, tss_size);  // Clear the TSS structure
    tss.ss0 = SELECTOR_K_STACK; // Set the stack segment for privilege level 0
    tss.io_base = tss_size;     // Set the I/O base address to the size of TSS

    /* GDT base address is 0x900, place the TSS at the 4th position, i.e., 0x900 + 0x20 */
   /*
      Author's note:
         Sometimes compilers might dump! Try setting a virtual memory breakpoint
         to inspect the GDT base info and adjust the following defines accordingly
   */
#define GDT_ACTUAL_POSITION   (0xc0000900)
    /* Add a TSS descriptor with DPL (Descriptor Privilege Level) 0 in the GDT */
    *((GDT_Descriptor *)(GDT_ACTUAL_POSITION + 0x20)) = make_gdt_desc(
        (uint32_t *)&tss, tss_size - 1, TSS_ATTR_LOW, TSS_ATTR_HIGH);

    /* Add code and data segment descriptors with DPL 3 (user level) in the GDT */
    *((GDT_Descriptor *)(GDT_ACTUAL_POSITION + 0x28)) = make_gdt_desc(
        (uint32_t *)0, 0xfffff, GDT_CODE_ATTR_LOW_DPL3, GDT_ATTR_HIGH);
    *((GDT_Descriptor *)(GDT_ACTUAL_POSITION + 0x30)) = make_gdt_desc(
        (uint32_t *)0, 0xfffff, GDT_DATA_ATTR_LOW_DPL3, GDT_ATTR_HIGH);

    /* GDT 16-bit limit and 32-bit segment base address */
    uint64_t gdt_operand =
        ((8 * 7 - 1) | ((uint64_t)(uint32_t)GDT_ACTUAL_POSITION << 16)); // 7 descriptors total
    asm volatile("lgdt %0" : : "m"(gdt_operand));  // Load the GDT
    asm volatile("ltr %w0" : : "r"(SELECTOR_TSS)); // Load the TSS selector
    verbose_ccputs("tss_init and ltr done\n");
}
