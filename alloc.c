#include "alloc.h"
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#define DEBUG 1
#if DEBUG
#define dprintf(...)                                                           \
    fprintf(stderr, "\t> %s(): \t", __FUNCTION__);                             \
    fprintf(stderr, __VA_ARGS__)
#else
#define dprintf(...)
#endif

/*
 * preamble & 0xfffe: size of allocation (must be multiple of 2)
 * preamble & 0x0001: is allocated to user
 */
typedef uint16_t preamble_t;

bool is_allocated(preamble_t);
void* get_chunk(size_t);
void print_heap();
void combine_chunks(void*);

void* heap_start_g = NULL;
static const size_t BLOCK_SIZE = _BLOCK_SIZE;
static const size_t MAX_ALLOC = _MAX_ALLOC;

_Static_assert(_BLOCK_SIZE % _MAX_ALLOC == 0,
               "BLOCK_SIZE must be divisible by MAX_ALLOC");
// _Static_assert(_MAX_ALLOC < (1 << 8 * sizeof(preamble_t)) - 2,
//                "MAX_ALLOC must fit in preamble_t");

/**
 * @brief Check if a chunk is allocated to the user
 *
 * @param preamble
 * @return if chunk is allocated to the user
 */
inline bool is_allocated(preamble_t preamble)
{
    return (preamble & 0x1) == 0x1;
}

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
    if (heap_start_g == NULL)
    {
        dprintf("Initializing Heap\n");
        heap_start_g = prog_break;
    }

    // check size parameter
    if ((size & 0x1) == 1)
    {
        dprintf("Size (%zu) is odd... bumping to %zu\n", size, size + 1);
        size++;
    }
    if (size > MAX_ALLOC)
    {
        dprintf("Size (%zu) too large (size > %zu)\n", size, MAX_ALLOC);
        return NULL;
    }

    // traverse through currently allocated memory to find free block
    dprintf("Searching for free chunk of memory\n");
    uint8_t* curr_chunk = heap_start_g;
    while (curr_chunk < prog_break)
    {
        preamble_t preamble = *(preamble_t*)curr_chunk;
        preamble_t chunk_size = preamble & -2;

        // valid chunk
        if (!is_allocated(preamble))
        {
            // combine chunks to get larger chunk
            combine_chunks(curr_chunk);
            chunk_size = *(preamble_t*)curr_chunk & -2;

            if (chunk_size >= size)
            {
                dprintf("Free chunk found: %p\n", curr_chunk);
                return curr_chunk;
            }
        }

        // go to next chunk
        curr_chunk += chunk_size;
    }

    // no memory is free --> allocate more memory
    dprintf("No free chunk found... allocating more memory\n");
    sbrk(BLOCK_SIZE);
    uint8_t* block = (uint8_t*)prog_break;
    // fill blocks' preamble
    for (size_t i = 0; i < BLOCK_SIZE; i += MAX_ALLOC)
    {
        // set chunk to size MAX_ALLOC
        *(preamble_t*)(block + i) = MAX_ALLOC & -2;
    }

    return prog_break;
}

void* allocm(size_t size)
{
    dprintf("size = %zu\n", size);

    if ((size & 0x1) == 1)
    {
        dprintf("Size (%zu) is odd... bumping to %zu\n", size, size + 1);
        size++;
    }

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

#if CLEAN_MEMORY
    for (size_t i = 0; i < size; i++)
    {
        *((uint8_t*)chunk + sizeof(preamble_t) + i) = 0xAA;
    }
#endif
    return chunk + sizeof(preamble_t);
}

void freem(void* ptr)
{
    dprintf("ptr = %p\n", ptr);

    uint8_t* chunk = (uint8_t*)ptr - sizeof(preamble_t);
    preamble_t* preamble = (preamble_t*)chunk;
    if (!is_allocated(*preamble))
    {
        // memory not allocated
        dprintf("Memory at %p is unallocated (Preamble: %#6X)\n", ptr,
                *preamble);
        return;
    }
    // set "free" bit to 0
    *preamble = *preamble & -2;

    // combine free chunks together
    combine_chunks(chunk);
}

void combine_chunks(void* start)
{
    if (is_allocated(*(preamble_t*)start))
    {
        dprintf("Start is allocated!");
        return;
    }

    uint8_t* chunk = start;
    preamble_t* preamble = start;
    preamble_t size = *preamble & -2;
    uint8_t* heap_end = sbrk(0);
    uint8_t* next_chunk = chunk + size;
    while (next_chunk < heap_end)
    {
        // if next chunk is used, cannot combine anymore
        if (is_allocated(*(preamble_t*)next_chunk))
        {
            break;
        }

        // combine chunk with adjacent chunk
        dprintf("Combining %p (%dB) with %p (%dB)\n", chunk, size, next_chunk,
                *(preamble_t*)next_chunk);
        *preamble = (size + *(preamble_t*)next_chunk);
        size = *preamble & -2;
        next_chunk = chunk + size;
    }
}

void print_heap()
{
    dprintf("\n");
    size_t heap_size = (size_t)(sbrk(0) - heap_start_g);
    int rows = heap_size / 0x10;
    int cols = 0x10;

    uint8_t* curr_addr = heap_start_g;

    printf("\t  pointer   \t_0____1____2____3____4____5____6____7____8____9___"
           "10___11___12___13___14___15\n");
    // 16 bytes per row
    for (int i = 0; i < rows; i++)
    {
        printf("\t%p:\t", curr_addr);
        // print each byte
        for (int j = 0; j < cols; j++)
        {
            printf("%02X   ", *curr_addr++);
        }
        printf("\n");
    }
    printf("\n");

    printf("\t  pointer    size(B)    hex  used \n");
    // print by chunk
    curr_addr = heap_start_g;
    while (curr_addr < (uint8_t*)heap_start_g + heap_size)
    {
        preamble_t preamble = *(preamble_t*)curr_addr;
        size_t size = (size_t)(preamble & -2);

        printf("\t%p  %5zu  (%#6zx)   %c\n", curr_addr, size, size,
               is_allocated(preamble) ? 'X' : ' ');

        curr_addr += size;
    }
}