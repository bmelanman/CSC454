/** @file tests.c
 *
 * @brief A description of the module's purpose.
 *
 * @author Bryce Melander
 * @date Jan-18-2024
 *
 * @copyright (c) 2024 by Bryce Melander under MIT License. All rights reserved.
 * (See http://opensource.org/licenses/MIT for more details.)
 */

#include "tests.h"

#include <limits.h>

#include "keyboard_driver_polling.h"
#include "printk.h"

/* Private Functions */

int pointer_func( void ) { return 0; }

/* Public Functions */

void test_printk( void )
{
    // Tests the following flags: %h[dux] %l[dux] %q[dux] %s

    printk( "                                        \n" );
    printk( "Begining printk() tests...              \n" );
    printk( "----------------------------------------\n" );
    printk( "Testing %%d:                            \n" );
    printk( "  INT_MIN........%d                     \n", INT_MIN );
    printk( "  INT_MAX.........%d                    \n", INT_MAX );
    printk( "                                        \n" );
    printk( "Testing %%u:                            \n" );
    printk( "  UINT_MAX........%u                    \n", UINT_MAX );
    printk( "                                        \n" );
    printk( "Testing %%X:                            \n" );
    printk( "  0x12345678......0x%X                  \n", 0x12345678 );
    printk( "                                        \n" );
    printk( "Testing %%c:                            \n" );
    printk( "  Test character..%c                    \n", '@' );
    printk( "                                        \n" );
    printk( "Testing %%s:                            \n" );
    printk( "  Test string.....%s                    \n", "Hello,_world!" );
    printk( "                                        \n" );
    printk( "Testing %%p:                            \n" );
    printk( "  &kernel_main....%p                    \n", &pointer_func );
    printk( "                                        \n" );

    // Wait for the user to press a key
    printk( "Press any key to continue the test...   \n" );
    keyboard_driver_polling_get_char();

    printk( "                                        \n" );
    printk( "Testing %%h[dux]:                       \n" );
    printk( "  SHRT_MIN.......%hd                    \n", SHRT_MIN );
    printk( "  SHRT_MAX........%hd                   \n", SHRT_MAX );
    printk( "  USHRT_MAX.......%hu                   \n", USHRT_MAX );
    printk( "  USHRT_MAX.......0x%hx                 \n", USHRT_MAX );
    printk( "                                        \n" );
    printk( "Testing %%l[dux]:                       \n" );
    printk( "  LONG_MIN.......%ld                    \n", LONG_MIN );
    printk( "  LONG_MAX........%ld                   \n", LONG_MAX );
    printk( "  ULONG_MAX.......%lu                   \n", ULONG_MAX );
    printk( "  ULONG_MAX.......0x%lx                 \n", ULONG_MAX );
    printk( "                                        \n" );
    printk( "Testing %%q[dux]:                       \n" );
    printk( "  LLONG_MIN......%qd                    \n", LLONG_MIN );
    printk( "  LLONG_MAX.......%qd                   \n", LLONG_MAX );
    printk( "  ULLONG_MAX......%qu                   \n", ULLONG_MAX );
    printk( "  ULLONG_MAX......0x%qx                 \n", ULLONG_MAX );
    printk( "----------------------------------------\n" );
    printk( "Done!                                   \n" );
    printk( "                                        \n" );
}

/*** End of File ***/