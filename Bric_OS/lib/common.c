/** @file common.c
 *
 * @brief A description of the module's purpose.
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

#pragma region Wait Functions

void io_wait( void ) { outb( 0x80, 0 ); }

void io_wait_n( uint32_t t )
{
    while ( t-- ) io_wait();
}

#pragma endregion

#pragma region Port I/O Functions

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

#pragma region Interrupt Functions

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

/*** End of File ***/