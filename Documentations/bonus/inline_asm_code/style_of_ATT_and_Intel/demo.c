#include <stdio.h>

void greetings(){
    printf("Hello, Charlie!\n");
}

int add(int a, int b){
    return a + b;
}

int main()
{
    greetings();
    int res = add(1, 2);
    return 0;
}