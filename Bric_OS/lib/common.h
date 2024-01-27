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
# include <stdbool.h>
# include <stddef.h>
# include <stdint.h>
# include <string.h>

# include "printk.h"

/* Defines */

# define __unused        __attribute__( ( unused ) )
# define __packed        __attribute__( ( packed ) )
# define __noreturn      __attribute__( ( noreturn ) )
# define __weak          __attribute__( ( weak ) )
# define __alligned( x ) __attribute__( ( aligned( x ) ) )

/* Macros */

// Print an info message
# define OS_INFO( ... ) printk( "INFO: " __VA_ARGS__ )
// Print a warning message
# define OS_WARN( ... ) printk( "WARN: " __VA_ARGS__ )
// Print an error message
# define OS_ERROR( ... ) printk( "ERROR: " __VA_ARGS__ )

/* Typedefs */

typedef enum { SUCCESS = 0, FAILURE = 1 } driver_status_t;

/* Public Functions */

# pragma region Wait Functions

// Wait a very small amount of time (1 to 4 microseconds, generally). Useful as a simple but
// imprecise wait.
void io_wait( void );

// Wait for a specified number of io_wait() calls to complete
void io_wait_n( uint64_t t );

# pragma endregion

# pragma region Port I/O Functions

// Read a byte from a port
uint8_t inb( uint16_t port );

// Read a word from a port
uint16_t inw( uint16_t port );

// Read a double word from a port
uint32_t inl( uint16_t port );

// Write a byte to a port
void outb( uint16_t port, uint8_t val );

// Write a word to a port
void outw( uint16_t port, uint16_t val );

// Write a double word to a port
void outl( uint16_t port, uint32_t val );

# pragma endregion

# pragma region Interrupt Functions

// Disable interrupts
unsigned long save_irqdisable( void );

// Restore interrupts
void irqrestore( unsigned long flags );

# pragma endregion

#endif /* UTIL_H */

/*** End of File ***/