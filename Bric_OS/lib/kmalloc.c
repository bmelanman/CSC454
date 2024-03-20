/** @file kmalloc.c
 *
 * @brief A custom implementation of kmalloc, kfree, kcalloc, and krealloc.
 */

#include "kmalloc.h"

#include "errno.h"
#include "mmu_driver.h"

#define DEBUG_MSG_ENABLE 0

static header_t *kernel_heap_head = NULL;

/**
 * @brief Attemps to return memory allocated with `kbrk()` to the OS.
 */
void kmalloc_cleanup( void )
{
    // kernel_heap_head must exist
    if ( !IS_VALID( kernel_heap_head ) )
    {
        return;
    }

    int num_unfree_blks = 0;
    header_t *b = kernel_heap_head;
    void *ret = NULL;

    // Traverse to the end of the linked list
    while ( IS_VALID( b ) && IS_VALID( b->next ) )
    {
        // Count the number of unfreed blocks and free them
        if ( b->free == false )
        {
            num_unfree_blks++;

            kfree( b->ptr );
            b = kernel_heap_head;
        }
        b = b->next;
    }

    // Report unfreed blocks
    if ( num_unfree_blks > 0 )
    {
        OS_WARN( "There are %d unfreed blocks!\n", num_unfree_blks );
    }

    // Check if we can give memory back to the OS
    if ( (uintptr_t)kernel_heap_head->ptr + kernel_heap_head->size == (uintptr_t)kbrk( 0 ) )
    {
        ret = (header_t *)kbrk( (int64_t)kernel_heap_head->size * -1 );

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
    if ( (uintptr_t)kbrk( 0 ) % ALIGN_SIZE != 0 )
    {
        kbrk( ALIGN_SIZE - ( (uintptr_t)kbrk( 0 ) % ALIGN_SIZE ) );
    }

    // Calculate the minimum amount of memory needed in multiples of BIN_SIZE
    size_t total_size = ROUND_UP( min_blk_size, BIN_SIZE );

    // Extend the heap
    header_t *b = (header_t *)kbrk( total_size + HEADER_SIZE );

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
    // Make sure kernel_heap_head exists
    if ( !IS_VALID( kernel_heap_head ) )
    {
        kernel_heap_head = extend_mem( BIN_SIZE, NULL, NULL );
        // atexit( kmalloc_cleanup );
    }

    // Start at kernel_heap_head
    header_t *b = kernel_heap_head;
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

void *kmalloc( size_t size )
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
        errno = ENOMEM;
        return NULL;
    }

    // Get a new block of empty memory
    header_t *b = get_empty_mem( total_size );

    // Error checking
    if ( IS_VALID( b ) == false )
    {
        errno = ENOMEM;
        return NULL;
    }

    if ( DEBUG_MSG_ENABLE )
    {
        OS_INFO( "kmalloc(%lu) => (ptr=%p, size=%u)\n", size, b->ptr, b->size );
    }

    return b->ptr;
}

void *kcalloc( size_t nmemb, size_t size )
{
    // Align the size to 16 bytes
    size_t total_size = ROUND_UP( nmemb * size, ALIGN_SIZE );

    // Check if the size is too big
    if ( nmemb > UINT32_MAX || size > UINT32_MAX || total_size > UINT32_MAX )
    {
        errno = ENOMEM;
        return NULL;
    }

    // Allocate and check for errors
    void *ptr = kmalloc( total_size );
    if ( IS_NULL( ptr ) )
    {
        return NULL;
    }

    memset( ptr, 0, GET_HEADER( ptr )->size );

    if ( DEBUG_MSG_ENABLE )
    {
        OS_INFO(
            "kcalloc(%lu, %lu) => (ptr=%p, size=%u)\n", nmemb, size, ptr, GET_HEADER( ptr )->size
        );
    }

    return ptr;
}

void *krealloc( void *ptr, size_t size )
{
    // krealloc(NULL, size) --> kmalloc(size)
    if ( IS_NULL( ptr ) )
    {
        return kmalloc( size );
    }

    // krealloc(p, 0) --> kfree(p)
    if ( size == 0 )
    {
        kfree( ptr );
        return NULL;
    }

    // Align the size to 16 bytes
    size_t total_size = ROUND_UP( size, ALIGN_SIZE );

    // Check if the size is too big
    if ( size > UINT32_MAX || total_size > UINT32_MAX )
    {
        errno = ENOMEM;
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
        new_ptr = kmalloc( total_size );

        // Copy the data from the old block to the new one
        memcpy( new_ptr, ptr, b->size );

        // Free the old block
        kfree( ptr );
    }

    // Split the new block if necessary
    split_block( GET_HEADER( new_ptr ), size );

    if ( DEBUG_MSG_ENABLE )
    {
        OS_INFO(  // NOLINT
            "krealloc(%p, %lu) => (ptr=%p, size=%u)\n", ptr, size, new_ptr,
            GET_HEADER( new_ptr )->size
        );
    }

    return new_ptr;
}

void kfree( void *ptr )
{
    // Check for NULL input
    if ( IS_NULL( ptr ) )
    {
        return;
    }

    // Simple check to make sure the pointer is within the know address space
    if ( ptr < (void *)kernel_heap_head || (void *)( (uintptr_t)kbrk( 0 ) - MIN_BLK_SIZE ) < ptr )
    {
        OS_WARN( "kfree(%p): Invalid pointer!\n", ptr );
        errno = EFAULT;
        return;
    }

    // Traverse the linked list to get the ptr's corrsponding block
    header_t *b = kernel_heap_head;
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
        OS_ERROR( "kfree(%p): Invalid pointer!\n", ptr );
        errno = EFAULT;
        return;
    }

    // Mark the current block as free
    b->free = true;

    // Try to merge the current block with the next block
    merge_blocks( b, b->next );

    // Try to merge the current block with the previous block
    merge_blocks( b->prev, b );

    if ( DEBUG_MSG_ENABLE )
    {
        OS_INFO( "kfree(%p)\n", ptr );
    }
}

/*** end of file ***/