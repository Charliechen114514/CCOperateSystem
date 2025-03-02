#include "include/user/library/user_assertion.h"
#include "include/user/stdio/stdio.h"

// user spin, only the thread disabled.
// but not kernel level panic!
void user_spin(char *filename, int line, const char *func,
               const char *condition) {
    printf("\n\n\n\nfilename %s\nline %d\nfunction %s\ncondition %s\n",
           filename, line, func, condition);
    while (1)
        ;
}
