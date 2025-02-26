#ifndef KERNEL_ASSERT_H
#define KERNEL_ASSERT_H

/*
    user should never call this directly as it is auto implemented
    by gcc!
*/
void kernel_panic_spin(char *filename, int line, const char *func_name, const char *condition);

// kernel panic spin macro
#define KERNEL_PANIC_SPIN(...) kernel_panic_spin(__FILE__, __LINE__, __func__, __VA_ARGS__)

#ifdef RELEASE_MODE
#define KERNEL_ASSERT(COND) ((void)0)
#else
#define KERNEL_ASSERT(COND)                               \
    if ((COND))                                           \
    {                                                     \
        /* do anything here for all assertions success */ \
    }                                                     \
    else                                                  \
    {                                                     \
        KERNEL_PANIC_SPIN(#COND);                         \
    }
#endif

#endif