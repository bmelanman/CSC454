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

#include "irq_handler.h"

/* Private Defines and Macros */

#define PAGE_SIZE ( 4096U )

// Necessary Virtual Address Regions:
// - Access to physical page frames
// - Kernel Regions:
//   - Text and globals
//   - Thread run-time stacks
//   - Heap
//   - Per-process stack
// - User space
//   - Text and globals
//   - Heap

#define NUM_MEM_REGIONS ( 9U )

// Virtual Memory Layout - 256 TiB Total
#define PHYSICAL_MAP_START ( 0x000000000000U )  // Physical Map ( 1 TiB )
#define PHYSICAL_MAP_END   ( 0x00FFFFFFFFFFU )
#define KERNEL_HEAP_START  ( 0x010000000000U )  // Kernel Heap ( 1 TiB )
#define KERNEL_HEAP_END    ( 0x01FFFFFFFFFFU )
#define RESERVED_START     ( 0x020000000000U )  // Reserved ( 11 TiB )
#define RESERVED_END       ( 0x0CFFFFFFFFFFU )
#define IST1_END           ( 0x0D0000000000U )  // Interrupt Stack Table 1 ( 512 GiB )
#define IST1_START         ( 0x0D7FFFFFFFFFU )
#define IST2_END           ( 0x0D8000000000U )  // Interrupt Stack Table 2 ( 512 GiB )
#define IST2_START         ( 0x0DFFFFFFFFFFU )
#define IST3_END           ( 0x0E0000000000U )  // Interrupt Stack Table 3 ( 512 GiB )
#define IST3_START         ( 0x0E7FFFFFFFFFU )
#define IST4_END           ( 0x0E8000000000U )  // Interrupt Stack Table 4 ( 512 GiB )
#define IST4_START         ( 0x0EFFFFFFFFFFU )
#define KERNEL_STACK_END   ( 0x0F0000000000U )  // Kernel Stack ( 1 TiB )
#define KERNEL_STACK_START ( 0x0FFFFFFFFFFFU )
#define USER_HEAP_START    ( 0x100000000000U )  // User Heap ( 240 TiB )
#define USER_HEAP_END      ( 0xFFFFFFFFFFFFU )

#define ALIGN_ADDR_8_BYTES( addr ) ( (void *)ALIGN( (uint64_t)( addr ), 8U ) )
#define PAGE_ALIGN_ADDR( addr )    ( (void *)ALIGN( (uint64_t)( addr ), PAGE_SIZE ) )

/* Virtual Address Structure:
 * | 63 - 48 | 47 - 39 | 38 - 30 | 29 - 21 | 20 - 12 | 11 - 00 |
 * |  Unused |    PML4 |    PDPT |      PD |      PT |  Offset |
 */

#define PAGE_MAP_OFFSET_MASK     ( 0x1FFU )  // 9 bits
#define PAGE_DIR_PTR_OFFSET_MASK ( 0x1FFU )  // 9 bits
#define PAGE_DIR_OFFSET_MASK     ( 0x1FFU )  // 9 bits
#define PAGE_TBL_OFFSET_MASK     ( 0x1FFU )  // 9 bits
#define PHYS_PAGE_OFFSET_MASK    ( 0xFFFU )  // 12 bits

#define GET_PAGE_MAP_INDEX( x )     ( ( (uint64_t)( x ) >> 39U ) & PAGE_MAP_OFFSET_MASK )
#define GET_PAGE_DIR_PTR_INDEX( x ) ( ( (uint64_t)( x ) >> 30U ) & PAGE_DIR_PTR_OFFSET_MASK )
#define GET_PAGE_DIR_INDEX( x )     ( ( (uint64_t)( x ) >> 21U ) & PAGE_DIR_OFFSET_MASK )
#define GET_PAGE_TBL_INDEX( x )     ( ( (uint64_t)( x ) >> 12U ) & PAGE_TBL_OFFSET_MASK )
#define GET_PHYS_PAGE_INDEX( x )    ( ( (uint64_t)( x ) >> 00U ) & PHYS_PAGE_OFFSET_MASK )

#define READ_FRAME_ADDR( entry )        ( (uint8_t *)( (uint64_t)( ( entry )->frame_addr ) << 12 ) )
#define WRITE_FRAME_ADDR( entry, addr ) ( ( entry )->frame_addr = ( (uint64_t)( addr ) >> 12 ) )

#define CHECK_PAGE_ALIGNED( x )                                      \
    do                                                               \
    {                                                                \
        if ( (uint64_t)( x ) % PAGE_SIZE == 0 ) break;               \
        OS_ERROR_HALT( "Address %p is not page aligned!\n", ( x ) ); \
    } while ( 0 )

#define PRESENT_BIT_MASK    ( 1U << 0U )
#define READ_WRITE_BIT_MASK ( 1U << 1U )
#define USER_SUPER_BIT_MASK ( 1U << 2U )
#define ACCESSED_BIT_MASK   ( 1U << 5U )
#define DIRTY_BIT_MASK      ( 1U << 6U )

/* Private Types and Enums */

// Entry in a Linked List of Page Frames
typedef struct pf_list_entry_s pf_list_entry_t;
struct pf_list_entry_s
{
    pf_list_entry_t *next;  // 8 bytes
};

// Entry in a Linked List of Valid Physical Address Ranges
typedef struct pf_addr_range_s pf_range_entry_t;
struct pf_addr_range_s
{
    void *start;
    void *curr_frame;
    void *end;
    pf_range_entry_t *next_entry;
};

// CR3 Register Entry
typedef struct page_map_entry_s
{                                 // 8 bytes (64 bits)
    uint64_t reserved1 : 3;       // Reserved Bits
    uint64_t write_through : 1;   // Page-Level Write-Through
    uint64_t cache_disabled : 1;  // Page-Level Cache Disable
    uint64_t reserved2 : 7;       // Reserved Bits
    uint64_t frame_addr : 52;     // Frame Address
} __packed pg_map_entry_t;

// Page Table, Page Directory, Directory Pointer Table, and Page Map Entry (Levels 1 - 4)
typedef struct page_directory_entry_s
{
    uint64_t present : 1;      // Present Bit ............... 1 = Child Table/Page Exists
    uint64_t writable : 1;     // Read/Write Bit ............ 0 = Read-Only, 1 = Read/Write
    uint64_t user : 1;         // User/Supervisor Bit ....... 0 = Supervisor, 1 = User
    uint64_t accessed : 1;     // Accessed Bit .............. 1 = Data has been Accessed
    uint64_t dirty : 1;        // Dirty Bit ................. 1 = Data has been Written to
    uint64_t alloc : 1;        // Allocate on Demand Bit .... 1 = Allocate before Accessing
    uint64_t unused : 16;      // Unused Bits ............... Currently Unused
    uint64_t frame_addr : 40;  // Frame Address ............. Address of the Child Table/Page
    uint64_t no_execute : 1;   // No-Execute Bit ............ 0 = Execute, 1 = No-Execute
} __packed pg_dir_entry_t;     // 64 bits Total

/* Global Variables */

// MMAP entries from multiboot2
static mb_mmap_entry_t *mmap_entries = NULL;
static uint32_t num_mmap_entries = 0;

// Free physical address ranges
pf_range_entry_t *addr_range_head = NULL;
pf_range_entry_t *addr_range_curr = NULL;
pf_range_entry_t *addr_range_tail = NULL;

// Kernel start and end addresses
extern uint64_t kernel_start, kernel_end;
void *kernel_start_addr = NULL, *kernel_end_addr = NULL;

// CR3 Register
pg_map_entry_t *cr3 = NULL;

// Page Map Table (Level 4)
static pg_dir_entry_t *pml4 = NULL;

// Free page frames
static pf_list_entry_t *pf_free_list_head = NULL;

// Local Heap for the Linked List of Valid Physical Address Ranges
static uint8_t *local_heap_ptr = (uint8_t *)( PAGE_SIZE );

// Next available virtual address in each region
static void *virt_addr[VIRT_ADDR_MAX] = {
    [VIRT_ADDR_PHYSICAL_MAP] = (void *)( PHYSICAL_MAP_START + PAGE_SIZE ),  // Skip the first page
    [VIRT_ADDR_KERNEL_HEAP] = (void *)KERNEL_HEAP_START,
    [VIRT_ADDR_RESERVED] = (void *)RESERVED_START,
    [VIRT_ADDR_IST1] = (void *)IST1_START,
    [VIRT_ADDR_IST2] = (void *)IST2_START,
    [VIRT_ADDR_IST3] = (void *)IST3_START,
    [VIRT_ADDR_IST4] = (void *)IST4_START,
    [VIRT_ADDR_KERNEL_STACK] = (void *)KERNEL_STACK_START,
    [VIRT_ADDR_USER_HEAP] = (void *)USER_HEAP_START
};

/* Private Functions */

// Allocates a new page frame from the list of MMAP entries
void *alloc_new_pf( void )
{
    void *phys_page = NULL;

    // Make sure a page frame is available
    if ( addr_range_curr == NULL || addr_range_curr->curr_frame == NULL )
    {
        OS_ERROR_HALT( "addr_range_curr is NULL!\n" );
    }

    // Get the current page frame address
    phys_page = addr_range_curr->curr_frame;

    // Adjust the current page frame address
    addr_range_curr->curr_frame += PAGE_SIZE;

    // Check if we need to go to the next range
    if ( (uint64_t)addr_range_curr->curr_frame >= (uint64_t)addr_range_curr->end )
    {
        addr_range_curr = addr_range_curr->next_entry;
    }

    OS_INFO( "Allocated NEW physical page at %p\n", phys_page );

    return phys_page;
}

// Allocates and setups up a page frame for a new entry
void alloc_table_entry( pg_dir_entry_t *parent_entry )
{
    // Allocate a new PD
    pg_dir_entry_t *new_pd = (pg_dir_entry_t *)MMU_pf_alloc();

    // Clear the new PD
    memset( new_pd, 0, PAGE_SIZE );

    // Set the PDPT entry to the new PD
    WRITE_FRAME_ADDR( parent_entry, new_pd );
    parent_entry->present = 1;
    parent_entry->writable = 1;
    parent_entry->user = 1;
}

// Get the Page Table Entry for a virtual address
pg_dir_entry_t *get_pt_entry( void *virt_addr )
{
    // DEBUG: Verify alignment
    CHECK_PAGE_ALIGNED( virt_addr );

    // Get the PML4 entry (Level 4)
    pg_dir_entry_t *dir_table = pml4;
    uint64_t offset = GET_PAGE_MAP_INDEX( virt_addr );
    pg_dir_entry_t *entry = dir_table + offset;

    // Check if the PML4 entry is present
    if ( !entry->present )
    {
        // Allocate a new PDPT entry
        alloc_table_entry( entry );
    }

    // Get the PDPT entry (Level 3)
    dir_table = (pg_dir_entry_t *)READ_FRAME_ADDR( entry );
    offset = GET_PAGE_DIR_PTR_INDEX( virt_addr );
    entry = (pg_dir_entry_t *)( (uint8_t *)dir_table ) + offset;

    // Check if the PDPT entry is present
    if ( !entry->present )
    {
        // Allocate a new PD entry
        alloc_table_entry( entry );
    }

    // Get the PD entry (Level 2)
    dir_table = (pg_dir_entry_t *)READ_FRAME_ADDR( entry );
    offset = GET_PAGE_DIR_INDEX( virt_addr );
    entry = (pg_dir_entry_t *)( (uint8_t *)dir_table ) + offset;

    // Check if the PD entry is present
    if ( !entry->present )
    {
        // Allocate a new PT entry
        alloc_table_entry( entry );
    }

    dir_table = (pg_dir_entry_t *)READ_FRAME_ADDR( entry );
    offset = GET_PAGE_TBL_INDEX( virt_addr );
    entry = (pg_dir_entry_t *)( (uint8_t *)dir_table ) + offset;

    // Get the PT entry (Level 1)
    return entry;
}

// Map a virtual page to a physical page
void map_page( void *phys_addr, void *virt_addr )
{
    // DEBUG: Verify alignment of the physical address
    CHECK_PAGE_ALIGNED( phys_addr );

    // OS_INFO( "Mapping page at %p to %p\n", virt_addr, phys_addr );

    // Get the PT entry
    pg_dir_entry_t *pt_entry = get_pt_entry( virt_addr );

    // Setup the PT entry
    WRITE_FRAME_ADDR( pt_entry, phys_addr );
    pt_entry->present = 1;
    pt_entry->writable = 1;
    pt_entry->user = 1;
    pt_entry->alloc = 0;

    // OS_INFO( "Mapped page at %p to %p\n", virt_addr, phys_addr );
}

// Returns the physical address associated with the given virtual address
void *virt_to_phys( void *virt_addr )
{
    // Get the PT entry
    pg_dir_entry_t *pt_entry = get_pt_entry( virt_addr );

    // Check if the PT entry is present
    if ( !pt_entry->present )
    {
        OS_ERROR( "Page at %p is not present!\n", virt_addr );
        return NULL;
    }

    // Get the physical address
    void *phys_addr = (void *)( READ_FRAME_ADDR( pt_entry ) + GET_PHYS_PAGE_INDEX( virt_addr ) );

    // OS_INFO( "Converted virtual address %p to physical address %p\n", virt_addr, phys_addr );

    return phys_addr;
}

void page_fault_irq( int irq, int err, void *arg )
{
    (void)irq;
    (void)err;
    (void)arg;

    // Read the CR2 Register
    void *cr2;
    asm volatile( "movq %%cr2, %0" : "=r"( cr2 ) );

    OS_ERROR_HALT( "Page Fault IRQ! Virtual address: %p\n", cr2 );
}

void addr_map_init( void *tag_ptr )
{
    uint64_t i;

    // Check if the tag pointer is valid
    if ( tag_ptr == NULL )
    {
        OS_ERROR_HALT( "Invalid multiboot2 tag pointer\n" );
    }

    // Get the MMAP entries from the multiboot2 tag
    get_multiboot2_mmap_info( tag_ptr, &mmap_entries, &num_mmap_entries );

    // Check if there are any MMAP entries
    if ( num_mmap_entries == 0 )
    {
        OS_ERROR( "No memory map entries found???\n" );
        HLT();
    }

    kernel_start_addr = &kernel_start;
    kernel_end_addr = &kernel_end;

    // DEBUG: Check if the kernel start and end addresses are valid
    if ( kernel_start_addr >= kernel_end_addr )
    {
        OS_ERROR_HALT(
            "Kernel start address (%p) is greater than or equal to the kernel end address (%p)!\n",
            kernel_start_addr, kernel_end_addr
        );
    }
    // DEBUG: Kernel start address should be 0x100000
    if ( (uint64_t)kernel_start_addr != 0x100000 )
    {
        OS_ERROR_HALT( "Kernel start address (%p) is not 0x100000\n", &kernel_start_addr );
    }
    // DEBUG: Throw an error if the mmap entries don't start at 0x0
    if ( mmap_entries[0].addr != 0x0 )
    {
        OS_ERROR_HALT( "First MMAP entries start at %p, not 0x0!\n", (void *)mmap_entries[0].addr );
    }

    // Setup the local heap
    memset( local_heap_ptr, 0, PAGE_SIZE );

    addr_range_head = (pf_range_entry_t *)local_heap_ptr;
    addr_range_curr = addr_range_head;

    // Adjust the first MMAP entry to start at page 2
    mmap_entries[0].addr = ( PAGE_SIZE << 1 );

    void *mmap_start, *mmap_end;

    // Start moving the MMAP entries to the linked list of valid addresses
    for ( i = 0; i < num_mmap_entries; ++i )
    {
        // Check if the MMAP entry is available
        if ( mmap_entries[i].type != MULTIBOOT_MEMORY_AVAILABLE )
        {
            continue;
        }

        // Check the kernel heap pointer first
        if ( local_heap_ptr == NULL || local_heap_ptr > (uint8_t *)( PAGE_SIZE << 1 ) )
        {
            OS_ERROR_HALT( "Heap pointer is NULL!? :(\n" );
        }

        mmap_start = (void *)mmap_entries[i].addr;
        mmap_end = (void *)( mmap_entries[i].addr + mmap_entries[i].len );

        // Add a new entry to the linked list
        addr_range_curr->next_entry = (pf_range_entry_t *)local_heap_ptr;
        local_heap_ptr += sizeof( pf_range_entry_t );

        // Move the current pointer to the next entry
        addr_range_curr = addr_range_curr->next_entry;

        // Does the kernel reside in the MMAP entry?
        if ( mmap_start <= kernel_start_addr && kernel_end_addr <= mmap_end )
        {
            // If the kernel starts at the beginning of the MMAP entry, then we need can skip
            // the entry that would have come before the kernel
            if ( kernel_start_addr != mmap_start )
            {
                // Setup the first entry before the kernel
                addr_range_curr->start = mmap_start;
                addr_range_curr->end = kernel_start_addr;
                addr_range_curr->curr_frame = addr_range_curr->start;

                addr_range_curr->next_entry = (pf_range_entry_t *)local_heap_ptr;
                local_heap_ptr += sizeof( pf_range_entry_t );

                addr_range_curr = addr_range_curr->next_entry;
            }

            // Setup the next entry after the kernel
            addr_range_curr->start = PAGE_ALIGN_ADDR( kernel_end_addr );
            addr_range_curr->end = mmap_end;
            addr_range_curr->curr_frame = addr_range_curr->start;
            addr_range_curr->next_entry = NULL;
        }
        else
        {
            // Setup the new entry
            addr_range_curr->start = mmap_start;
            addr_range_curr->end = mmap_end;
            addr_range_curr->curr_frame = addr_range_curr->start;
            addr_range_curr->next_entry = NULL;
        }

        // DEBUG: Check if the linked list of valid addresses is valid
        if ( addr_range_curr == NULL )
        {
            OS_ERROR_HALT( "addr_range_curr is NULL!? i = %ld\n", i );
        }
    }

    // Point the tail to the current entry
    addr_range_tail = addr_range_curr;

    // Move the current pointer to the head
    addr_range_curr = addr_range_head;

    // DEBUG: Check if the linked list of valid addresses is valid
    if ( addr_range_head == NULL || addr_range_tail == NULL )
    {
        OS_ERROR_HALT( "addr_range_head or addr_range_tail is NULL!\n" );
    }
}

/* Public Functions */

// Initialize the MMU and the address space
driver_status_t MMU_init( void *tag_ptr )
{
    uint64_t i;

    // Initialize the address map
    addr_map_init( tag_ptr );

    // Allocate the Page Map Table (Level 4)
    pml4 = MMU_pf_alloc();

    // Clear the PML4
    memset( pml4, 0, PAGE_SIZE );

    // Setup the CR3 Register
    cr3 = MMU_pf_alloc();
    memset( cr3, 0, PAGE_SIZE );
    WRITE_FRAME_ADDR( cr3, pml4 );

    // Map the kernel pages
    printk( "\n" );
    OS_INFO( "Mapping physical pages from %p to %p\n", NULL, kernel_end_addr );
    for ( i = 0; i <= (uint64_t)kernel_end_addr; i += PAGE_SIZE )
    {
        map_page( (void *)i, (void *)i );
    }

    // DEBUG: Check if the kernel pages are mapped
    for ( i = 0; i <= (uint64_t)kernel_end_addr; i += PAGE_SIZE )
    {
        void *temp = virt_to_phys( (void *)i );

        if ( (void *)i != temp )
        {
            OS_ERROR_HALT(
                "Pointer mismatch:    \n"
                "        Physical: %p \n"
                "        Virtual:  %p \n"
                "        \n",
                (void *)i, temp
            );
        }
    }
    OS_INFO( "Successfully mapped %lu kernel pages\n", i / PAGE_SIZE );

    OS_INFO( "PML4 Real Address: %p\n", pml4 );
    OS_INFO( "PML4 CR3 Address: %p\n\n", READ_FRAME_ADDR( cr3 ) );

    // Load the CR3 Register
    OS_INFO( "Loading CR3 Register: %p\n", cr3 );
    asm volatile( "movq %0, %%cr3" : : "r"( cr3 ) );

    OS_INFO( "CR3 Register setup complete\n" );

    // Setup the page fault IRQ
    if ( IRQ_set_exception_handler( IRQ14_PAGE_FAULT, page_fault_irq, NULL ) )
    {
        OS_ERROR_HALT( "Failed to set the page fault IRQ handler\n" );
    }

    return SUCCESS;
}

// Allocate a physical page frame
void *MMU_pf_alloc( void )
{
    // Check if there are any free pages
    if ( pf_free_list_head == NULL )
    {
        // Allocate a new page frame
        return alloc_new_pf();
    }

    // Get the next free page frame
    pf_list_entry_t *pf = pf_free_list_head;
    pf_free_list_head = pf_free_list_head->next;

    OS_INFO( "Page frame allocated at %p\n", pf );

    return pf;
}

// Free a physical page frame
void MMU_pf_free( void *pf )
{
    // DEBUG: Check if the page frame is aligned
    CHECK_PAGE_ALIGNED( pf );

    // Add the page frame to the free list
    ( (pf_list_entry_t *)pf )->next = pf_free_list_head;
    pf_free_list_head = pf;

    OS_INFO( "Page deallocated at %p\n", pf_free_list_head );
}

// Allocate a virtual page in a specific region
void *MMU_page_alloc( virt_addr_t region )
{
    // Get the next available virtual address
    void *virt_page = virt_addr[region];

    // Setup the page table entry
    pg_dir_entry_t *pt_entry = get_pt_entry( virt_page );

    // Set the allocate on demand bit
    pt_entry->alloc = 1;
    pt_entry->present = 0;

    // Increment the virtual address
    virt_addr[region] += PAGE_SIZE;

    OS_INFO( "Allocated virtual page at %p\n", virt_page );

    return virt_page;
}

// Free a virtual page
void MMU_page_free( void *page )
{
    //
    (void)page;
}

/*** End of File ***/