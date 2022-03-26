#include "alloc.h"
#include <stdio.h>

int main(void)
{
    printf("Hello, World!\n");
    void* ptr = allocm(10);

    return 0;
}