#include <stdio.h>
int main(){
    int a = 1, b = 2;
    int result = 0;
    asm("addl %%ebx, %%eax": "=a"(result) : "a"(a), "b"(b));
    printf("result: %d\n", result); 
}