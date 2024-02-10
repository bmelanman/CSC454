/** @file malloc.h
 *
 * @brief A custom implementation of malloc, free, calloc, and realloc.
 */

#ifndef MALLOC_H
# define MALLOC_H

// # include <errno.h>
// # include <stdarg.h>
// # include <stdbool.h>
// # include <stdint.h>
// # include <stdio.h>
// # include <stdlib.h>
// # include <string.h>
// # include <unistd.h>

# include "common.h"

/* Important Sizes */
# define BIN_SIZE       ( (size_t)( 65536U ) )
# define HEADER_SIZE    ( (size_t)( sizeof( header_t ) ) )
# define ALIGN_SIZE     ( (size_t)( 16U ) )
# define MAX_ALLOC_SIZE ( (size_t)( UINT32_MAX ) )
# define MIN_BLK_SIZE   ( (size_t)( HEADER_SIZE + ALIGN_SIZE ) )

/* Macros */
# define ROUND_UP( n, d )  ( ( ( n - 1 ) | ( d - 1 ) ) + 1 )
# define GET_HEADER( ptr ) ( (header_t *)( (uintptr_t)ptr - HEADER_SIZE ) )
# define IS_NULL( p )      ( p == NULL )
# define IS_VALID( b )     ( b != NULL )
# define IS_FREE( b )      ( b->free == true )

/* Memory Block Struct */
typedef struct __packed _header_s header_t;
struct __packed _header_s
{
    // Total: 32 bytes
    void *ptr;        // 8 bytes
    uint32_t size;    // 4 bytes
    bool free;        // 1 byte
    uint8_t _pad[3];  // 3 bytes
    header_t *next;   // 8 bytes
    header_t *prev;   // 8 bytes
};

/***** Library Functions *****/

/**
 * @brief Allocates `size` bytes and returns a pointer to the (uninitialized)
 *        allocated memory. If size is 0, then `malloc()` returns a unique
 *        pointer value that can later be successfully passed to `free()`.
 * @param size The size of the block to allocate, aligned to 16 bytes.
 * @return A pointer to the allocated memory, aligned to 16 bytes. If an error
 *         occurs, NULL is returned.
 */
void *malloc( size_t size );

/**
 * @brief Frees the memory space pointed to by ptr, which must have been
 *        returned by a previous call to `malloc()` or related functions.
 *        Otherwise, or if ptr has already been freed, undefined behavior
 *        occurs. If ptr is NULL, no operation is performed.
 * @param ptr A pointer to the memory block to free.
 */
void free( void *ptr );

/**
 * @brief Allocates memory for an array of `nmemb` elements of `size` bytes
 *        each and returns a pointer to the allocated memory, which has been
 *        initialized to zero. If nmemb or size is 0, then `calloc()` returns a
 *        unique pointer that can later be successfully passed to `free()`. \n
 *
 *        If the multiplication of nmemb and size would result in integer
 *        overflow, then `calloc()` returns an error.
 *
 * @param nmemb The number of elements in the array.
 * @param size The size of each element in the array, aligned to 16 bytes.
 * @return A pointer to the allocated memory, aligned to 16 bytes, with all
 *         bytes set to zero. If an error occurs, NULL is returned.
 */
void *calloc( size_t nmemb, size_t size );

/**
 * @brief changes the size of the memory block pointed to by ptr to size bytes.
 *        The contents of the memory will be unchanged in the range from the
 *        start of the region up to the minimum of the old and new sizes. If
 *        the new size is larger than the old size, the added memory will not
 *        be initialized. \n
 *
 *        If ptr is NULL, then the call is equivalent to `malloc(size)`, for
 *        all values of size. \n
 *
 *        If size is equal to zero, and ptr is not NULL, then the call is
 *        equivalent to `free(ptr)`. \n
 *
 *        Unless ptr is NULL, it must have been returned by an earlier call to
 *        malloc or related functions. If the area pointed to was moved, a
 *        `free(ptr)` is done.
 * @param ptr A pointer to the memory block to reallocate.
 * @param size The new size of the memory block, aligned to 16 bytes.
 * @return A pointer to the reallocated memory block, aligned to 16 bytes, or
 *         NULL if an error occurred.
 */
void *realloc( void *ptr, size_t size );

#endif /* MALLOC_H */

/*** end of file ***/