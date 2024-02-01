/** @file pic.c
 *
 * @brief Programmable Interrupt Controller (PIC) implementation.
 *
 * @author Bryce Melander
 * @date Jan-28-2024
 *
 * @copyright (c) 2024 by Bryce Melander under MIT License. All rights reserved.
 * (See http://opensource.org/licenses/MIT for more details.)
 */

#include "pic.h"

/* Includes */

#include "common.h"

/* Private Defines and Macros */

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

/* Global Variables */

/* Private Functions */

/* Public Functions */

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
void PIC_sendEOI( unsigned int irq )
{
    if ( irq >= 8 )
    {
        outb( PIC2_COMMAND, PIC_EOI );
    }

    outb( PIC1_COMMAND, PIC_EOI );
}

/*** End of File ***/