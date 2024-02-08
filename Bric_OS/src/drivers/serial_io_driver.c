/** @file serial_io_driver.c
 *
 * @brief Serial I/O driver implementation.
 *
 * @author Bryce Melander
 * @date Feb-01-2024
 *
 * @copyright (c) 2024 by Bryce Melander under MIT License. All rights reserved.
 * (See http://opensource.org/licenses/MIT for more details.)
 */

#include "serial_io_driver.h"

/* Private Includes */

#include "irq_handler.h"

/* Private Defines and Macros */
#define COM_PORT ( 1U )  // COM1

#define BAUD_RATE                   ( 115200U )  // 115200, 57600, 38400, etc.
#define BAUD_RATE_DIVISOR           ( (uint16_t)( 115200U / BAUD_RATE ) )  // ( 115200 / 115200 ) = 1
#define BAUD_RATE_DIVISOR_LOW_BYTE  ( BAUD_RATE_DIVISOR & 0xFF )
#define BAUD_RATE_DIVISOR_HIGH_BYTE ( ( BAUD_RATE_DIVISOR >> 8 ) & 0xFF )

#define LINE_DLAB_ENABLE ( 0b10000000U )  // Enable DLAB

#define LINE_8_DATA_BITS ( 0b11U )        // 8 data bits
#define LINE_1_STOP_BIT  ( 0b0U << 2 )    // 1 stop bit
#define LINE_NO_PARITY   ( 0b000U << 3 )  // No parity
#define LINE_INIT        ( LINE_8_DATA_BITS | LINE_1_STOP_BIT | LINE_NO_PARITY )

#define FIFO_ENABLE           ( 0b1U << 0 )   // Enable FIFO
#define FIFO_CLEAR_TX_RX      ( 0b11U << 1 )  // Clear TX and RX FIFOs
#define FIFO_1_BYTE_THRESHOLD ( 0b00U << 6 )  // 1-byte threshold
#define FIFO_INIT             ( FIFO_ENABLE | FIFO_CLEAR_TX_RX | FIFO_1_BYTE_THRESHOLD )

#define MODEM_DTR_ENABLE       ( 0b1U << 0 )   // Enable Data Terminal Ready (DTR)
#define MODEM_RTS_ENABLE       ( 0b1U << 1 )   // Enable Request To Send (RTS)
#define MODEM_OUT1_OUT2_ENABLE ( 0b11U << 2 )  // Enable OUT#1 and OUT#2
#define MODEM_INIT             ( MODEM_DTR_ENABLE | MODEM_RTS_ENABLE | MODEM_OUT1_OUT2_ENABLE )

#define MODEM_SELF_TEST ( ( 0b1U << 4 ) | MODEM_INIT )  // Enable loopback mode
#define MODEM_TEST_BYTE ( 0xAEU )  // This is just a random byte I promise it is not special

#define IER_ENABLE_RX_FULL  ( 0b1U << 0 )  // Enable RX buffer full interrupt
#define IER_ENABLE_TX_EMPTY ( 0b1U << 1 )  // Enable TX buffer empty interrupt
#define IER_ENABLE_LINE_ERR ( 0b1U << 2 )  // Enable line status interrupt
#define IER_ENABLE_STATUS   ( 0b1U << 3 )  // Enable modem status interrupt
#define IER_INIT            ( IER_ENABLE_TX_EMPTY | IER_ENABLE_LINE_ERR | IER_ENABLE_STATUS )

#define IS_EMPTY( fifo ) ( ( fifo )->count == 0 )
#define IS_FULL( fifo )  ( ( fifo )->count == SERIAL_BUFF_SIZE )

#define IIR_IRQ_MASK       ( 0b111U << 1 )
#define IIR_TX_IRQ         ( 0b01U << 1 )
#define IIR_LINE_IRQ       ( 0b11U << 1 )
#define IS_TX_IRQ( iir )   ( ( ( iir ) & IIR_IRQ_MASK ) == IIR_TX_IRQ )
#define IS_LINE_IRQ( iir ) ( ( ( iir ) & IIR_IRQ_MASK ) == IIR_LINE_IRQ )
#define IS_HW_TX_EMPTY()   ( ( inb( SERIAL_PORT + 5 ) & 0x20 ) == 0x20 )

#define SERIAL_BUFF_SIZE ( 0x10U )

#if ( COM_PORT == 1 )
# define SERIAL_PORT ( (uint16_t)0x3F8 )  // COM1
# define SERIAL_IRQ  ( IRQ36_COM1 )
#elif ( COM_PORT == 2 )
# define SERIAL_PORT ( (uint16_t)0x2F8 )  // COM2
# define SERIAL_IRQ  ( IRQ35_COM2 )
#elif ( COM_PORT == 3 )
# define SERIAL_PORT ( (uint16_t)0x3E8 )  // COM3
# define SERIAL_IRQ  ( IRQ36_COM1 )       // Shared with COM1
#elif ( COM_PORT == 4 )
# define SERIAL_PORT ( (uint16_t)0x2E8 )  // COM4
# define SERIAL_IRQ  ( IRQ35_COM2 )       // Shared with COM2
#else
# error "Invalid COM port number!"
#endif

#define IS_BUFF_EMPTY() ( serial_tx->count == 0 )

/* Private Types and Enums */

// Circular FIFO buffer
typedef struct
{
    uint8_t buff[SERIAL_BUFF_SIZE];
    uint8_t *prod;
    uint8_t *cons;
    uint32_t count;
    bool idle_flag;
} serial_fifo_t;

/* Global Variables */

static serial_fifo_t _serial_tx_obj;
static serial_fifo_t *serial_tx = &( _serial_tx_obj );

/* Private Functions */

#define tx_next() hw_write()
#define tx_stop() ( serial_tx )->idle_flag = true

int cons_get( serial_fifo_t *stream, uint8_t *d );

void hw_write( void )
{
    uint8_t data;

    if ( serial_tx->idle_flag )
    {
        // Clear the idle flag
        serial_tx->idle_flag = false;

        // Read the next byte from the TX buffer
        if ( cons_get( serial_tx, &data ) == 0 )
        {
            // Write the data to the TX buffer
            outb( SERIAL_PORT, data );
        }
    }
    else
    {
        // Check if the HW TX buffer is empty
        if ( IS_HW_TX_EMPTY() )
        {
            // Set the idle flag
            serial_tx->idle_flag = true;
        }
    }
}

void serial_fifo_init( serial_fifo_t *fifo )
{
    // Error checking
    if ( fifo == NULL )
    {
        return;
    }

    // Clear the buffer
    memset( fifo->buff, 0, SERIAL_BUFF_SIZE );

    // Initialize the buffer input and output pointers
    fifo->prod = &fifo->buff[0];
    fifo->cons = &fifo->buff[0];

    // Set the idle flag
    fifo->idle_flag = true;

    // Set the count to 0
    fifo->count = 0;
}

/**
 * @brief To be used by a `consumer` to read the next byte from a standard IO stream. Note: This
 * call should be made from within an ISR that has been configured with an interrupt gate.
 * @param stream The stream to be read from.
 * @param d A pointer to where the byte should be stored.
 * @return 0 on success, -1 if there is no data available.
 */
int cons_get( serial_fifo_t *stream, uint8_t *d )
{
    // Error checking
    if ( stream == NULL || d == NULL )
    {
        return -1;
    }

    // Check if there is data to read
    if ( IS_EMPTY( stream ) )
    {
        return -1;
    }

    // Disable interrupts
    IRQ_disable();

    // Read the data
    *d = *stream->cons++;

    // Decrement the count
    --stream->count;

    // Wrap around
    if ( stream->cons >= &stream->buff[SERIAL_BUFF_SIZE] )
    {
        stream->cons = &stream->buff[0];
    }

    // Check if the buffer is empty
    if ( IS_EMPTY( stream ) )
    {
        tx_stop();
    }

    // Restore interrupts
    IRQ_reenable();

    return 0;
}

/**
 * @brief To be used by a `producer` to write a byte to the next available space in a standard IO
 * stream.
 * @param stream The stream to be written to.
 * @param d The byte to be written.
 * @return 0 on success, -1 if there is no space available.
 */
int prod_add( serial_fifo_t *stream, uint8_t d )
{
    // Error checking
    if ( stream == NULL )
    {
        return -1;
    }

    int ret = 0;

    // Disable interrupts
    IRQ_disable();

    // Check if there is space to write data
    if ( !( IS_FULL( stream ) ) )
    {
        // Write the data
        *stream->prod++ = d;

        // Increment the count
        ++stream->count;

        // Wrap around
        if ( stream->prod >= ( &stream->buff[SERIAL_BUFF_SIZE] ) )
        {
            stream->prod = &stream->buff[0];
        }

        // If this was the first byte written, initiate a HW write
        if ( serial_tx->count == 1 )
        {
            tx_next();
        }
    }
    else
    {
        ret = -1;
    }

    // Restore interrupts
    IRQ_reenable();

    return ret;
}

void serial_tx_irq_handler( int __unused irq, int __unused error, void __unused *arg )
{
    // `serial_tx_irq_handler` is the consumer

    // DEBUG: Print a message to the screen
    OS_INFO( "Serial TX IRQ!\n" );

    // Read the IIR to determine the cause of the interrupt
    uint8_t iir = inb( SERIAL_PORT + 2 );

    // Check if the interrupt was caused by the TX buffer being empty
    if ( IS_TX_IRQ( iir ) )
    {
        // Clear the interrupt by reading from the IIR
        inb( SERIAL_PORT );

        // Set the idle flag
        serial_tx->idle_flag = true;

        // Iniate a HW write
        hw_write();
    }
    // If its a LINE interrupt, read the LSR to clear it
    else if ( IS_LINE_IRQ( iir ) )
    {
        inb( SERIAL_PORT + 5 );
    }
    // Unknown interrupt
    else
    {
        // DEBUG: Print a message to the screen
        OS_INFO( "Serial TX IRQ: Unknown interrupt, IIR: 0x%X\n", iir );
    }
}

/* Public Functions */

driver_status_t serial_driver_init( void )
{
    // Initialize the TX buffer
    serial_fifo_init( serial_tx );

    // Disable all interrupts
    outb( SERIAL_PORT + 1, 0x00 );

    // Enable DLAB
    outb( SERIAL_PORT + 3, LINE_DLAB_ENABLE );

    // Set the baud rate to ( 115200 / 1 ) = 115200 bps
    outb( SERIAL_PORT + 0, BAUD_RATE_DIVISOR_LOW_BYTE );
    outb( SERIAL_PORT + 1, BAUD_RATE_DIVISOR_HIGH_BYTE );

    // 8 bits, no parity, one stop bit
    outb( SERIAL_PORT + 3, LINE_INIT );

    // Enable FIFO, clear them, trigger at a 1-byte threshold
    outb( SERIAL_PORT + 2, FIFO_INIT );

    // Enable loopback mode to test the serial chip
    outb( SERIAL_PORT + 4, MODEM_SELF_TEST );
    // Send byte `0xAE` to check if serial returns same byte
    outb( SERIAL_PORT + 0, MODEM_TEST_BYTE );

    // Serial is faulty if the byte received is not the same as the byte sent
    if ( inb( SERIAL_PORT + 0 ) != MODEM_TEST_BYTE )
    {
        OS_ERROR( "Serial COM%u is faulty, did not pass self test!\n", COM_PORT );
        return FAILURE;
    }

    // If serial is not faulty, setup normal operation mode (Enables DTR, RTS, OUT#1, and OUT#2).
    outb( SERIAL_PORT + 4, MODEM_INIT );

    // Enable the TX and Line status IRQs
    outb( SERIAL_PORT + 1, IER_INIT );

    // Install the IRQ handler
    if ( IRQ_set_handler( SERIAL_IRQ, serial_tx_irq_handler, NULL ) != 0 )
    {
        OS_ERROR( "Failed to install the serial TX IRQ handler!\n" );
        return FAILURE;
    }

    // Enable the IRQ
    IRQ_clear_mask( IRQ36_COM1 );

    return SUCCESS;
}

/**
 * @brief Read data from the serial port.
 * @param buff - A pointer to the buffer where the data should be stored.
 * @param len - The number of bytes to read.
 * @return The number of bytes written to the buffer.
 */
size_t serial_write( const char *buff, size_t len )
{
    // `serial_write` is the producer
    size_t i;

    // Error checking
    if ( buff == NULL )
    {
        return 0;
    }

    // If len is negative, it will double as an error code :)
    if ( len < 1 )
    {
        return len;
    }

    // Write the data in `buff` to the serial TX FIFO buffer
    for ( i = 0; i < len; ++i )
    {
        // Try to add the byte to the buffer
        if ( prod_add( serial_tx, buff[i] ) )
        {
            // Stop writing to the buffer
            break;
        }
    }

    return i;
}

/**
 * @brief Write a string to the serial port.
 * @param str - A pointer to the string to be written.
 */
void serial_print( const char *str )
{
    // Error checking
    if ( str == NULL )
    {
        return;
    }

    // Write the string to the serial port
    serial_write( str, strlen( str ) );
}

/*** End of File ***/