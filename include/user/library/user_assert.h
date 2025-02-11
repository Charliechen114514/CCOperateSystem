#ifndef USER_ASSERT_H
#define USER_ASSERT_H

#include "include/library/types.h"

void user_spin(char *filename, int line, const char *func,
               const char *condition);
#define panic(...) user_spin(__FILE__, __LINE__, __func__, __VA_ARGS__)

#define assert(CONDITION)  \
    if (!(CONDITION)) {    \
        panic(#CONDITION); \
    }

#endif