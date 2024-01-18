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
#include "tests.h"
#include "vga_driver.h"

int kernel_main( void )
{
    // Initialize the VGA driver
    if ( vga_driver_init() == FAILURE )
    {
        printk( "vga_driver_init() failed!\n" );

        return 1;
    }

    printk( "vga_driver_init() succeeded!\n" );

    // Initialize the keyboard driver
    if ( keyboard_driver_polling_init() == FAILURE )
    {
        printk( "keyboard_driver_polling_init() failed!\n" );

        return 1;
    }

    printk( "keyboard_driver_polling_init() succeeded!\n" );

    //// Poll the keyboard driver and print the character to the screen
    // while ( 1 )
    //{
    //     char c = keyboard_driver_polling_get_char();
    //     if ( c != 0 )
    //     {
    //         printk( "%c", c );
    //     }
    // }

    // Run tests

    test_printk();

    return 0;
}

/*** End of File ***/