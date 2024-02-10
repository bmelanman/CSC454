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

int kernel_main( unsigned long magic, unsigned long addr )
{
    // Run initialization
    if ( system_initialization( magic, addr ) )
    {
        OS_ERROR( "System initialization failed!\n" );
        HLT();
    }

    uint i, j, num_pages = 10;
    uint8_t test_pattern;

    // Test the memory manager
    OS_INFO( "Testing memory manager...\n" );

    OS_INFO( "Allocating %d pages...\n", num_pages );
    void *first_pg = MMU_pf_alloc_n( (int)num_pages ), *curr_pg = first_pg;

    for ( i = 0; i < num_pages; i++ )
    {
        test_pattern = (uint8_t)( (uintptr_t)curr_pg );

        OS_INFO( "Page %d: %p\n", i, curr_pg );

        // Write a test pattern to the page
        OS_INFO( "Writing test pattern to page %d...\n", i );

        memset( curr_pg, test_pattern, PAGE_SIZE );

        // Verify the test pattern
        for ( j = 0; j < PAGE_SIZE; j++ )
        {
            if ( *( (uint8_t *)curr_pg + j ) != test_pattern )
            {
                OS_ERROR( "Memory test failed!\n" );
                HLT();
            }
        }

        // Move to the next page
        curr_pg = (void *)( (uintptr_t)curr_pg + PAGE_SIZE );
    }

    // Free the pages
    OS_INFO( "Freeing %d pages...\n", num_pages );
    MMU_pf_free_n( first_pg, (int)num_pages );

    // HLT();

    return 0;
}

int MEM_init( unsigned long magic, unsigned long addr )
{
    // Check if the multiboot2 header is valid
    if ( magic != MULTIBOOT2_BOOTLOADER_MAGIC )
    {
        OS_ERROR( "Invalid Multiboot2 header!\n" );
        return 1;
    }

    // Initialize the memory manager
    if ( MMU_init( (void *)addr ) == FAILURE )
    {
        OS_ERROR( "Memory manager initialization failed!\n" );
        return 1;
    }

    OS_INFO( "Memory manager initialization is complete!\n" );

    return 0;
}

int VGA_init( void )
{
    // Initialize the VGA driver
    if ( VGA_driver_init() == FAILURE )
    {
        OS_ERROR( "VGA driver initialization failed!\n" );
        return 1;
    }

    VGA_clear();

    OS_INFO( "VGA driver initialization is complete!\n" );

    return 0;
}

int SER_init( void )
{
    // Initialize the serial driver
    if ( serial_driver_init() == FAILURE )
    {
        OS_ERROR( "Serial driver initialization failed!\n" );
        return 1;
    }

    // Send a clear screen command
    serial_print( "\033c" );

    OS_INFO( "Serial driver initialization is complete!\n" );

    return 0;
}

int ISR_init( void )
{
    // Initialize the ISR
    if ( IRQ_init() == FAILURE )
    {
        OS_ERROR( "ISR initialization failed!\n" );
        return 1;
    }

    OS_INFO( "ISR initialization is complete!\n" );

    return 0;
}

int KB_init( void )
{
    // Initialize the keyboard driver
    if ( ps2_keyboard_driver_init( true ) == FAILURE )
    {
        OS_ERROR( "Keyboard driver initialization failed!\n" );
        return 1;
    }

    OS_INFO( "Keyboard driver initialization is complete!\n" );

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