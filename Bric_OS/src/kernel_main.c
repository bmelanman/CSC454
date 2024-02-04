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

#include "common.h"
#include "irq_handler.h"
#include "multiboot2.h"
#include "ps2_keyboard_driver.h"
#include "serial_io_driver.h"
#include "splash.h"
#include "vga_driver.h"

int system_initialization( void );

int kernel_main( void )
{
    // Run initialization
    if ( system_initialization() )
    {
        OS_ERROR( "System initialization failed!\n" );
        return 1;
    }

    /*
    // Test printing an escape sequence
    // printk( "\033[2J" );  // Clear the screen

    //// Test multiboot header parsing
    // if ( multiboot_parse_tags() )
    //{
    //     OS_ERROR( "Multiboot header parsing failed!\n" );
    //     return 1;
    // }*/

    char str[64] = "Hello0\n";
    size_t len, str_len = strlen( str );

    // Wait a bit
    // for ( size_t i = 0; i < 1e6; i++ )
    //{
    //    io_wait_n( (uint64_t)2e7 );
    //}

    while ( 1 )
    {
        // DEBUG: Print a message to the VGA buffer
        // OS_INFO( "Writing \"%s\" (%lu bytes) to serial output.\n", str, str_len );

        // Write to the serial port
        len = serial_write( str, str_len );

        // DEBUG: Print a message to the VGA buffer
        OS_INFO( "Wrote %lu bytes to serial output.\n", len );

        str[5] += 1;

        if ( str[5] > 'z' )
        {
            str[5] = '0';
        }

        // Wait a bit
        io_wait_n( (uint64_t)1e7 );
    }

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

    OS_INFO( "Serial driver initialization is complete!\n" );

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

int system_initialization( void )
{
    // Disable interrupts
    CLI();

    // Clear the screen
    VGA_clear();

    OS_INFO( "Beginning system initialization...\n" );

    if ( SER_init() ) return 1;

    if ( VGA_init() ) return 1;

    if ( ISR_init() ) return 1;

    if ( KB_init() ) return 1;

    OS_INFO( "System initialization is complete!\n" );

    // Wait a bit
    // io_wait_n( (uint64_t)2e7 );

    // Print the splash screen
    splash_screen();

    // Enable interrupts
    STI();

    //// Clear the screen
    // VGA_clear();

    printk( "\n" );

    return 0;
}

/*** End of File ***/