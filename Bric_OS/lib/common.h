/** @file util.h
 *
 * @brief Commonly used macros and definitions.
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

# define NO_CHAR ( -1 )

/* Macros */

// Print an info message
# define OS_INFO( ... ) printk( "INFO: " __VA_ARGS__ )
// Print a warning message
# define OS_WARN( ... ) printk( "WARN: " __VA_ARGS__ )
// Print an error message
# define OS_ERROR( ... ) printk( "ERROR: " __VA_ARGS__ )

// Print an error message and halt the CPU
# define OS_ERROR_HALT( ... )                                                     \
        do                                                                        \
        {                                                                         \
            printk( "\n" );                                                       \
            OS_ERROR( __VA_ARGS__ );                                              \
            OS_ERROR( "This error has occurred at %s:%d\n", __FILE__, __LINE__ ); \
            OS_ERROR( "The system will now halt.\n" );                            \
            HLT();                                                                \
        } while ( 0 )

// Halt the CPU
# define HLT() \
        while ( 1 ) asm volatile( "hlt" )

// Align x to the next multiple of n
# define ALIGN( x, n ) ( ( ( (x)-1 ) | ( (n)-1 ) ) + 1 )

/* Typedefs */

typedef enum { SUCCESS = 0, FAILURE = 1 } driver_status_t;

typedef unsigned int uint;

/* Public Functions */

# pragma region Wait

// Wait a very small amount of time (1 to 4 microseconds, generally). Useful as a simple but
// imprecise wait.
void io_wait( void );

// Wait for a specified number of io_wait() calls to complete
void io_wait_n( uint64_t t );

# pragma endregion

# pragma region Port I/O

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

# pragma region Interrupts

// Disable interrupts
unsigned long save_irqdisable( void );

// Restore interrupts
void irqrestore( unsigned long flags );

# pragma endregion

# pragma region Atomic Operations

// Atomically compare and swap a value
int atomic_test_and_set( int* value, int compare, int swap );

// Atomically swap two values
int atomic_swap( int* valA, int* valB );

# pragma endregion

# pragma region Binary Semaphore

// Binary semaphore type
typedef struct _binary_semaphore_s
{
    int locked;
} binary_semaphore_t;

// Lock a binary semaphore
// void binary_semaphore_lock( binary_semaphore_t* sem );

# define binary_semaphore_lock( sem )                             \
        while ( atomic_test_and_set( &sem.locked, false, true ) ) \
        {                                                         \
            io_wait();                                            \
        }

// Unlock a binary semaphore
// void binary_semaphore_unlock( binary_semaphore_t* sem );

# define binary_semaphore_unlock( sem ) ( sem ).locked = false

#endif /* UTIL_H */

/*** End of File ***/