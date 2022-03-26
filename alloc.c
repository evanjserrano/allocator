#include "alloc.h"
#include <stdio.h>
#include <unistd.h>

const size_t BLOCK_SIZE = 0x1000; // 4KB
void* heap_start = (void*)-1;

void print_heap();

/**
 * @brief Return free chunk of at least a certain size. If no chunk exists, brk
 * will be called to allocate more memory.
 *
 * @param size Size of chunk to find
 * @return void* Pointer to chunk of size >= `size`
 */
void* get_chunk(size_t size)
{
    void* prog_break = sbrk(0);

    // initialize the heap start
    if (heap_start == (void*)-1)
        heap_start = prog_break;

    void* curr_chunk = heap_start;

    // traverse through currently allocated memory to find free block
    while (curr_chunk < prog_break)
    {
        if (0)
            return NULL;
    }

    // no memory is free --> allocate more memory
    sbrk(BLOCK_SIZE);

    size_t* preamble = (size_t*)prog_break;
    // zero-out lsb -- mem is free
    *preamble = BLOCK_SIZE & (size_t)(-2);

    print_heap();

    return prog_break;
}

void* allocm(size_t size)
{
    /* Look for free chunk */
    get_chunk(size);

    /* Allocate in free chunk */

    /* Add preamble */
    return NULL;
}

void freem(void* ptr)
{
}

void print_heap()
{
    size_t heap_size = (size_t)(sbrk(0) - heap_start);
    int rows = heap_size / 0x10;
    int cols = 0x10;

    uint8_t* curr_addr = heap_start;

    for (int i = 0; i < rows; i++)
    {
        printf("%p:\t", curr_addr);
        for (int j = 0; j < cols; j++)
        {
            printf("%02X   ", *curr_addr++);
        }
        printf("\n");
    }
}