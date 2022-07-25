#ifndef _EALLOC_H_
#define _EALLOC_H_

#include <stdlib.h>

#define _MAX_ALLOC  0x20
#define _BLOCK_SIZE 0x40

_Static_assert(_BLOCK_SIZE % _MAX_ALLOC == 0,
               "MAX_ALLOC must be divisible by BLOCK_SIZE");

/**
 * @brief Allocate block of memory of `size` bytes
 *
 * @param size Number of bytes to allocate
 * @return void* Pointer to start of allocated chunk
 */
void* allocm(size_t size);

/**
 * @brief Deallocate block of memory previously allocated by `allocm()`
 *
 * @param ptr Pointer to start of allocated chunk to free
 */
void freem(void* ptr);

#endif