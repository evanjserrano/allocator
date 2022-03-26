#ifndef _EALLOC_H_
#define _EALLOC_H_

#include <stdlib.h>

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