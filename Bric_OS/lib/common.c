/** @file common.c
 *
 * @brief Commonly used functions.
 *
 * @author Bryce Melander
 * @date Jan-25-2024
 *
 * @copyright (c) 2024 by Bryce Melander under MIT License. All rights reserved.
 * (See http://opensource.org/licenses/MIT for more details.)
 */

#include "common.h"

/* Private Defines and Macros */

/* Global Variables */

/* Private Functions */

/* Public Functions */

#pragma region Wait

void io_wait( void ) { outb( 0x80, 0 ); }

void io_wait_n( uint64_t t )
{
    while ( t-- ) io_wait();
}

#pragma endregion

#pragma region Port I/O

uint8_t inb( uint16_t port )  // NOLINT
{
    uint8_t ret;
    asm volatile( "inb %1, %0" : "=a"( ret ) : "Nd"( port ) : "memory" );
    return ret;
}

uint16_t inw( uint16_t port )  // NOLINT
{
    uint16_t ret;
    asm volatile( "inw %1, %0" : "=a"( ret ) : "Nd"( port ) : "memory" );
    return ret;
}

uint32_t inl( uint16_t port )  // NOLINT
{
    uint32_t ret;
    asm volatile( "inl %1, %0" : "=a"( ret ) : "Nd"( port ) : "memory" );
    return ret;
}

void outb( uint16_t port, uint8_t val )  // NOLINT
{
    asm volatile( "outb %%al, %%dx" : : "d"( port ), "a"( val ) : "memory" );
}

void outw( uint16_t port, uint16_t val )  // NOLINT
{
    asm volatile( "outw %%ax, %%dx" : : "d"( port ), "a"( val ) : "memory" );
}

void outl( uint16_t port, uint32_t val )  // NOLINT
{
    asm volatile( "outl %%eax, %%dx" : : "d"( port ), "a"( val ) : "memory" );
}

#pragma endregion

#pragma region Interrupts

bool are_interrupts_enabled()
{
    unsigned long flags;

    asm volatile(
        "pushf\n\t"
        "pop %0"
        : "=g"( flags )
    );

    return flags & ( 1 << 9 );
}

unsigned long save_irqdisable( void )
{
    unsigned long flags;
    asm volatile( "pushf\n\tcli\n\tpop %0" : "=r"( flags ) : : "memory" );
    return flags;
}

void irqrestore( unsigned long flags )
{
    asm( "push %0\n\tpopf" : : "rm"( flags ) : "memory", "cc" );
}

// void intended_usage( void )
// {
//     unsigned long f = save_irqdisable();
//     do_whatever_without_irqs();
//     irqrestore( f );
// }

#pragma endregion

#pragma region Atomic Operations

int atomic_test_and_set( int* value, int compare, int swap )  // NOLINT
{
    int ret;
    asm volatile( "lock cmpxchg %2, %1"
                  : "=a"( ret ), "+m"( *value )
                  : "r"( swap ), "0"( compare )
                  : "memory" );
    return ret;
}

// Atomic swap: swap the values of two variables atomically
// int atomic_swap( int* valA, int* valB )  // NOLINT
//{
//    return __sync_val
//}

#pragma endregion

#pragma region Binary Semaphore

// void binary_semaphore_lock( binary_semaphore_t* sem )
//{
//     while ( atomic_test_and_set( (int*)&sem->locked, false, true ) )
//     {
//         io_wait();
//     }
// }

// void binary_semaphore_unlock( binary_semaphore_t* sem ) { sem->locked = false; }

#pragma endregion

/*** End of File ***/