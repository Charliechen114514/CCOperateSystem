#include "include/kernel/interrupt.h"
#include "include/io/io.h"
#include "include/kernel/kernel_table_property.h"
#include "include/library/ccos_print.h"
#include "include/library/types.h"

#include "include/kernel/interrupt.h"
#include "include/library/bitmap.h"
#include "include/library/ccos_print.h"
#include "include/library/kernel_assert.h"
#include "include/library/string.h"
#include "include/library/types.h"

/* PIC control and data port definitions for 8259A */
#define PIC_M_CTRL 0x20 // Master PIC control port
#define PIC_M_DATA 0x21 // Master PIC data port
#define PIC_S_CTRL 0xa0 // Slave PIC control port
#define PIC_S_DATA 0xa1 // Slave PIC data port

#define IDT_DESC_CNT 0x81    // Total number of supported interrupts
#define PAGE_FAULT_NUM (14)  // Page Fault interrupt number
#define EFLAGS_IF 0x00000200 // EFLAGS interrupt flag (IF bit)
#define GET_EFLAGS(EFLAG_VAR) \
    asm volatile("pushfl; \
      popl %0"   \
                 : "=g"(EFLAG_VAR))

extern uint32_t syscall_handler(void); // External syscall handler function

/* Interrupt Gate Descriptor Structure */
struct gate_desc {
    uint16_t func_offset_low_word;  // Low 16 bits of the function offset
    uint16_t selector;              // Code segment selector
    uint8_t dcount;                 // Double word count (fixed, unused here)
    uint8_t attribute;              // Descriptor attributes
    uint16_t func_offset_high_word; // High 16 bits of the function offset
};

static void make_idt_desc(
    struct gate_desc *p_gdesc, uint8_t attr,
    intr_func_ptr function); // Static function to create IDT descriptor
static struct gate_desc
    idt[IDT_DESC_CNT]; // IDT is the interrupt descriptor table, essentially an
                       // array of interrupt gate descriptors

char *interrupt_name[IDT_DESC_CNT]; // Array to store names of exceptions

/********    Define interrupt handler array    ********
 * The interrupt handler entry points are defined in kernel.S.
 * Eventually, the functions in idt_table are called by these entry points.
 */
intr_func_ptr idt_table[IDT_DESC_CNT]; // Array of interrupt handlers
/********************************************/

extern intr_func_ptr
    intr_entry_table[IDT_DESC_CNT]; // Declare the interrupt entry function
                                    // array defined in kernel.S

/* Initialize the 8259A Programmable Interrupt Controller */
static void pic_init(void) {
    /* Initialize master PIC */
    outb(PIC_M_CTRL, 0x11); // ICW1: Edge trigger, cascade mode, requires ICW4
    outb(PIC_M_DATA, 0x20); // ICW2: Start interrupt vector 0x20, IR[0-7] will
                            // correspond to 0x20 ~ 0x27
    outb(PIC_M_DATA, 0x04); // ICW3: IR2 connected to slave PIC
    outb(PIC_M_DATA, 0x01); // ICW4: 8086 mode, normal EOI (End of Interrupt)

    /* Initialize slave PIC */
    outb(PIC_S_CTRL, 0x11); // ICW1: Edge trigger, cascade mode, requires ICW4
    outb(PIC_S_DATA, 0x28); // ICW2: Start interrupt vector 0x28, IR[8-15] will
                            // correspond to 0x28 ~ 0x2F
    outb(PIC_S_DATA, 0x02); // ICW3: Connect slave to master PIC through IR2
    outb(PIC_S_DATA, 0x01); // ICW4: 8086 mode, normal EOI

    /* Enable IRQ2 for cascading slave PIC; IRQ0 (timer), IRQ1 (keyboard), and
     * IRQ2 must be enabled on the master PIC */
    outb(PIC_M_DATA, 0xf8);

    /* Enable IRQ14 for hard disk controller on slave PIC */
    outb(PIC_S_DATA, 0xbf);

    verbose_write("   pic_init done\n");
}

/* Create an interrupt gate descriptor */
static void make_idt_desc(struct gate_desc *p_gdesc, uint8_t attr,
                          intr_func_ptr function) {
    p_gdesc->func_offset_low_word =
        (uint32_t)function & 0x0000FFFF; // Low 16 bits of the function address
    p_gdesc->selector = SELECTOR_K_CODE; // Selector for kernel code segment
    p_gdesc->dcount = 0;                 // Double word count (not used here)
    p_gdesc->attribute = attr; // Set attributes (such as privilege level)
    p_gdesc->func_offset_high_word = ((uint32_t)function & 0xFFFF0000) >>
                                     16; // High 16 bits of the function address
}

/* Initialize the Interrupt Descriptor Table (IDT) */
static void idt_desc_init(void) {
    verbose_write("   initing idts...\n");
    for (int i = 0; i < IDT_DESC_CNT; i++) {
        make_idt_desc(
            &idt[i], IDT_DESC_ATTR_DPL0,
            intr_entry_table[i]); // Set up IDT for kernel-level interrupts
    }
    /* Special handling for system calls, which have a DPL of 3 (user-level) */
    make_idt_desc(&idt[IDT_DESC_CNT - 1], IDT_DESC_ATTR_DPL3,
                  syscall_handler); // Set up the syscall interrupt handler
    verbose_write("   idt_desc_init done\n");
}

/* General interrupt handler, typically used for handling exceptions */
static void general_intr_handler(uint8_t vec_nr) {
    // IRQ7 and IRQ15 produce spurious interrupts that do not need handling
    if (vec_nr == 0x27 ||
        vec_nr == 0x2f) { // 0x2f is the last IRQ on the slave 8259A, reserved
        return;           // No need to handle IRQ7 or IRQ15 spurious interrupts
    }

    ccos_puts("\n\n");
    ccos_puts("----------- Exceptions occurs! ---------------\n");

    ccos_puts(interrupt_name[vec_nr]); // Display the name of the interrupt

    ccos_puts("\n\n");
    if (vec_nr ==
        PAGE_FAULT_NUM) { // If it's a page fault, print the missing address
        int page_fault_vaddr = 0;
        asm("movl %%cr2, %0"
            : "=r"(page_fault_vaddr)); // CR2 holds the address causing the page
                                       // fault
        ccos_puts("\npage fault occurs, with addr is 0x");
        __ccos_display_int(page_fault_vaddr);
        ccos_puts("\n\n");
    }
    ccos_puts("----------- Exceptions Message End! ---------------\n");

    // Once inside the interrupt handler, the interrupt is disabled,
    // so the following infinite loop cannot be interrupted.
    while (1)
        ;
}

/* Initialize general interrupt handler and register exception names */
static void exception_init(void) {
    int i;
    for (i = 0; i < IDT_DESC_CNT; i++) {
        // By default, all interrupts are handled by general_intr_handler
        idt_table[i] = general_intr_handler;
        interrupt_name[i] = "unknown"; // Default name is "unknown"
    }
    // Register specific exception names
    interrupt_name[0] = "#DE Divide Error";
    interrupt_name[1] = "#DB Debug Exception";
    interrupt_name[2] = "NMI Interrupt";
    interrupt_name[3] = "#BP Breakpoint Exception";
    interrupt_name[4] = "#OF Overflow Exception";
    interrupt_name[5] = "#BR BOUND Range Exceeded Exception";
    interrupt_name[6] = "#UD Invalid Opcode Exception";
    interrupt_name[7] = "#NM Device Not Available Exception";
    interrupt_name[8] = "#DF Double Fault Exception";
    interrupt_name[9] = "Coprocessor Segment Overrun";
    interrupt_name[10] = "#TS Invalid TSS Exception";
    interrupt_name[11] = "#NP Segment Not Present";
    interrupt_name[12] = "#SS Stack Fault Exception";
    interrupt_name[13] = "#GP General Protection Exception";
    interrupt_name[14] = "#PF Page-Fault Exception";
    // interrupt_name[15] is reserved by Intel, not used
    interrupt_name[16] = "#MF x87 FPU Floating-Point Error";
    interrupt_name[17] = "#AC Alignment Check Exception";
    interrupt_name[18] = "#MC Machine-Check Exception";
    interrupt_name[19] = "#XF SIMD Floating-Point Exception";
}

/* Enable interrupts and return the previous interrupt status */
Interrupt_Status enable_intr() {
    Interrupt_Status old_status;
    if (INTR_ON == intr_get_status()) {
        old_status = INTR_ON; // Interrupts are already enabled
        return old_status;
    } else {
        old_status = INTR_OFF; // Interrupts are disabled
        asm volatile("sti");   // Enable interrupts with STI instruction
        return old_status;
    }
}

/* Disable interrupts and return the previous interrupt status */
Interrupt_Status disable_intr() {
    Interrupt_Status old_status;
    if (INTR_ON == intr_get_status()) {
        old_status = INTR_ON; // Interrupts are already enabled
        asm volatile("cli"
                     :
                     :
                     : "memory"); // Disable interrupts with CLI instruction
        return old_status;
    } else {
        old_status = INTR_OFF; // Interrupts are already disabled
        return old_status;
    }
}

/* Set the interrupt state to the specified status */
Interrupt_Status set_intr_state(Interrupt_Status status) {
    return status & INTR_ON ? enable_intr() : disable_intr();
}

/* Get the current interrupt state */
Interrupt_Status intr_get_status() {
    uint32_t eflags = 0;
    GET_EFLAGS(eflags);
    return (EFLAGS_IF & eflags) ? INTR_ON : INTR_OFF;
}

/* Register a specific interrupt handler for the given interrupt vector number
 */
void register_intr_handler(uint8_t vector_no, intr_func_ptr function) {
    // Update the IDT entry for the given interrupt vector with the handler
    // function
    idt_table[vector_no] = function;
}

/* Initialize the interrupt descriptor table (IDT) and related components */
void idt_init() {
    verbose_write("idt_init start\n");
    idt_desc_init();  // Initialize the interrupt descriptor table
    exception_init(); // Initialize exception names and register default
                      // handlers
    pic_init();       // Initialize the 8259A PIC

    // Load the IDT into the CPU
    uint64_t idt_operand =
        ((sizeof(idt) - 1) | ((uint64_t)(uint32_t)idt << 16));
    asm volatile("lidt %0" : : "m"(idt_operand));
    verbose_write("idt_init done\n");
}
