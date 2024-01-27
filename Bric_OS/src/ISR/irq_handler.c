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

#include "gdt.h"
#include "idt.h"

/* Private Defines and Macros */

#define __interrupt __attribute__( ( interrupt ) )

#define NUM_IRQS ( 16U )

#define PIC1         ( 0x20U )  // IO base address for controller PIC
#define PIC2         ( 0xA0U )  // IO base address for peripheral PIC
#define PIC1_COMMAND ( PIC1 )
#define PIC1_DATA    ( PIC1 + 1 )
#define PIC2_COMMAND ( PIC2 )
#define PIC2_DATA    ( PIC2 + 1 )

#define PIC1_OFFSET ( 0x20U )  // Controller PIC interrupt offset
#define PIC2_OFFSET ( 0x28U )  // Peripheral PIC interrupt offset

#define PIC_EOI ( 0x20 )  // End of interrupt

#define ICW1_ICW4      ( 0x01U )  // Indicates that ICW4 will be present
#define ICW1_SINGLE    ( 0x02U )  // Single (cascade) mode
#define ICW1_INTERVAL4 ( 0x04U )  // Call address interval 4 (8)
#define ICW1_LEVEL     ( 0x08U )  // Level triggered (edge) mode
#define ICW1_INIT      ( 0x10U )  // Initialization - required!

#define ICW4_8086      ( 0x01U )  // 8086/88 (MCS-80/85) mode
#define ICW4_AUTO      ( 0x02U )  // Auto (normal) EOI
#define ICW4_BUF_PERIF ( 0x08U )  // Buffered mode/peripheral
#define ICW4_BUF_CTRLR ( 0x0CU )  // Buffered mode/controller
#define ICW4_SFNM      ( 0x10U )  // Special fully nested (not)

// Interrupt Descriptor Table Struct
typedef struct __packed
{
    void* arg;
    irq_handler_t handler;
} irq_table[NUM_IRQS];

/* Global Variables */

/* Private Functions */

/**
 * @brief Remap the PICs to the specified offsets
 * @param offset1 Vector offset for controller PIC
 * @param offset2 Vector offset for peripheral PIC
 */
void PIC_remap( int offset1, int offset2 )
{
    uint8_t a1, a2;

    a1 = inb( PIC1_DATA );  // save masks
    a2 = inb( PIC2_DATA );

    // Start the initialization sequence (in cascade mode)
    outb( PIC1_COMMAND, ICW1_INIT | ICW1_ICW4 );
    io_wait();
    outb( PIC2_COMMAND, ICW1_INIT | ICW1_ICW4 );
    io_wait();

    // Set the vector offsets
    outb( PIC1_DATA, offset1 );
    io_wait();
    outb( PIC2_DATA, offset2 );
    io_wait();

    // Inform controller of peripheral PIC at IRQ2, then tell the peripheral to cascade
    outb( PIC1_DATA, 4 );
    io_wait();
    outb( PIC2_DATA, 2 );
    io_wait();

    // Enable 8086 mode
    outb( PIC1_DATA, ICW4_8086 );
    io_wait();
    outb( PIC2_DATA, ICW4_8086 );
    io_wait();

    outb( PIC1_DATA, a1 );  // restore saved masks.
    outb( PIC2_DATA, a2 );
}

/**
 * @brief Disable the PICs
 */
void PIC_disable( void )
{
    outb( PIC1_DATA, 0xFF );
    outb( PIC2_DATA, 0xFF );
}

/**
 * @brief Send an EOI (End of Interrupt) to the PIC
 * @param irq IRQ to send the EOI to
 */
void PIC_sendEOI( uint8_t irq )
{
    if ( irq >= 8 ) outb( PIC2_COMMAND, PIC_EOI );

    outb( PIC1_COMMAND, PIC_EOI );
}

void interrupt_handler( int irq, int error, void* arg )
{
    // TODO: implement arg
    // TODO: implement error

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
    gdt_init();

    // Initialize the IDT
    idt_init();

    // Remap the PICs to the specified offsets
    PIC_remap( PIC1_OFFSET, PIC2_OFFSET );

    // Set the interrupt handlers
    for ( uint8_t i = 0; i < NUM_IRQS; i++ )
    {
        IRQ_set_handler( i, interrupt_handler, NULL );
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

#define PRESENT_FLAG        ( 0b10000000 )  // 0x80
#define INTERRUPT_GATE_FLAG ( 0b00001110 )  // 0x0E

void IRQ_set_handler( int irq, irq_handler_t handler, void* arg )
{
    // TODO: implement arg
    (void)arg;

    // TODO: implement IST
    idt_set_descriptor( irq, handler, ( PRESENT_FLAG | INTERRUPT_GATE_FLAG ), 0 );

    IRQ_clear_mask( irq );
}

/*** End of File ***/