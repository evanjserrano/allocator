#include "alloc.h"
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#ifdef DEBUG
#define dprintf(...)                                                           \
    fprintf(stderr, "\t> %s(): \t", __FUNCTION__);                             \
    fprintf(stderr, __VA_ARGS__)
#else
#define dprintf(...)
#endif

/**
 * preamble & 0xfffe: size of allocation (must be multiple of 2)
 * preamble & 0x0001: is allocated to user
 */
#define PREAMB_SIZE_MASK  0xfffe
#define PREAMB_ALLOC_MASK 0x0001
typedef uint16_t preamble_t;
_Static_assert(_MAX_ALLOC >= sizeof(preamble_t),
               "MAX_ALLOC cannot fit a preamble");

/* Helper Function Prototypes */
bool is_allocated(preamble_t);
size_t get_size(preamble_t);
void* get_free_chunk(size_t);
void print_heap();
void combine_chunks(void*);

/* Global Variables */
void* heap_start_g = NULL;
void* heap_end_g = NULL;
size_t heap_size_g = 0;

/* Global constants */
static const size_t BLOCK_SIZE = _BLOCK_SIZE;
static const size_t MAX_ALLOC = _MAX_ALLOC;

/**
 * @brief Check if a chunk is allocated to the user
 *
 * @param preamble
 * @return if chunk is allocated to the user
 */
inline bool is_allocated(preamble_t preamble)
{
    return (preamble & PREAMB_ALLOC_MASK) == 0x1;
}

/**
 * @brief Get the size of a chunk
 *
 * @param preamble preamble to the chunk
 * @return size of the chunk
 */
inline size_t get_size(preamble_t preamble)
{
    return preamble & PREAMB_SIZE_MASK;
}

/**
 * @brief Return free chunk of at least a certain size. If no chunk exists, brk
 * will be called to allocate more memory.
 *
 * @param size Size of chunk to find (including preamble)
 * @return void* Pointer to chunk of size >= `size`
 */
void* get_free_chunk(size_t size)
{
    // initialize the heap start
    if (heap_start_g == NULL)
    {
        dprintf("Initializing Heap\n");
        heap_start_g = heap_end_g = sbrk(0);
    }

    // check size parameter
    if ((size % 2) == 1)
    {
        dprintf("Chunk size (%zu) is odd... bumping to %zu\n", size, size + 1);
        size++;
    }
    if (size > MAX_ALLOC)
    {
        dprintf("Chunk size (%zu) too large (size > %zu)\n", size, MAX_ALLOC);
        return NULL;
    }

    // traverse through currently allocated memory to find free block
    dprintf("Searching for free chunk of memory\n");
    void* curr_chunk = heap_start_g;
    while (curr_chunk < heap_end_g)
    {
        preamble_t* preamble = curr_chunk;
        preamble_t curr_chunk_size = get_size(*preamble);

        // valid chunk
        if (!is_allocated(*preamble))
        {
            // combine chunks to get larger chunk
            combine_chunks(curr_chunk);
            curr_chunk_size = get_size(*preamble);

            if (curr_chunk_size >= size)
            {
                dprintf("Free chunk found: %p\n", curr_chunk);
                return curr_chunk;
            }
        }

        // go to next chunk
        curr_chunk = (uint8_t*)curr_chunk + curr_chunk_size;
    }

    // no memory is free --> allocate more memory
    dprintf("No free chunk found... allocating more memory\n");

    // save end of heap to be start of next block
    uint8_t* block = heap_end_g;

    sbrk(BLOCK_SIZE); // syscall to allocate more memory
    heap_end_g = (uint8_t*)heap_end_g + BLOCK_SIZE;
    heap_size_g += BLOCK_SIZE;

    // fill blocks' preamble
    size_t i;
    for (i = 0; i + MAX_ALLOC <= BLOCK_SIZE; i += MAX_ALLOC)
    {
        // set chunk to size MAX_ALLOC
        *(preamble_t*)(block + i) = MAX_ALLOC & PREAMB_SIZE_MASK;
    }
    // set remaining chunk size
    if (i < BLOCK_SIZE)
    {
        *(preamble_t*)(block + i) = (BLOCK_SIZE - i) & PREAMB_SIZE_MASK;
    }

    return block;
}

void* allocm(size_t size)
{
    dprintf("size = %zu\n", size);

    // size must be divisible by 2 to be used in preamble
    if ((size % 2) == 1)
    {
        dprintf("Size (%zu) is odd... bumping to %zu\n", size, size + 1);
        size++;
    }

    /* Look for free chunk */
    size_t chunk_size = size + sizeof(preamble_t);
    void* chunk = get_free_chunk(chunk_size);
    preamble_t rem;
    if (chunk == NULL)
    {
        dprintf("Memory could not be allocated\n");
        return NULL;
    }

    // remaining free chunk space
    rem = *(preamble_t*)chunk - chunk_size;

    /* Allocate in free chunk */
    if (rem > 0)
    {
        uint8_t* next_chunk = (uint8_t*)chunk + chunk_size;
        *(preamble_t*)next_chunk = rem;
    }

    /* Add preamble and set to 'allocated' */
    *(preamble_t*)chunk = chunk_size | PREAMB_ALLOC_MASK;

#ifdef CLEAN_MEMORY
    dprintf("Filling user's memory\n");
    for (size_t i = 0; i < size; i++)
    {
        *((uint8_t*)chunk + sizeof(preamble_t) + i) = 0xAA;
    }
#endif

    dprintf("Allocating %zu Bytes at %p\n", size, chunk + sizeof(preamble_t));
    // offset pointer from preamble
    return chunk + sizeof(preamble_t);
}

void freem(void* ptr)
{
    dprintf("ptr = %p\n", ptr);

    if (ptr == NULL)
    {
        dprintf("Trying to free a NULL pointer\n");
        return;
    }

    // chunk starts sizeof(preamble_t) bytes before user's ptr
    uint8_t* chunk = (uint8_t*)ptr - sizeof(preamble_t);
    preamble_t* preamble = (preamble_t*)chunk;
    if (!is_allocated(*preamble))
    {
        // memory not allocated
        dprintf("Memory at %p is already unallocated (Preamble: %#6X)\n", ptr,
                *preamble);
        return;
    }
    // set "free" bit to 0
    *preamble = *preamble & PREAMB_SIZE_MASK;

    // combine free chunks together
    combine_chunks(chunk);
}

void combine_chunks(void* start)
{
    // cannot combine a chunk that already is allocated
    if (start == NULL || is_allocated(*(preamble_t*)start))
    {
        dprintf("Start is allocated!");
        return;
    }

    uint8_t* chunk = start;
    preamble_t* preamble = start;
    preamble_t size = get_size(*preamble);
    uint8_t* heap_end = heap_end_g;
    uint8_t* next_chunk = chunk + size;
    while (next_chunk < heap_end)
    {
        // if next chunk is used, cannot combine anymore
        // if the current chunk is too big, don't need to combine anymore
        if (is_allocated(*(preamble_t*)next_chunk) || size >= MAX_ALLOC)
        {
            break;
        }

        // combine chunk with adjacent chunk
        dprintf("Combining %p (%dB) with %p (%dB)\n", chunk, size, next_chunk,
                *(preamble_t*)next_chunk);
        *preamble = (size + *(preamble_t*)next_chunk);
        size = get_size(*preamble);
        next_chunk = chunk + size;
    }
}

void print_heap()
{
    dprintf("\n");
    int rows = heap_size_g / 0x10;
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
    while (curr_addr < (uint8_t*)heap_end_g)
    {
        preamble_t preamble = *(preamble_t*)curr_addr;
        size_t size = get_size(preamble);

        printf("\t%p  %5zu  (%#6zx)   %c\n", curr_addr, size, size,
               is_allocated(preamble) ? 'X' : ' ');

        curr_addr += size;
    }
}