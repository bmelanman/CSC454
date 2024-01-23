/** @file util.h
 *
 * @brief A description of the module's purpose.
 *
 * @author Bryce Melander
 * @date Jan-09-2024
 *
 * @copyright (c) 2024 by Bryce Melander under MIT License. All rights reserved.
 * (See http://opensource.org/licenses/MIT for more details.)
 */

#ifndef UTIL_H
# define UTIL_H

/* Includes */

# include <stdarg.h>
# include <stddef.h>
# include <stdint.h>
# include <string.h>

# include "printk.h"

/* Defines */

# define TRUE  ( 1U )
# define FALSE ( 0U )

/* Macros */

/* Typedefs */

typedef enum { SUCCESS = 0, FAILURE = 1 } driver_status_t;

/* Public Functions */

// Read a byte from the specified address
# define ASM_read8( addr )                                                           \
        ( {                                                                          \
            uint8_t ret;                                                             \
            asm volatile( "inb %%dx, %%al" : "=a"( ret ) : "d"( addr ) : "memory" ); \
            ret;                                                                     \
        } )
// Read a word from the specified address
# define ASM_read16( addr )                                                          \
        ( {                                                                          \
            uint16_t ret;                                                            \
            asm volatile( "inw %%dx, %%ax" : "=a"( ret ) : "d"( addr ) : "memory" ); \
            ret;                                                                     \
        } )
// Read a double word from the specified address
# define ASM_read32( addr )                                                           \
        ( {                                                                           \
            uint32_t ret;                                                             \
            asm volatile( "inl %%dx, %%eax" : "=a"( ret ) : "d"( addr ) : "memory" ); \
            ret;                                                                      \
        } )

// Write a byte to the specified address
# define ASM_write8( addr, val ) \
        asm volatile( "outb %%al, %%dx" : : "d"( addr ), "a"( val ) : "memory" )
// Write a word to the specified address
# define ASM_write16( addr, val ) \
        asm volatile( "outw %%ax, %%dx" : : "d"( addr ), "a"( val ) : "memory" )
// Write a double word to the specified address
# define ASM_write32( addr, val ) \
        asm volatile( "outl %%eax, %%dx" : : "d"( addr ), "a"( val ) : "memory" )

// Print an info message
# define OS_INFO( ... ) printk( "INFO: " __VA_ARGS__ )
// Print a warning message
# define OS_WARN( ... ) printk( "WARN: " __VA_ARGS__ )
// Print an error message
# define OS_ERROR( ... ) printk( "ERROR: " __VA_ARGS__ )

# define ONE_SECOND_IN_USEC ( 1000000U )

// Sleep for the specified number of milliseconds
void __attribute__( ( weak ) ) sleep_ms( uint64_t ms )
{
    for ( uint64_t i = 0; i < ms; i++ )
    {
        for ( uint64_t j = 0; j < ONE_SECOND_IN_USEC; j++ )
        {
            asm volatile( "nop" : : : "memory" );
        }
    }
}

#endif /* UTIL_H */

/*** End of File ***/