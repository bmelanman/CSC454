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

/* Public Functions */
int kernel_main( void )
{
    printk( "Hello, World!\n" );

    for ( int i = 0; i < 3; i++ )
    {
        printk( "This is printk() call #%d\n", i );
    }

    printk( "                                        \n" );
    printk( "Testing printk()...                     \n" );
    printk( "----------------------------------------\n" );
    printk( "Testing MAX and MIN values...           \n" );
    printk( "                                        \n" );
    printk( "SHRT_MIN.......%hd                      \n", SHRT_MIN );
    printk( "SHRT_MAX.......%hd                      \n", SHRT_MAX );
    printk( "USHRT_MAX......%hu (0x%hx)              \n", USHRT_MAX, USHRT_MAX );
    printk( "                                        \n" );
    printk( "INT_MIN........%d                       \n", INT_MIN );
    printk( "INT_MAX........%d                       \n", INT_MAX );
    printk( "UINT_MAX.......%u (0x%x)                \n", UINT_MAX, UINT_MAX );
    printk( "                                        \n" );
    printk( "LONG_MIN.......%ld                      \n", LONG_MIN );
    printk( "LONG_MAX.......%ld                      \n", LONG_MAX );
    printk( "ULONG_MAX......%lu (0x%lx)              \n", ULONG_MAX, ULONG_MAX );
    printk( "                                        \n" );
    printk( "LLONG_MIN......%lld                     \n", LLONG_MIN );
    printk( "LLONG_MAX......%lld                     \n", LLONG_MAX );
    printk( "ULLONG_MAX.....%llu (0x%llx)            \n", ULLONG_MAX, ULLONG_MAX );
    printk( "----------------------------------------\n" );
    printk( "String.........%s                       \n", "Hello, World!" );
    printk( "Char...........%c                       \n", 'B' );
    printk( "Hex............0x%X                     \n", 0xdeadbeef );
    printk( "Pointer........%p                       \n", &kernel_main );
    printk( "----------------------------------------\n" );
    printk( "Done!                                   \n" );

    return 0;
}

/*** End of File ***/