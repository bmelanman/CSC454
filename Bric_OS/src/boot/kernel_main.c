/** @file kernel_main.c
 *
 * @brief Main kernel implementation.
 *
 * @author Bryce Melander
 * @date Jan-08-2024
 *
 * @copyright (c) 2024 by Bryce Melander under MIT License. All rights reserved.
 * (See http://opensource.org/licenses/MIT for more details.)
 */

/* Includes */

#include "common.h"
#include "irq_handler.h"
#include "multiboot2.h"
#include "ps2_keyboard_driver.h"
#include "serial_io_driver.h"
#include "splash.h"
#include "vga_driver.h"

/* Testing Area */

#include "mmu_driver.h"

int system_initialization( unsigned long magic, unsigned long addr );

void test_page( uint8_t *page )
{
    uint j, test_pattern = ( (uint64_t)page + 0xA5 ) & 0xFF;

    // Write a test pattern to the page
    OS_INFO( "Writing test pattern to page...\n" );

    memset( page, (uint8_t)test_pattern, PAGE_SIZE );

    // Verify the test pattern
    for ( j = 0; j < PAGE_SIZE; j++ )
    {
        if ( *( page + j ) != test_pattern )
        {
            OS_ERROR_HALT( "Memory test failed!\n" );
        }
    }

    printk( "\n" );
}

void test_pf( void )
{
    uint i, num_pages = 10;

    // Test the memory manager
    OS_INFO( "Testing memory manager...\n" );

    OS_INFO( "Allocating %d pages...\n", num_pages );
    void *page_frames[num_pages];

    for ( i = 0; i < num_pages; i++ )
    {
        page_frames[i] = MMU_pf_alloc();

        OS_INFO( "Page %d: %p\n", i, page_frames[i] );

        test_page( page_frames[i] );
    }

    // Free the pages
    OS_INFO( "Freeing %d pages...\n", num_pages >> 1 );

    for ( i = 0; i < num_pages >> 1; ++i )
    {
        MMU_pf_free( page_frames[i] );
    }

    // Allocate a page after deallocating some
    page_frames[--i] = MMU_pf_alloc();

    OS_INFO( "Page %d: %p\n", i, page_frames[i] );

    test_page( page_frames[i] );

    // Free the remaining pages
    OS_INFO( "Freeing %d pages...\n", num_pages >> 1 );

    for ( ; i < num_pages; ++i )
    {
        MMU_pf_free( page_frames[i] );
    }

    OS_INFO( "Memory manager test is complete.\n" );
}

void test_virt_pages( void )
{
    // Allocate a virtual page in the kernel heap
    uint8_t *page = MMU_page_alloc( VIRT_ADDR_KERNEL_HEAP );

    OS_INFO( "Virtual Page: %p\n", page );

    test_page( page );

    // Free the virtual page
    MMU_page_free( page );
}

int kernel_main( unsigned long magic, unsigned long addr )
{
    // Run initialization
    if ( system_initialization( magic, addr ) )
    {
        OS_ERROR( "System initialization failed!\n" );
        HLT();
    }

    // Test the memory manager
    // test_pf();
    test_virt_pages();

    OS_INFO( "Done!\n" );

    HLT();

    return 0;
}

int MEM_init( unsigned long magic, unsigned long addr )
{
    // Check if the multiboot2 header is valid
    if ( magic != MULTIBOOT2_BOOTLOADER_MAGIC )
    {
        OS_ERROR( "\nInvalid Multiboot2 header!\n" );
        return 1;
    }

    // Initialize the memory manager
    if ( MMU_init( (void *)addr ) == FAILURE )
    {
        OS_ERROR( "\nMemory manager initialization failed!\n" );
        return 1;
    }

    OS_INFO( "Memory manager initialization is complete!\n\n" );

    return 0;
}

int VGA_init( void )
{
    // Initialize the VGA driver
    if ( VGA_driver_init() == FAILURE )
    {
        OS_ERROR( "\nVGA driver initialization failed!\n" );
        return 1;
    }

    VGA_clear();

    OS_INFO( "VGA driver initialization is complete!\n\n" );

    return 0;
}

int SER_init( void )
{
    // Initialize the serial driver
    if ( serial_driver_init() == FAILURE )
    {
        OS_ERROR( "\nSerial driver initialization failed!\n" );
        return 1;
    }

    // Send a clear screen command
    serial_print( "\033c" );

    OS_INFO( "Serial driver initialization is complete!\n\n" );

    return 0;
}

int ISR_init( void )
{
    // Initialize the ISR
    if ( IRQ_init() == FAILURE )
    {
        OS_ERROR( "\nISR initialization failed!\n" );
        return 1;
    }

    OS_INFO( "ISR initialization is complete!\n\n" );

    return 0;
}

int KB_init( void )
{
    // Initialize the keyboard driver
    if ( ps2_keyboard_driver_init( true ) == FAILURE )
    {
        OS_ERROR( "\nKeyboard driver initialization failed!\n" );
        return 1;
    }

    OS_INFO( "Keyboard driver initialization is complete!\n\n" );

    return 0;
}

int system_initialization( unsigned long magic, unsigned long addr )
{
    if ( VGA_init() ) return 1;

    if ( SER_init() ) return 1;

    if ( MEM_init( magic, addr ) ) return 1;

    if ( ISR_init() ) return 1;

    if ( KB_init() ) return 1;

    OS_INFO( "System initialization is complete!\n" );

    // Print the splash screen
    splash_screen();

    // Enable interrupts
    IRQ_enable();

    printk( "\n" );

    return 0;
}

/*** End of File ***/