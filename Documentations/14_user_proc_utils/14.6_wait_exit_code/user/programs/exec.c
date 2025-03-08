#include "include/user/program/exec.h"
#include "include/io/stdio-kernel.h"
#include "include/filesystem/filesystem.h"
#include "include/thread/thread.h"
#include "include/library/string.h"
#include "include/library/types.h"
#include "include/memory/memory.h"
#include "include/memory/memory_settings.h"
#include "include/memory/memory_tools.h"

extern void intr_exit(void);
typedef uint32_t Elf32_Word, Elf32_Addr, Elf32_Off;
typedef uint16_t Elf32_Half;

/* 32-bit ELF header */
struct Elf32_Ehdr {
    unsigned char e_ident[16]; // ELF identification bytes
    Elf32_Half e_type;         // Type of file (e.g., executable)
    Elf32_Half e_machine;      // Machine architecture
    Elf32_Word e_version;      // ELF version
    Elf32_Addr e_entry;        // Entry point address
    Elf32_Off e_phoff;         // Program header offset
    Elf32_Off e_shoff;         // Section header offset
    Elf32_Word e_flags;        // Processor-specific flags
    Elf32_Half e_ehsize;       // ELF header size
    Elf32_Half e_phentsize;    // Program header entry size
    Elf32_Half e_phnum;        // Number of program headers
    Elf32_Half e_shentsize;    // Section header entry size
    Elf32_Half e_shnum;        // Number of section headers
    Elf32_Half e_shstrndx;     // Section header string table index
};

/* Program header (segment descriptor) */
struct Elf32_Phdr {
    Elf32_Word p_type;   // Segment type (e.g., PT_LOAD)
    Elf32_Off p_offset;  // Offset in file
    Elf32_Addr p_vaddr;  // Virtual address in memory
    Elf32_Addr p_paddr;  // Physical address (unused)
    Elf32_Word p_filesz; // Size of segment in file
    Elf32_Word p_memsz;  // Size of segment in memory
    Elf32_Word p_flags;  // Segment flags
    Elf32_Word p_align;  // Segment alignment
};

/* Segment types */
enum segment_type {
    PT_NULL,    // Ignore segment
    PT_LOAD,    // Loadable segment
    PT_DYNAMIC, // Dynamic loading information
    PT_INTERP,  // Name of dynamic loader
    PT_NOTE,    // Auxiliary information
    PT_SHLIB,   // Reserved
    PT_PHDR     // Program header
};

/* Load a segment from a file into virtual memory at the specified address */
static bool segment_load(int32_t fd, uint32_t offset, uint32_t filesz,
                         uint32_t vaddr) {
    uint32_t vaddr_first_page =
        vaddr & 0xfffff000; // First page of the virtual address
    uint32_t size_in_first_page =
        PG_SIZE - (vaddr & 0x00000fff); // Size of the segment in the first page
    uint32_t occupy_pages = 0;

    // If the segment doesn't fit in a single page
    if (filesz > size_in_first_page) {
        uint32_t left_size = filesz - size_in_first_page;
        occupy_pages = ROUNDUP(left_size, PG_SIZE) + 1; // +1 for the first page
    } else {
        occupy_pages = 1;
    }

    // Allocate memory for the segment in the process's address space
    uint32_t page_idx = 0;
    uint32_t vaddr_page = vaddr_first_page;
    while (page_idx < occupy_pages) {
        uint32_t *pde = pde_ptr(vaddr_page); // Page directory entry
        uint32_t *pte = pte_ptr(vaddr_page); // Page table entry

        // Allocate memory if PDE or PTE doesn't exist
        if (!(*pde & PG_P_1) || !(*pte & PG_P_1)) {
            if (!get_a_page(PF_USER, vaddr_page)) {
                return false;
            }
        }
        vaddr_page += PG_SIZE;
        page_idx++;
    }

    // Read the segment data from the file and load it into memory
    sys_lseek(fd, offset, SEEK_SET);
    sys_read(fd, (void *)vaddr, filesz);
    return true;
}

/* Load a user program from the filesystem by pathname, return entry point
 * address or -1 on failure */
static int32_t load(const char *pathname) {
    int32_t ret = -1;
    struct Elf32_Ehdr elf_header;
    struct Elf32_Phdr prog_header;
    k_memset(&elf_header, 0, sizeof(struct Elf32_Ehdr));

    int32_t fd = sys_open(pathname, O_RDONLY); // Open the program file
    if (fd == -1) {
        return -1;
    }

    // Read the ELF header from the file
    if (sys_read(fd, &elf_header, sizeof(struct Elf32_Ehdr)) !=
        sizeof(struct Elf32_Ehdr)) {
        ret = -1;
        goto done;
    }

    // Verify the ELF header
    if (k_memcmp(elf_header.e_ident, "\177ELF\1\1\1", 7) ||
        elf_header.e_type != 2 || elf_header.e_machine != 3 ||
        elf_header.e_version != 1 || elf_header.e_phnum > 1024 ||
        elf_header.e_phentsize != sizeof(struct Elf32_Phdr)) {
        ret = -1;
        goto done;
    }

    Elf32_Off prog_header_offset = elf_header.e_phoff;
    Elf32_Half prog_header_size = elf_header.e_phentsize;

    // Iterate over all program headers
    uint32_t prog_idx = 0;
    while (prog_idx < elf_header.e_phnum) {
        k_memset(&prog_header, 0, prog_header_size);

        // Seek to the program header location in the file
        sys_lseek(fd, prog_header_offset, SEEK_SET);

        // Read the program header from the file
        if (sys_read(fd, &prog_header, prog_header_size) != prog_header_size) {
            ret = -1;
            goto done;
        }

        // If the segment is loadable, load it into memory
        if (PT_LOAD == prog_header.p_type) {
            if (!segment_load(fd, prog_header.p_offset, prog_header.p_filesz,
                              prog_header.p_vaddr)) {
                ret = -1;
                goto done;
            }
        }

        // Move to the next program header
        prog_header_offset += elf_header.e_phentsize;
        prog_idx++;
    }

    ret = elf_header.e_entry; // Return the entry point of the program
done:
    sys_close(fd); // Close the file
    return ret;
}

/* Replace the current process with the program at the specified path */
int32_t sys_execv(const char *path, const char *argv[]) {
    uint32_t argc = 0;
    while (argv[argc]) {
        argc++; // Count the number of arguments
    }

    // Load the program and get its entry point
    int32_t entry_point = load(path);
    if (entry_point == -1) { // If loading failed, return -1
        return -1;
    }

    TaskStruct *cur = current_thread(); // Get the current running thread (process)
    k_memcpy(cur->name, path, TASK_NAME_ARRAY_SZ); // Update the process name

    // Update the stack with the arguments
    Interrupt_Stack *intr_0_stack =
        (Interrupt_Stack *)((uint32_t)cur + PG_SIZE - sizeof(Interrupt_Stack));
    intr_0_stack->ebx = (int32_t)argv;
    intr_0_stack->ecx = argc;
    intr_0_stack->eip = (void *)entry_point;
    intr_0_stack->esp = (void *)KERNEL_V_START; // Set stack pointer to the highest
                                            // user space address

    // Jump to the entry point of the new process
    asm volatile("movl %0, %%esp; jmp intr_exit"
                 :
                 : "g"(intr_0_stack)
                 : "memory");
    return 0;
}
