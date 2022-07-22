#ifndef _EALLOC_H_
#define _EALLOC_H_

#include <stdlib.h>

#define _BLOCK_SIZE 0x20UL
#define _MAX_ALLOC  0x20UL
// #define _MAX_ALLOC (UINT16_MAX & -2)

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