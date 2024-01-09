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

#include "printk.h"
#include "util.h"

/* Private Defines and Macros */

/* Global Variables */

/* Private Functions */

/* Public Functions */
int kernel_main( void )
{
    printk( "Hello, World!\n" );

    for ( int i = 0; i < 3; i++ )
    {
        printk( "This is printk() call #%d\n", i );
    }

    // printk(
    //     "-----------------------------------------\n"
    //     "Testing MAX and MIN values...            \n"
    //     "SHRT_MIN = %hd                           \n"
    //     "SHRT_MAX = %hd                           \n"
    //     "USHRT_MAX = %hu                          \n"
    //     "INT_MIN = %d                             \n"
    //     "INT_MAX = %d                             \n"
    //     "UINT_MAX = %u                            \n"
    //     "LONG_MIN = %ld                           \n"
    //     "LONG_MAX = %ld                           \n"
    //     "ULONG_MAX = %lu                          \n"
    //     "LLONG_MIN = %qd                          \n"
    //     "LLONG_MAX = %qd                          \n"
    //     "ULLONG_MAX = %qu                         \n"
    //     "-----------------------------------------\n"
    //     "Testing strings, chars, and pointers...  \n"
    //     "%s                                       \n"
    //     "%c %c %c %c %c                           \n"
    //     "%p                                       \n"
    //     "-----------------------------------------\n"
    //     "Done!                                    \n",
    //     SHRT_MIN, SHRT_MAX, USHRT_MAX, INT_MIN, INT_MAX, UINT_MAX, LONG_MIN, LONG_MAX, ULONG_MAX,
    //     LLONG_MIN, LLONG_MAX, ULLONG_MAX, "Hello, World!\n", 'p', 'o', 'o', 'p', '!',
    //     &kernel_main
    //);

    printk( "                                        \n" );
    printk( "Testing printk()...                     \n" );
    printk( "----------------------------------------\n" );
    printk( "Testing MAX and MIN values...           \n" );
    printk( "                                        \n" );
    printk( "SHRT_MIN.......%hd                      \n", SHRT_MIN );
    printk( "SHRT_MAX.......%hd                      \n", SHRT_MAX );
    printk( "USHRT_MAX......%hu                      \n", USHRT_MAX );
    printk( "                                        \n" );
    printk( "INT_MIN........%d                       \n", INT_MIN );
    printk( "INT_MAX........%d                       \n", INT_MAX );
    printk( "UINT_MAX.......%u                       \n", UINT_MAX );
    printk( "                                        \n" );
    printk( "LONG_MIN.......%ld                      \n", LONG_MIN );
    printk( "LONG_MAX.......%ld                      \n", LONG_MAX );
    printk( "ULONG_MAX......%lu                      \n", ULONG_MAX );
    printk( "                                        \n" );
    printk( "LLONG_MIN......%qd                      \n", LLONG_MIN );
    printk( "LLONG_MAX......%qd                      \n", LLONG_MAX );
    printk( "ULLONG_MAX.....%qu                      \n", ULLONG_MAX );
    printk( "----------------------------------------\n" );
    printk( "String: %s                              \n", "Hello, World!" );
    printk( "Char: %c                                \n", 'B' );
    printk( "Hex: %x                                 \n", 'B' );
    printk( "Pointer: %p                             \n", &kernel_main );
    printk( "----------------------------------------\n" );
    printk( "Done!                                   \n" );

    return 0;
}

/*** End of File ***/