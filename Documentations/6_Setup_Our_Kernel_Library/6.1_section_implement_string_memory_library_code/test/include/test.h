#ifndef TEST_H
#define TEST_H

#ifdef IN_KER_TEST
#include "include/library/ccos_print.h"
#include "include/library/kernel_assert.h"
#define TEST_PRINT(strings) ccos_puts(strings)
#define TEST_ASSERT(COND) KERNEL_ASSERT(COND)
#else
#include <stdio.h>
#include <assert.h>
#define TEST_PRINT(strings) puts(strings)
#define TEST_ASSERT(COND) assert(COND)
#endif

// involk test
void run_strings_test(void);
void involk_test(void);

#endif