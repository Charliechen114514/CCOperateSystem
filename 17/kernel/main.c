# include "kernel/print.h"
# include "init.h"
# include "debug.h"
# include "string.h"
# include "memory.h"
int main(void) {
    init_all();

    void* addr = get_kernel_pages(3);
    put_str("fetch kernel page start virtual memory:\n");
    put_int((uint32_t)addr);
    put_str("\n");
    while (1);
    return 0;
}
