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

#include <limits.h>

#include "keyboard_driver_polling.h"
#include "printk.h"
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

    // Run tests
    // test_printk();

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

int system_initialization( void )
{
    OS_INFO( "Beginning system initialization...\n" );

    // Initialize the VGA driver
    OS_INFO( "Initializing VGA driver...\n" );
    if ( vga_driver_init() == FAILURE )
    {
        OS_ERROR( "VGA driver initialization failed!\n" );
        return 1;
    }

    OS_INFO( "VGA driver initialization was successful!\n" );

    // Initialize the keyboard driver
    OS_INFO( "Initializing keyboard driver...\n" );
    if ( keyboard_driver_polling_init() == FAILURE )
    {
        OS_ERROR( "Keyboard driver initialization failed!\n" );
        return 1;
    }

    OS_INFO( "Keyboard driver initialization was successful!\n" );

    // Init complete, print splash screen
    OS_INFO( "System initialization is complete!\n" );

    vga_clear();

    splash_screen();

    return 0;
}

/*** End of File ***/