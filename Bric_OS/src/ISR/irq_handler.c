/** @file irq_handler.c
 *
 * @brief A description of the module's purpose.
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

#define NUM_IRQS ( 16U )

#define PIC1_OFFSET ( 0x20U )  // Controller PIC interrupt offset
#define PIC2_OFFSET ( 0x28U )  // Peripheral PIC interrupt offset

// Interrupt Descriptor Table Struct
typedef struct __packed
{
    void* arg;
    irq_handler_t handler;
} irq_table[NUM_IRQS];

/* Global Variables */

/* Private Functions */

__noreturn void exception_handler( void )
{
    // Try to print the exception
    VGA_display_str( "Exception!\n" );

    // Cause a hang
    asm volatile(
        "cli\n\t"
        "hlt"
    );

    // Should never get here
    while ( 1 )
        ;
}

void interrupt_handler( int irq, int error, void* arg )
{
    // TODO: implement arg
    // TODO: implement error

    asm volatile( "nop" );
    asm volatile( "nop" );
    asm volatile( "nop" );

    OS_INFO( "Interrupt %d was called! \n", irq );
    OS_INFO( "  Error:  %d             \n", error );
    OS_INFO( "  Arg:    %p             \n", arg );

    PIC_sendEOI( irq );
}

/* Public Functions */

driver_status_t IRQ_init( void )
{
    // Disable interrupts
    CLI();

    // Initialize the GDT
    // gdt_init();

    // Initialize the IDT
    idt_init();

    // Remap the PICs to the specified offsets
    PIC_remap( PIC1_OFFSET, PIC2_OFFSET );

    // Mask all interrupts
    for ( uint8_t i = 0; i < NUM_IRQS; i++ )
    {
        IRQ_set_mask( i );
    }

    // Enable interrupts
    STI();

    return SUCCESS;
}

void IRQ_set_mask( int irq )
{
    uint16_t port;
    uint8_t value;

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
    (void)IRQline;
    return 0;
}

void IRQ_end_of_interrupt( int irq ) { PIC_sendEOI( irq ); }

void IRQ_set_handler( uint16_t irq, irq_handler_t handler, void* arg )
{
    // Make sure the IRQ is valid
    if ( irq >= NUM_IRQS )
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

    // TODO: implement arg
    (void)arg;

    idt_set_descriptor( irq, handler, PRESENT_INTERRUPT_GATE );

    IRQ_clear_mask( irq );
}

/*** End of File ***/