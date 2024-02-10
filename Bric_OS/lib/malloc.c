/** @file malloc.c
 *
 * @brief A custom implementation of malloc, free, calloc, and realloc.
 */

#include "malloc.h"

#include "mmu_driver.h"

/* Private Defines and Macros */

#define DEBUG 0

#define sbrk( n ) MMU_pf_alloc_n( n )

/* Global Variables */

header_t *__mem_head = NULL;

/**
 * @brief Attemps to return memory allocated with `sbrk()` to the OS.
 */
void malloc_cleanup( void )
{
    // __mem_head must exist
    if ( !IS_VALID( __mem_head ) )
    {
        return;
    }

    int num_unfree_blks = 0;
    header_t *b = __mem_head;
    void *ret = NULL;

    // Traverse to the end of the linked list
    while ( IS_VALID( b ) && IS_VALID( b->next ) )
    {
        // Count the number of unfreed blocks and free them
        if ( b->free == false )
        {
            num_unfree_blks++;

            free( b->ptr );
            b = __mem_head;
        }
        b = b->next;
    }

    // Report unfreed blocks
    OS_WARN( "%d block(s) were not freed!\n", num_unfree_blks );

    // Check if we can give memory back to the OS
    if ( (uintptr_t)__mem_head->ptr + __mem_head->size == (uintptr_t)sbrk( 0 ) )
    {
        ret = (header_t *)sbrk( (int64_t)__mem_head->size * -1 );

        if ( ret != (void *)( -1 ) )
        {
            OS_INFO( "All memory was successfully returned!\n" );
            return;
        }
    }

    OS_ERROR( "An error occurred while cleaning up! :(\n" );
}

/**
 * @brief Attempts to merge two blocks together. Both blocks must be valid
 *        (non-`NULL`) and free. The blocks must also be contiguous (i.e.
 *        `b1->ptr + b1->size == b2->ptr`). If any of these conditions are not
 *        met, nothing happens.
 * @param b1 The first block to merge.
 * @param b2 The second block to merge.
 */
void merge_blocks( header_t *b1, header_t *b2 )
{
    // The blocks must be valid (non-NULL) and free
    if ( !( IS_VALID( b1 ) && IS_VALID( b2 ) && IS_FREE( b1 ) && IS_FREE( b2 ) ) )
    {
        return;
    }

    // The blocks must be contiguous (i.e. b1->ptr + b1->size == b2->ptr)
    if ( !( (void *)( (uintptr_t)b1->ptr + b1->size ) == (void *)( b2 ) ) )
    {
        return;
    }

    // Merge the blocks
    b1->size += b2->size + HEADER_SIZE;
    b1->next = b2->next;

    // If the next pointer isn't the end of the linked list, connect the
    // current and (new) next blocks
    if ( IS_VALID( b2->next ) )
    {
        b2->next->prev = b1;
    }
}

/**
 * @brief Attempts to split a block into two blocks, with the the first block
 *        being `size` bytes long. The second block is connected after the
 *        first, and consists of the remaining space in the original block.
 *        If the block is too small to split, nothing happens.
 * @param block The block to split.
 * @param size The block's new size.
 */
void split_block( header_t *block, size_t size )
{
    // Make sure the block is big enough to split into two smaller blocks
    if ( block->size < ( size + MIN_BLK_SIZE ) )
    {
        return;
    }

    // Set up the new block
    header_t *new_b = (header_t *)( (uintptr_t)block->ptr + size );
    new_b->ptr = new_b + 1;
    new_b->size = block->size - size - HEADER_SIZE;
    new_b->free = true;
    new_b->prev = block;

    // If we aren't at the end of the linked list, connect the next block and
    // the new block together
    if ( IS_VALID( block->next ) )
    {
        new_b->next = block->next;
        new_b->next->prev = new_b;

        // Try to merge the new block with the next block
        merge_blocks( new_b, new_b->next );
    }
    else
    {
        new_b->next = NULL;
    }

    // Set up the old block
    block->size = size;
    block->next = new_b;
}

/**
 * @brief Extends the heap by the minimum amount of memory needed to satisfy
 *        the request, aligned to 16 bytes.
 * @param min_blk_size The minimum amount of memory needed, aligned to 16 bytes.
 * @return A pointer to the new block, or NULL if an error occurred.
 */
header_t *extend_mem( size_t min_blk_size, header_t *prev, header_t *next )
{
    // Align the top of the heap to 16 bytes if necessary
    if ( (uintptr_t)sbrk( 0 ) % ALIGN_SIZE != 0 )
    {
        sbrk( ALIGN_SIZE - ( (uintptr_t)sbrk( 0 ) % ALIGN_SIZE ) );
    }

    // Calculate the minimum amount of memory needed in multiples of BIN_SIZE
    size_t total_size = ROUND_UP( min_blk_size, BIN_SIZE );

    // Extend the heap
    header_t *b = (header_t *)sbrk( total_size + HEADER_SIZE );

    // Error checking
    if ( (void *)( b ) == (void *)( -1 ) )
    {
        return NULL;
    }

    // Set up the new block
    b->ptr = b + 1;
    b->size = total_size;
    b->free = true;
    b->next = next;
    b->prev = prev;

    // Split the block if necessary
    split_block( b, min_blk_size );

    return b;
}

/**
 * @brief Searches the linked list of blocks for a block that is free and
 *        has enough space to satisfy the request. If no such block exists,
 *        the heap is extended by the minimum amount of memory needed to
 *        satisfy the request, aligned to 16 bytes.
 * @param size The minimum amount of memory needed, aligned to 16 bytes.
 * @return A pointer to the new block, or NULL if an error occurred.
 */
header_t *get_empty_mem( size_t size )
{
    // Make sure __mem_head exists
    if ( !IS_VALID( __mem_head ) )
    {
        __mem_head = extend_mem( BIN_SIZE, NULL, NULL );
        // atexit( malloc_cleanup );
    }

    // Start at __mem_head
    header_t *b = __mem_head;
    header_t *b_alt = NULL;

    // Traverse the linked list
    while ( IS_VALID( b ) )
    {
        if ( IS_FREE( b ) )
        {
            // Look for an ideal block
            if ( b->size == size )
            {
                break;
            }

            // Keep a backup in case we can't find an ideal block of memory
            if ( b->size > size )
            {
                b_alt = b;

                // Use the oversized block before extending the linked list
                if ( !IS_VALID( b->next ) )
                {
                    break;
                }
            }
        }

        // If the end of the linked list isn't big enough, add a new block
        if ( !IS_VALID( b->next ) )
        {
            b->next = extend_mem( size, b, NULL );
        }

        // Go to the next block
        b = b->next;
    }

    // Error checking
    if ( !IS_VALID( b ) )
    {
        // Check if we have a backup
        if ( IS_VALID( b_alt ) )
        {
            b = b_alt;
        }
        else
        {
            return NULL;
        }
    }

    // Split the block if necessary
    split_block( b, size );

    // Mark the block as in use
    b->free = false;

    return b;
}

void *malloc( size_t size )
{
    // Check for size = 0 so we never have a block of size 0
    if ( size < 1 )
    {
        ++size;
    }

    // Align the size to 16 bytes
    size_t total_size = ROUND_UP( size, ALIGN_SIZE );

    // Check if the size is too big
    if ( size > UINT32_MAX || total_size > UINT32_MAX )
    {
        // errno = ENOMEM;
        return NULL;
    }

    // Get a new block of empty memory
    header_t *b = get_empty_mem( total_size );

    // Error checking
    if ( IS_VALID( b ) == false )
    {
        // errno = ENOMEM;
        return NULL;
    }

    // DEBUG_MALLOC_MSG( "malloc(%zu) => (ptr=%p, size=%zu)\n", size, b->ptr, b->size );

    return b->ptr;
}

void *calloc( size_t nmemb, size_t size )
{
    // Align the size to 16 bytes
    size_t total_size = ROUND_UP( nmemb * size, ALIGN_SIZE );

    // Check if the size is too big
    if ( nmemb > UINT32_MAX || size > UINT32_MAX || total_size > UINT32_MAX )
    {
        // errno = ENOMEM;
        return NULL;
    }

    // Allocate and check for errors
    void *ptr = malloc( total_size );
    if ( IS_NULL( ptr ) )
    {
        return NULL;
    }

    memset( ptr, 0, GET_HEADER( ptr )->size );

    // DEBUG_MALLOC_MSG(
    //     "calloc(%zu, %zu) => (ptr=%p, size=%zu)\n", nmemb, size, ptr, GET_HEADER( ptr )->size
    //);

    return ptr;
}

void *realloc( void *ptr, size_t size )
{
    // realloc(NULL, size) --> malloc(size)
    if ( IS_NULL( ptr ) )
    {
        return malloc( size );
    }

    // realloc(p, 0) --> free(p)
    if ( size == 0 )
    {
        free( ptr );
        return NULL;
    }

    // Align the size to 16 bytes
    size_t total_size = ROUND_UP( size, ALIGN_SIZE );

    // Check if the size is too big
    if ( size > UINT32_MAX || total_size > UINT32_MAX )
    {
        // errno = ENOMEM;
        return NULL;
    }

    header_t *b = GET_HEADER( ptr );

    // If the new size is smaller than the current size, just split the block
    if ( total_size < b->size )
    {
        split_block( b, total_size );
        return ptr;
    }

    void *new_ptr = NULL;

    // Set the current block to free for the merge operations
    b->free = true;

    // First try to extend the current block
    merge_blocks( b, b->next );

    // If that wasn't enough, try to extend the previous block as well
    if ( b->size < total_size )
    {
        merge_blocks( b->prev, b );
    }
    else
    {
        // If b + b->next was enough, b->ptr is the new pointer
        new_ptr = b->ptr;
    }

    // Reset free
    b->free = false;

    // If that wasn't enough, get a new block of memory
    if ( b->size < total_size )
    {
        new_ptr = malloc( total_size );

        // Copy the data from the old block to the new one
        memcpy( new_ptr, ptr, b->size );

        // Free the old block
        free( ptr );
    }

    // Split the new block if necessary
    split_block( GET_HEADER( new_ptr ), size );

    // DEBUG_MALLOC_MSG(
    //     "realloc(%p, %zu) => (ptr=%p, size=%zu)\n", ptr, size, new_ptr, GET_HEADER( new_ptr
    //     )->size
    //);

    return new_ptr;
}

void free( void *ptr )
{
    // Check for NULL input
    if ( IS_NULL( ptr ) )
    {
        return;
    }

    // Simple check to make sure the pointer is within the know address space
    if ( ptr < (void *)__mem_head || (void *)( (uintptr_t)sbrk( 0 ) - MIN_BLK_SIZE ) < ptr )
    {
        // UTEST_MALLOC_MSG( "free(%lu): Invalid pointer!\n", ptr );
        // errno = EFAULT;
        return;
    }

    // Traverse the linked list to get the ptr's corrsponding block
    header_t *b = __mem_head;
    while ( IS_VALID( b ) )
    {
        // Check if the ptr is within the current block's memory space
        if ( (void *)b <= ptr && ptr < (void *)( (uintptr_t)b->ptr + b->size ) )
        {
            break;
        }

        b = b->next;
    }

    // Check if we found a valid block
    if ( IS_NULL( b ) )
    {
        // UTEST_MALLOC_MSG( "free(%lu): Invalid pointer!\n", ptr );
        // errno = EFAULT;
        return;
    }

    // Mark the current block as free
    b->free = true;

    // Try to merge the current block with the next block
    merge_blocks( b, b->next );

    // Try to merge the current block with the previous block
    merge_blocks( b->prev, b );

    // DEBUG_MALLOC_MSG( "free(%p)\n", ptr );
}

/*** end of file ***/