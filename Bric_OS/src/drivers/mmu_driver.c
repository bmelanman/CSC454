/** @file mmu_driver.c
 *
 * @brief
 *
 * @author Bryce Melander
 * @date Feb-07-2024
 *
 * @copyright (c) 2024 by Bryce Melander under MIT License. All rights reserved.
 * (See http://opensource.org/licenses/MIT for more details.)
 */

#include "mmu_driver.h"

/* Private Defines and Macros */

#define NUM_INIT_PAGES ( 4U )

#define PAGE_AND_HEADER_SIZE ( PAGE_SIZE + sizeof( page_frame_t ) )

#define BYTE_ALIGN_LEN ( 8U )

// Align x to the next multiple of n
#define ALIGN( x, n ) ( ( ( (x)-1 ) | ( (n)-1 ) ) + 1 )

#define BTYE_ALIGN_ADDR( x ) ( (void *)ALIGN( (uintptr_t)( x ), BYTE_ALIGN_LEN ) )

#define GET_PF_HEADER( pf ) ( (page_frame_t *)( (uint8_t *)( pf ) - sizeof( page_frame_t ) ) )

/* Private Types and Enums */

typedef struct page_frame_s page_frame_t;

struct page_frame_s
{
    page_frame_t *next;  // 8 bytes
    void *addr;          // 8 bytes
};

/* Global Variables */

// MMAP entries from multiboot2
static mb_mmap_entry_t *mmap_entries;
static uint32_t num_mmap_entries;

// In-use page frames
static page_frame_t *pf_list = NULL;

// Free page frames
static page_frame_t *free_pf_list = NULL;

/* Private Functions */

void create_page_frame( void )
{
    void *addr = NULL;

    // Get the next available memory region from the MMAP entries
    for ( uint32_t i = 0; i < num_mmap_entries; i++ )
    {
        if ( MEM_IS_AVAILABLE( mmap_entries[i] ) )
        {
            addr = (void *)mmap_entries[i].addr;
            break;
        }
    }

    // DEBUG: Im pretty sure i dont have to do this but idk rn
    if ( addr == NULL )
    {
        addr = (void *)PAGE_SIZE;

        mmap_entries->addr += PAGE_SIZE;
        mmap_entries->len -= PAGE_SIZE;
    }

    // Remove the page from the mmap list
    mmap_entries->addr += PAGE_AND_HEADER_SIZE;
    mmap_entries->len -= PAGE_AND_HEADER_SIZE;

    page_frame_t *new_pf = (page_frame_t *)addr;
    new_pf->addr = BTYE_ALIGN_ADDR( ( (uintptr_t)addr + sizeof( page_frame_t ) ) );

    new_pf->next = free_pf_list;
    free_pf_list = new_pf;

    OS_INFO( "Page frame created at %p with address %p\n", (void *)new_pf, new_pf->addr );
}

/* Public Functions */

driver_status_t MMU_init( void *tag_ptr )
{
    if ( tag_ptr == NULL )
    {
        OS_ERROR( "Invalid multiboot2 tag pointer\n" );
        return FAILURE;
    }

    get_multiboot2_mmap_info( tag_ptr, &mmap_entries, &num_mmap_entries );

    if ( num_mmap_entries == 0 )
    {
        OS_ERROR( "No memory map entries found???\n" );
        HLT();
    }

    // Create the first free page frames
    for ( uint i = 0; i < NUM_INIT_PAGES; i++ )
    {
        create_page_frame();
    }

    return SUCCESS;
}

void *MMU_pf_alloc( void )
{
    if ( free_pf_list == NULL )
    {
        create_page_frame();
    }

    page_frame_t *pf = free_pf_list;
    free_pf_list = free_pf_list->next;

    pf->next = pf_list;
    pf_list = pf;

    OS_INFO( "Page allocated at %p\n", pf->addr );

    return pf->addr;
}

void *MMU_pf_alloc_n( int n )
{
    int i = 0;
    void *first_addr = NULL;

    while ( ( i++ < n ) && ( free_pf_list != NULL ) )
    {
        page_frame_t *pf = free_pf_list;
        free_pf_list = free_pf_list->next;

        pf->next = pf_list;
        pf_list = pf;

        if ( first_addr == NULL )
        {
            first_addr = pf->addr;
        }
    }

    return first_addr;
}

void MMU_pf_free( void *pf )
{
    page_frame_t *pf_to_free = GET_PF_HEADER( pf );

    pf_to_free->next = free_pf_list;
    free_pf_list = pf_to_free;

    OS_INFO( "Page deallocated at %p\n", free_pf_list );
}

void MMU_pf_free_n( void *pf, int n )
{
    int i = 0;

    OS_INFO( "Freeing %d pages starting at %p\n", n, pf );

    page_frame_t *pf_to_free = GET_PF_HEADER( pf );

    while ( ( i++ < n ) && ( pf_to_free != NULL ) )
    {
        uintptr_t next_pf = (uintptr_t)pf_to_free->next;

        MMU_pf_free( pf_to_free->addr );

        pf_to_free = (page_frame_t *)next_pf;
    }

    if ( i < n )
    {
        OS_ERROR( "Not enough pages to free\n" );
    }
}

/*** End of File ***/