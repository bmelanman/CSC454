/** @file kernel_main.c
 *
 * @brief A description of the module's purpose.
 *
 * @author Bryce Melander
 * @date Jan-08-2024
 *
 * @copyright (c) 2024 by Bryce Melander under MIT License. All rights reserved.
 * (See http://opensource.org/licenses/MIT for more details.)
 */

#include "ISR/irq_handler.h"
#include "common.h"
#include "keyboard_driver_polling.h"
#include "splash.h"
#include "tests.h"
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

    // Poll the keyboard driver and print the character to the screen
    while ( 1 )
    {
        char c = keyboard_driver_polling_get_char();

        if ( c != 0 )
        {
            printk( "%c", c );
        }
    }

    return 0;
}

int keyboard_init( void )
{
    // Initialize the keyboard driver
    if ( keyboard_driver_polling_init() == FAILURE )
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
    // Clear the screen
    VGA_clear();

    OS_INFO( "Beginning system initialization...\n" );

    if ( keyboard_init() ) return 1;

    if ( ISR_init() ) return 1;

    OS_INFO( "System initialization is complete!\n" );

    // Wait for a second
    io_wait_n( (uint64_t)1e6 );

    // Clear the screen
    VGA_clear();

    // Print the splash screen
    splash_screen();

    return 0;
}

/*** End of File ***/