#include <assert.h>
#include <stdio.h>
int main()
{
    printf("Welcomes assert demo!\n");
    int x = 5;
    printf("x is %d\nwe will assert that x > 0, which will be pass!\n", x);
    assert(x > 0);
    printf("ok, and then it will failed as we assert x is <= 0!\n");
    assert(x <= 0);
}