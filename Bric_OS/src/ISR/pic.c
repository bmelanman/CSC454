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

#define PIC_ENABLE_CASCADE ( 0x02U )  // Enable the cascade mode
#define PIC_ENABLE_IRQ2    ( 0x04U )  // Enable IRQ2 (cascade)
#define PIC_EOI            ( 0x20U )  // End of interrupt
#define PIC_DISABLE        ( 0xFFU )  // Disable the PIC

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

#define IRQ_RANGE_ERROR( irq ) OS_ERROR( "Invalid IRQ: %d, out of PIC range!\n", irq )

/* Global Variables */

/* Private Functions */

/* Public Functions */

void PIC_init( void )
{
    // Remap the PICs to the specified offsets
    PIC_remap( PIC1_OFFSET, PIC2_OFFSET );

    // Mask all PIC controlled IRQs
    for ( uint i = PIC1_MIN_IRQ; i < PIC2_MAX_IRQ; i++ )
    {
        PIC_set_mask( i );
    }
}

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
    outb( PIC1_DATA, PIC_ENABLE_IRQ2 );
    io_wait();
    outb( PIC2_DATA, PIC_ENABLE_CASCADE );
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
 * @brief Set the mask for the specified IRQ.
 * @param irq IRQ to set the mask for.
 * @return 0 on success, -1 on failure.
 */
int PIC_set_mask( uint16_t irq )
{
    uint16_t pic_data_port;
    uint8_t mask;

    // Make sure the IRQ is valid
    if ( !IS_PIC_IRQ( irq ) )
    {
        IRQ_RANGE_ERROR( irq );
        return -1;
    }

    irq -= PIC1_OFFSET;

    if ( irq < PIC1_MIN_IRQ )
    {
        pic_data_port = PIC1_DATA;
    }
    else
    {
        pic_data_port = PIC2_DATA;
        irq -= 8;
    }

    // Set the appropriate bit in the current mask
    mask = inb( pic_data_port ) | ( 1 << irq );
    // Write the new mask back to the PIC
    outb( pic_data_port, mask );

    return 0;
}

/**
 * @brief Clear the mask for the specified IRQ.
 * @param irq IRQ to clear the mask for.
 * @return 0 on success, -1 on failure.
 */
int PIC_clear_mask( uint16_t irq )
{
    uint16_t pic_data_port;
    uint8_t mask;

    // Make sure the IRQ is valid
    if ( !IS_PIC_IRQ( irq ) )
    {
        IRQ_RANGE_ERROR( irq );
        return -1;
    }

    irq -= PIC1_OFFSET;

    if ( irq < PIC1_MIN_IRQ )
    {
        pic_data_port = PIC1_DATA;
    }
    else
    {
        pic_data_port = PIC2_DATA;
        irq -= 8;
    }

    // Clear the appropriate bit in the current mask
    mask = inb( pic_data_port ) & ~( 1 << irq );
    // Write the new mask back to the PIC
    outb( pic_data_port, mask );

    return 0;
}

/**
 * @brief Get the mask for the specified IRQ.
 * @param irq IRQ to get the mask for.
 * @return Mask for the specified IRQ.
 */
int PIC_get_mask( uint16_t irq )
{
    uint16_t pic_data_port;
    uint8_t mask;

    // Make sure the IRQ is valid
    if ( !IS_PIC_IRQ( irq ) )
    {
        IRQ_RANGE_ERROR( irq );
        return -1;
    }

    // Account for the offset since we are using the PIC offsets
    irq -= PIC1_OFFSET;

    if ( irq < PIC1_MIN_IRQ )
    {
        pic_data_port = PIC1_DATA;
    }
    else
    {
        pic_data_port = PIC2_DATA;
        irq -= 8;
    }

    // Read the current mask from the PIC
    mask = inb( pic_data_port ) & ( 1 << irq );

    // Return weather or not the IRQ is masked
    return ( mask > 0 );
}

/**
 * @brief Disable the PICs
 */
void PIC_disable( void )
{
    outb( PIC1_DATA, PIC_DISABLE );
    outb( PIC2_DATA, PIC_DISABLE );
}

/**
 * @brief Send an EOI (End of Interrupt) to the PIC
 * @param irq IRQ to send the EOI to
 */
void PIC_send_EOI( unsigned int irq )
{
    // Ensure the IRQ is within the valid range
    if ( irq < PIC1_MIN_IRQ || irq > PIC2_MAX_IRQ )
    {
        OS_ERROR( "Cannot send EOI for IRQ %d", irq );
        return;
    }

    // Send an EOI to the second PIC only if necessary
    if ( irq > PIC1_MAX_IRQ )
    {
        outb( PIC2_COMMAND, PIC_EOI );
    }

    // Always send an EOI to the first PIC
    outb( PIC1_COMMAND, PIC_EOI );
}

/*** End of File ***/