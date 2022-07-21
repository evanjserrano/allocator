#include "alloc.h"
#include <stdio.h>

extern void print_heap();

int main(void)
{
    printf("Hello, World!\n");

    /**
     * ptr = alloc(2)
     * *ptr = 0xabcd
     * free(ptr)
     */

    short* ptr = allocm(sizeof(short));
    printf("%p: ", ptr);
    *ptr = 0xabcd;
    printf("%hX\n", *ptr);
    print_heap();
    freem(ptr);
    print_heap();

    /**
     * ptr = alloc(2)
     * *ptr = 0xabcd
     * ptr2 = alloc(8)
     * *ptr2 = 0x12345678beef
     * free(ptr2)
     * free(ptr)
     */
    ptr = allocm(sizeof(short));
    printf("%p: ", ptr);
    *ptr = 0xabcd;
    printf("%hX\n", *ptr);
    print_heap();
    long* ptr2 = allocm(sizeof(long));
    printf("%p: ", ptr);
    *ptr2 = 0x12345678beef;
    printf("%lX\n", *ptr2);
    print_heap();
    freem(ptr2);
    print_heap();
    freem(ptr);
    print_heap();

    return 0;
}