#include <stdio.h>

int main()
{
    printf("movl %%esp,%%eax\n\t" \
     "pushl $0x17\n\t" \
     "pushl %%eax\n\t" \
     "pushfl\n\t" \
     "pushl $0x0f\n\t" \
    "pushl $1f\n\t" \
     "iret\n" \
     "1:\tmovl $0x17,%%eax\n\t" \
     "movw %%ax,%%ds\n\t" \
     "movw %%ax,%%es\n\t" \
     "movw %%ax,%%fs\n\t" \
     "movw %%ax,%%gs" \
    );
}