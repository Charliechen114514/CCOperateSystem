const char* msg = "Sup! Inline ASM\n";
int n = 0;

int main()
{
    while (msg[n]){
        n++;
    }
    asm("pusha;\
        movl $4, %eax;\
        movl $1, %ebx;\
        movl msg, %ecx;\
        movl n, %edx; \
        int $0x80;\
        popa");
}