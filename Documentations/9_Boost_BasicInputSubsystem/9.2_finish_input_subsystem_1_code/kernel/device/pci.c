#include "include/device/pci.h"
#include "include/io/io.h"
#include "include/library/ccos_print.h"

// PCI ports for controlling and data transfer
#define PCI_MASTER_CONTROL_PORT     (0x20)  // Port for the master PIC control
#define PCI_MASTER_DATA_PORT        (0x21)  // Port for the master PIC data
#define PCI_SLAVE_CONTROL_PORT      (0xa0)  // Port for the slave PIC control
#define PCI_SLAVE_DATA_PORT         (0xa1)  // Port for the slave PIC data

// Initialize the PCI interrupt controller
void pci_init()
{
    verbose_ccputs("    initing pci issue!\n");  // Debug message indicating the start of PCI initialization
    
    // Initialize the master PIC (Programmable Interrupt Controller) with ICW1
    outb(PCI_MASTER_CONTROL_PORT, 0x11);   // Send ICW1: initialization command word 1
    outb(PCI_MASTER_DATA_PORT, 0x20);      // Send ICW2: vector offset for interrupts from the master PIC
    outb(PCI_MASTER_DATA_PORT, 0x04);      // Send ICW3: slave connection (master to slave IRQ2)
    outb(PCI_MASTER_DATA_PORT, 0x01);      // Send ICW4: final initialization command (non-buffered mode)

    // Initialize the slave PIC with ICW1
    outb(PCI_SLAVE_CONTROL_PORT, 0x11);    // Send ICW1 for the slave PIC
    outb(PCI_SLAVE_DATA_PORT, 0x28);       // Send ICW2 for the slave PIC (vector offset for interrupts from the slave)
    outb(PCI_SLAVE_DATA_PORT, 0x02);       // Send ICW3 for the slave PIC (slave PIC is connected to IRQ2 of master)
    outb(PCI_SLAVE_DATA_PORT, 0x01);       // Send ICW4 for the slave PIC (non-buffered mode)

    // Mask interrupts to disable all IRQs
    outb(PCI_MASTER_DATA_PORT, 0xfd);      // Mask all IRQs on the master PIC (set bit 0)
    outb(PCI_SLAVE_DATA_PORT, 0xff);       // Mask all IRQs on the slave PIC (set all bits)

    verbose_ccputs("    init pci issue done!\n");  // Debug message indicating PCI initialization is complete
}
