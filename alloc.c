#include "alloc.h"
#include <stdio.h>
#include <unistd.h>

#define DEBUG 1

#if DEBUG
#define dprintf(...) fprintf(stderr, "%s(): \t", __FUNCTION__); \
                     fprintf(stderr, __VA_ARGS__)
#else
#define dprintf(...)
#endif

// const size_t BLOCK_SIZE = 0x1000; // 4KB
const size_t BLOCK_SIZE = 0x20; // 32B
void* heap_start = NULL;

typedef uint16_t preamble_t;
const size_t MAX_ALLOC = 0x8; // 8B
// const size_t MAX_ALLOC = BLOCK_SIZE;
// const size_t MAX_ALLOC = UINT16_MAX & -2;

void* get_chunk(size_t);
void print_heap();

/**
 * @brief Return free chunk of at least a certain size. If no chunk exists, brk
 * will be called to allocate more memory.
 *
 * @param size Size of chunk to find (including preamble)
 * @return void* Pointer to chunk of size >= `size`
 */
void* get_chunk(size_t size)
{
    uint8_t* prog_break = sbrk(0);

    // initialize the heap start
    if (heap_start == NULL)
    {
        dprintf("Initializing Heap\n");
        heap_start = prog_break;
    }

    // check size parameter
    if (size > MAX_ALLOC)
    {
        dprintf("Size (%zu) too large (size > %zu)\n", size, MAX_ALLOC);
        return NULL;
    }

    // traverse through currently allocated memory to find free block
    dprintf("Searching for free block of memory\n");
    uint8_t* curr_chunk = heap_start;
    while (curr_chunk < prog_break)
    {
        preamble_t chunk_size = *((preamble_t*)curr_chunk);
        // valid chunk
        if (chunk_size >= size)
        {
            dprintf("Free chunk found: %p\n", curr_chunk);
            return curr_chunk;
        }

        // go to next chunk
        curr_chunk += chunk_size;
    }

    // no memory is free --> allocate more memory
    sbrk(BLOCK_SIZE);
    uint8_t* block = (uint8_t*)prog_break;
    // fill blocks' preamble
    for (size_t i = 0; i < BLOCK_SIZE; i += MAX_ALLOC)
    {
        *(preamble_t*)(block + i) = MAX_ALLOC & -2;
    }
    print_heap();

    return prog_break;
}

void* allocm(size_t size)
{
    dprintf("size = %zu\n", size);

    /* Look for free chunk */
    size_t chunk_size = size + sizeof(preamble_t);
    void* chunk = get_chunk(chunk_size);
    preamble_t rem;
    if (chunk == NULL || (rem = *(preamble_t*)chunk - chunk_size) < 0)
    {
        return NULL;
    }

    /* Allocate in free chunk */
    if (rem > 0)
    {
        uint8_t* next_chunk = (uint8_t*)chunk + chunk_size;
        *(preamble_t*)next_chunk = rem;
    }

    /* Add preamble */
    *(preamble_t*)chunk = chunk_size | 0x0001;
    print_heap();
    return chunk + sizeof(preamble_t);
}

void freem(void* ptr)
{
    uint8_t* chunk = (uint8_t*)ptr - sizeof(preamble_t);
    preamble_t* preamble = (preamble_t*)chunk;
    if (!(*preamble & 0x1))
    {
        // memory not allocated
        return;
    }
    // set "free" bit to 0
    *preamble = *preamble & -2;
}

void print_heap()
{
    dprintf("\n");
    size_t heap_size = (size_t)(sbrk(0) - heap_start);
    int rows = heap_size / 0x10;
    int cols = 0x10;

    uint8_t* curr_addr = heap_start;

    // 16 bytes per row
    for (int i = 0; i < rows; i++)
    {
        printf("%p:\t", curr_addr);
        // print each byte
        for (int j = 0; j < cols; j++)
        {
            printf("%02X   ", *curr_addr++);
        }
        printf("\n");
    }
    printf("\n");
}