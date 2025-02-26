#ifndef KERNEL_ASSERT_H
#define KERNEL_ASSERT_H



/* 
    user should never call this directly as it is auto implemented
    by gcc!
*/
void kernel_panic_spin(char* filename, int line, const char* func_name, const char* condition);




#endif