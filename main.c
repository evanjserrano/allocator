#include "alloc.h"
#include <stdio.h>

extern void print_heap();

#define PRINT_HEAP()                                                           \
    printf("~~~~\nHEAP: (main:%d)\n", __LINE__);                               \
    print_heap();                                                              \
    printf("~~~~\n")

int main(void)
{
    printf("Hello, World!\n");

#define NUM_PTRS 8
    void* ptrs[NUM_PTRS];

    ptrs[0] = allocm(0);
    PRINT_HEAP();
    freem(ptrs[0]);
    PRINT_HEAP();
    for (int i = 0; i < NUM_PTRS; i++)
    {
        ptrs[i] = allocm(i);
        printf("ptr: %p, size: %d\n", ptrs[i], i);
    }
    PRINT_HEAP();

    freem(ptrs[3]);
    PRINT_HEAP();
    ptrs[3] = allocm(0);
    PRINT_HEAP();
    freem(ptrs[3]);
    PRINT_HEAP();
    freem(ptrs[4]);
    PRINT_HEAP();
    ptrs[3] = allocm(7);
    PRINT_HEAP();

    return 0;

    /**
     * ptr = alloc(2)
     * *ptr = 0xabcd
     * free(ptr)
     */

    short* ptr = allocm(sizeof(short));
    printf("%p: ", ptr);
    *ptr = 0xabcd;
    printf("%hX\n", *ptr);
    PRINT_HEAP();
    freem(ptr);
    PRINT_HEAP();

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
    PRINT_HEAP();
    long* ptr2 = allocm(sizeof(long));
    printf("%p: ", ptr);
    *ptr2 = 0x12345678beefL;
    printf("%lX\n", *ptr2);
    PRINT_HEAP();
    freem(ptr2);
    PRINT_HEAP();
    freem(ptr);
    PRINT_HEAP();

    return 0;
}