/** @file irq_handler.c
 *
 * @brief Interrupt Request (IRQ) handler implementation.
 *
 * @author Bryce Melander
 * @date Jan-25-2024
 *
 * @copyright (c) 2024 by Bryce Melander under MIT License. All rights reserved.
 * (See http://opensource.org/licenses/MIT for more details.)
 */

#include "irq_handler.h"

/* Includes */

#include "idt.h"
#include "pic.h"
#include "vga_driver.h"

/* Private Defines and Macros */

#define __interrupt __attribute__( ( interrupt ) )

#define PIC1_OFFSET ( 0x20U )  // Controller PIC interrupt offset
#define PIC2_OFFSET ( 0x28U )  // Peripheral PIC interrupt offset

// Interrupt Descriptor Table Struct
typedef struct
{
    irq_handler_t handler;
    void* arg;
} irq_handler_entry_t;

/* Global Variables */

irq_handler_entry_t irq_handler_table[IDT_MAX_DESCRIPTORS];

/* Private Functions */

__noreturn void exception_handler( int error )
{
    // Print error
    OS_ERROR( "Exception Occurred! Error: %d\n", error );

    // Halt
    HLT();

    // Should never get here
    while ( 1 )
        ;
}

void interrupt_handler( int irq, int error )
{
    if ( (uint)irq < IRQ_EXCEPTION_MAX )
    {
        // Call the exception handler
        exception_handler( error );
    }

    if ( (uint)irq < IDT_MAX_DESCRIPTORS && irq_handler_table[irq].handler != NULL )
    {
        // Call the IRQ handler
        irq_handler_table[irq].handler( irq, error, irq_handler_table[irq].arg );
    }
    else
    {
        // Unhandled interrupt error
        OS_ERROR( "Unhandled interrupt: %d\n", irq );

        // Halt
        HLT();
    }

    IRQ_end_of_interrupt( irq );
}

/* Public Functions */

driver_status_t IRQ_init( void )
{
    uint16_t i;

    // Disable interrupts
    CLI();

    // Initialize the IDT
    idt_init();

    // Remap the PICs to the specified offsets
    PIC_remap( PIC1_OFFSET, PIC2_OFFSET );

    // Mask all interrupts
    for ( i = 0; i < IDT_MAX_DESCRIPTORS; i++ )
    {
        IRQ_set_mask( i );
    }

    return SUCCESS;
}

void IRQ_set_mask( int irq )
{
    uint16_t port;
    uint8_t value;

    // Can't mask the first 32 interrupts
    if ( irq < 32 )
    {
        return;
    }

    irq -= 32;

    if ( irq < 8 )
    {
        port = PIC1_DATA;
    }
    else
    {
        port = PIC2_DATA;
        irq -= 8;
    }

    value = inb( port ) | ( 1 << irq );
    outb( port, value );
}

void IRQ_clear_mask( int irq )
{
    uint16_t port;
    uint8_t value;

    // Can't clear the first 32 interrupts
    if ( irq < 32 )
    {
        return;
    }

    irq -= 32;

    if ( irq < 8 )
    {
        port = PIC1_DATA;
    }
    else
    {
        port = PIC2_DATA;
        irq -= 8;
    }

    value = inb( port ) & ~( 1 << irq );
    outb( port, value );
}

int IRQ_get_mask( int IRQline )
{
    uint16_t port;
    uint8_t value;

    // Can't get the mask of the first 32 interrupts
    if ( IRQline < 32 )
    {
        return 0;
    }

    IRQline -= 32;

    if ( IRQline < 8 )
    {
        port = PIC1_DATA;
    }
    else
    {
        port = PIC2_DATA;
        IRQline -= 8;
    }

    value = inb( port ) & ( 1 << IRQline );
    return ( value > 0 );
}

void IRQ_end_of_interrupt( int irq ) { PIC_sendEOI( irq ); }

void IRQ_set_handler( uint16_t irq, irq_handler_t handler, void* arg )
{
    // Make sure the IRQ is valid
    if ( irq >= IDT_MAX_DESCRIPTORS )
    {
        OS_ERROR( "IRQ %d is invalid!\n", irq );
        return;
    }

    // Make sure the handler is valid
    if ( handler == NULL )
    {
        OS_ERROR( "IRQ %d handler is NULL!\n", irq );
        return;
    }

    // Set the handler and arg in the table
    irq_handler_table[irq].handler = handler;
    irq_handler_table[irq].arg = arg;
}

/*** End of File ***/