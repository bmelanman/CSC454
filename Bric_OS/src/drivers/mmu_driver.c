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

#define BYTE_ALIGN_LEN       ( 8U )
#define BTYE_ALIGN_ADDR( x ) ( (void *)ALIGN( (uintptr_t)( x ), BYTE_ALIGN_LEN ) )

#define PAGE_MAP_OFFSET_MASK     ( 0x1FFU )  // 9 bits
#define PAGE_DIR_PTR_OFFSET_MASK ( 0x1FFU )  // 9 bits
#define PAGE_DIR_OFFSET_MASK     ( 0x1FFU )  // 9 bits
#define PAGE_TBL_OFFSET_MASK     ( 0x1FFU )  // 9 bits
#define PHYS_PAGE_OFFSET_MASK    ( 0xFFFU )  // 12 bits

#define GET_PAGE_MAP_OFFSET( x )     ( (uintptr_t)( x ) & PAGE_MAP_OFFSET_MASK )
#define GET_PAGE_DIR_PTR_OFFSET( x ) ( (uintptr_t)( x ) & PAGE_DIR_PTR_OFFSET_MASK )
#define GET_PAGE_DIR_OFFSET( x )     ( (uintptr_t)( x ) & PAGE_DIR_OFFSET_MASK )
#define GET_PAGE_TBL_OFFSET( x )     ( (uintptr_t)( x ) & PAGE_TBL_OFFSET_MASK )
#define GET_PHYS_PAGE_OFFSET( x )    ( (uintptr_t)( x ) & PHYS_PAGE_OFFSET_MASK )

#define READ_FRAME_ADDR( x )  ( (void *)( (uintptr_t)( x )->frame_addr << 12 ) )
#define WRITE_FRAME_ADDR( x ) ( (uint64_t)( (uintptr_t)( x ) >> 12 ) )

#define CHECK_PAGE_ALIGNED( x )                                                \
    do                                                                         \
    {                                                                          \
        if ( (uintptr_t)( x ) % PAGE_SIZE != 0 )                               \
        {                                                                      \
            OS_ERROR_HALT( "Address %p is not aligned to 8 bytes!\n", ( x ) ); \
            HLT();                                                             \
        }                                                                      \
    } while ( 0 )

#define PRESENT_BIT_MASK    ( 1U << 0U )
#define READ_WRITE_BIT_MASK ( 1U << 1U )
#define USER_SUPER_BIT_MASK ( 1U << 2U )
#define ACCESSED_BIT_MASK   ( 1U << 5U )
#define DIRTY_BIT_MASK      ( 1U << 6U )

/* Private Types and Enums */

// Page Frame Linked List Entry
typedef struct pf_list_entry_s pf_list_entry_t;
struct pf_list_entry_s
{
    pf_list_entry_t *next;  // 8 bytes
};

// CR3 Register Entry
typedef struct pg_map_entry_s
{                                 // 8 bytes (64 bits)
    uint64_t reserved1 : 3;       // Reserved Bits
    uint64_t write_through : 1;   // Page-Level Write-Through
    uint64_t cache_disabled : 1;  // Page-Level Cache Disable
    uint64_t reserved2 : 7;       // Reserved Bits
    uint64_t frame_addr : 40;     // Frame Address
    uint64_t reserved3 : 12;      // Reserved Bits
} __packed page_map_entry_t;

// Page Table, Page Directory, Directory Pointer Table, and Page Map Entry (Levels 1 - 4)
typedef struct pg_dir_entry_s
{
    uint64_t present : 1;           // Present Bit
    uint64_t writable : 1;          // Read/Write Bit
    uint64_t user : 1;              // User/Supervisor Bit
    uint64_t accessed : 1;          // Accessed Bit
    uint64_t dirty : 1;             // Dirty Bit                <-- Used by Page Table Entries
    uint64_t alloc_on_write : 1;    // Allocate on Demand Bit   <--
    uint64_t unused : 16;           // Unused Bits
    uint64_t frame_addr : 40;       // Frame Address ( 40 bits for Page Map Entries )
    uint64_t no_execute : 1;        // No-Execute Bit
} __packed page_directory_entry_t;  // 64 bits Total

/* Global Variables */

// MMAP entries from multiboot2
static mb_mmap_entry_t *mmap_entries = NULL;
static uint32_t num_mmap_entries = 0;
static long curr_mmap_entry = -1;

// Current page frame address (for allocation)
static void *curr_pf_addr = NULL;
static uint64_t curr_remaining_len = 0;

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

// Kernel start and end addresses
extern uint64_t kernel_start;
extern uint64_t kernel_end;

// Page Map Table (Level 4)
static page_directory_entry_t *pml4 = NULL;

// Free page frames
static pf_list_entry_t *pf_free_list_head = NULL;

/* Private Functions */

// Allocates a new page frame from the list of MMAP entries
void *get_next_phys_page( void )
{
    uint i;
    void *phys_page = NULL;

    // Get a new MMAP entry if the current one is out of pages
    if ( curr_remaining_len < PAGE_SIZE )
    {
        // Mark the current MMAP entry as used
        if ( curr_mmap_entry >= 0 )
        {
            mmap_entries[curr_mmap_entry].type = MULTIBOOT_MEMORY_RESERVED;
        }

        // Parse through the MMAP entries (starting at the next MMAP entry index)
        for ( i = ( curr_mmap_entry + 1 ); i < num_mmap_entries; i++ )
        {
            // Look for an available page
            if ( MEM_IS_AVAILABLE( mmap_entries[i] ) )
            {
                // Set the current page frame address
                curr_pf_addr = (void *)mmap_entries[i].addr;
                curr_remaining_len = mmap_entries[i].len;

                break;
            }
        }

        // Check if we actually found a page
        if ( curr_remaining_len < PAGE_SIZE )
        {
            OS_ERROR_HALT( "No available memory found???\n" );
        }

        // Set the current page frame index
        curr_mmap_entry = i;
    }

    // Get the current page frame address
    phys_page = curr_pf_addr;

    // Adjust the current page frame address
    curr_pf_addr += PAGE_SIZE;
    curr_remaining_len -= PAGE_SIZE;

    // Clear the page
    memset( phys_page, 0, PAGE_SIZE );

    OS_INFO( "Allocated NEW physical page at %p\n", phys_page );

    return phys_page;
}

// Allocates and setups up a page frame for a new entry
void alloc_entry( page_directory_entry_t *parent_entry )
{
    // Allocate a new PD
    page_directory_entry_t *new_pd = (page_directory_entry_t *)MMU_pf_alloc();

    // Set the PDPT entry to the new PD
    parent_entry->frame_addr = WRITE_FRAME_ADDR( new_pd );
    parent_entry->present = 1;
    parent_entry->writable = 1;
    parent_entry->user = 1;
}

// Get the Page Table Entry for a virtual address
page_directory_entry_t *get_pt_entry( void *virt_addr )
{
    // DEBUG: Check if the virtual address is aligned, not sure if this is actually necessary lol
    if ( (uintptr_t)virt_addr % BYTE_ALIGN_LEN != 0 )
    {
        OS_WARN( "Virtual address %p is not aligned to 8 bytes!\n", virt_addr );
    }

    // Get the PML4 entry (Level 4)
    page_directory_entry_t *dir_table = pml4;
    page_directory_entry_t *entry = dir_table + GET_PAGE_MAP_OFFSET( virt_addr );

    // Check if the PML4 entry is present
    if ( !entry->present )
    {
        // Allocate a new PDPT entry
        alloc_entry( entry );
    }

    // Get the PDPT entry (Level 3)
    dir_table = READ_FRAME_ADDR( entry );
    entry = dir_table + GET_PAGE_DIR_PTR_OFFSET( virt_addr );

    // Check if the PDPT entry is present
    if ( !entry->present )
    {
        // Allocate a new PD entry
        alloc_entry( entry );
    }

    // Get the PD entry (Level 2)
    dir_table = READ_FRAME_ADDR( entry );
    entry = dir_table + GET_PAGE_DIR_OFFSET( virt_addr );

    // Check if the PD entry is present
    if ( !entry->present )
    {
        // Allocate a new PT entry
        alloc_entry( entry );
    }

    // Get the PT entry (Level 1)
    page_directory_entry_t *pt = READ_FRAME_ADDR( entry );

    return pt + GET_PAGE_TBL_OFFSET( virt_addr );
}

// Map a virtual page to a physical page
void map_page( void *phys_addr, void *virt_addr )
{
    // DEBUG: Verify alignment
    CHECK_PAGE_ALIGNED( phys_addr );
    CHECK_PAGE_ALIGNED( virt_addr );

    OS_INFO( "Mapping page at %p to %p\n", virt_addr, phys_addr );

    // Get the PT entry
    page_directory_entry_t *pt_entry = get_pt_entry( virt_addr );

    // Set the PT entry
    pt_entry->frame_addr = WRITE_FRAME_ADDR( phys_addr );
    pt_entry->present = 1;
    pt_entry->writable = 1;
    pt_entry->user = 1;

    OS_INFO( "Mapped page at %p to %p\n", virt_addr, phys_addr );
}

// Convert a virtual address to a physical address
void *virt_to_phys( void *virt_addr )
{
    // Get the PT entry
    page_directory_entry_t *pt_entry = get_pt_entry( virt_addr );

    // Check if the PT entry is present
    if ( !pt_entry->present )
    {
        OS_ERROR( "Page at %p is not present!\n", virt_addr );
        return NULL;
    }

    // Get the physical address
    void *phys_addr =
        (void *)( ( pt_entry->frame_addr << 12 ) + GET_PHYS_PAGE_OFFSET( virt_addr ) );

    OS_INFO( "Converted virtual address %p to physical address %p\n", virt_addr, phys_addr );

    return phys_addr;
}

void addr_space_init( void *tag_ptr )
{
    uint i;

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

    void *kernel_start_addr = &kernel_start, *kernel_end_addr = &kernel_end;
    uint64_t kernel_addr_len = (uint64_t)kernel_end_addr - (uint64_t)kernel_start_addr;

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

    OS_INFO( "Kernel start address is %p\n", kernel_start_addr );
    OS_INFO( "Kernel end address is %p\n", kernel_end_addr );

    bool found_kernel = false;
    void *mmap_entry_start = NULL, *mmap_entry_end = NULL;

    // Remove the kernel from the MMAP entries
    for ( i = 0; i < num_mmap_entries; ++i )
    {
        mmap_entry_start = (void *)mmap_entries[i].addr;
        mmap_entry_end = (void *)( mmap_entries[i].addr + mmap_entries[i].len );

        if ( mmap_entry_start <= kernel_start_addr && kernel_end_addr <= mmap_entry_end )
        {
            // The kernel SHOULD start at the beginning of an MMAP entry
            if ( mmap_entry_start != kernel_start_addr )
            {
                OS_ERROR_HALT(
                    "Kernel start address (%p) is not at the beginning of an MMAP entry (%p)!\n",
                    kernel_start_addr, mmap_entry_start
                );
            }

            // Remove the kernel from the MMAP entry
            mmap_entries[i].addr += kernel_addr_len;
            mmap_entries[i].len -= kernel_addr_len;

            // DEBUG: If the length is 0, mark the entry as reserved (should never happen?)
            if ( mmap_entries[i].len == 0 )
            {
                mmap_entries[i].type = MULTIBOOT_MEMORY_RESERVED;
                OS_INFO( "MMAP entry %d is now reserved\n", i );
            }

            found_kernel = true;
        }
    }

    // Check if the kernel was found
    if ( !found_kernel )
    {
        OS_ERROR_HALT( "Kernel was not found in the MMAP entries???\n" );
    }
}

void page_fault_irq( int irq, int err, void *arg )
{
    (void)irq;
    (void)err;
    (void)arg;

    OS_ERROR( "Page Fault IRQ\n" );
    OS_ERROR( "Halting!\n" );
    HLT();
}

/* Public Functions */

// Initialize the MMU and the address space
driver_status_t MMU_init( void *tag_ptr )
{
    // Initialize the address space
    addr_space_init( tag_ptr );

    // Allocate the first page frame for the free list
    pf_free_list_head = get_next_phys_page();

    // Ignore the first page frame if its NULL
    if ( pf_free_list_head == NULL )
    {
        OS_WARN( "First page frame is NULL, ignoring...\n" );

        pf_free_list_head = get_next_phys_page();
    }

    // Allocate the Page Map Table (Level 4)
    pml4 = MMU_pf_alloc();

    return SUCCESS;
}

// Allocate a physical page frame
void *MMU_pf_alloc( void )
{
    // Check if there are any free pages
    if ( pf_free_list_head == NULL )
    {
        // Allocate a new page frame
        return get_next_phys_page();
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
    // Add the page frame to the free list
    ( (pf_list_entry_t *)pf )->next = pf_free_list_head;
    pf_free_list_head = pf;

    OS_INFO( "Page deallocated at %p\n", pf_free_list_head );
}

// Allocate a virtual page
void *MMU_alloc_region( virt_addr_t region )
{
    // Get the next available virtual address
    void *virt_page = virt_addr[region];

    // Setup the page table entry
    page_directory_entry_t *pt_entry = get_pt_entry( virt_page );

    // Set the allocate on demand bit
    pt_entry->alloc_on_write = 1;
    pt_entry->present = 0;

    // Increment the virtual address
    virt_addr[region] += PAGE_SIZE;

    OS_INFO( "Allocated virtual page at %p\n", virt_page );

    return virt_page;
}

// Free a virtual page
void MMU_free_page( void *page )
{
    //
    (void)page;
}

/*** End of File ***/