#include "alloc.h"
#include <stdio.h>

extern void print_heap();

int main(void)
{
    printf("Hello, World!\n");
    short* ptr = allocm(sizeof(short));
    printf("%p\n", ptr);
    *ptr = 0xabcd;
    print_heap();
    freem(ptr);
    print_heap();

    return 0;
}