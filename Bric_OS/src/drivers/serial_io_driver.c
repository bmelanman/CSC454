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

#include "common.h"
#include "isr_common.h"

/* Private Defines and Macros */

#define SERIAL_BUFF_SIZE ( 0x100U )

#define IS_EMPTY( fifo ) ( ( fifo )->prod == ( fifo )->cons )
#define IS_FULL( fifo )                                                          \
    ( ( fifo )->prod == ( fifo )->cons - 1 ||   /* prod is 1 idex behind cons */ \
      ( ( fifo )->cons == &( fifo )->buff[0] && /* cons is at the start */       \
        ( fifo )->prod == &( fifo )->buff[SERIAL_BUFF_SIZE - 1] ) ) /* prod is at the end */

/* Private Types and Enums */

// #define SERIAL_BUFF_SIZE_ACTUAL ( SERIAL_BUFF_SIZE + 1 )

// Circular FIFO buffer
typedef struct
{
    uint8_t buff[SERIAL_BUFF_SIZE];
    uint8_t *prod;
    uint8_t *cons;
    bool idle_flag;
} serial_fifo_t;

/* Global Variables */

static serial_fifo_t serial_tx;

static serial_fifo_t *serial_tx_ptr = &serial_tx;

/* Private Functions */

void serial_fifo_init( serial_fifo_t *fifo )
{
    memset( fifo->buff, 0, SERIAL_BUFF_SIZE );
    fifo->prod = &fifo->buff[0];
    fifo->cons = &fifo->buff[0];
    fifo->idle_flag = true;
}

// TODO: Implement these
// extern void tx_next( void );
// extern void tx_stop( void );

/**
 * @brief To be used by a `consumer` to read the next byte from a standard IO stream. Note: This
 * call should be made from within an ISR that has been configured with an interrupt gate.
 * @param stream The stream to be read from.
 * @param d A pointer to where the byte should be stored.
 * @return 0 on success, 1 if there is no data available.
 */
int cons_get( serial_fifo_t *stream, uint8_t *d )
{
    // Error checking
    if ( stream == NULL || d == NULL )
    {
        return 1;
    }

    // Check if there is data to read
    if ( IS_EMPTY( stream ) )
    {
        return 1;
    }

    // Write the data to the given address
    outb( *stream->cons++, (uintptr_t)d );

    // Wrap around
    if ( stream->cons >= &stream->buff[SERIAL_BUFF_SIZE] )
    {
        stream->cons = &stream->buff[0];
    }

    // if ( IS_EMPTY( stream ) )
    //{
    //     tx_stop();
    // }

    return 0;
}

/**
 * @brief To be used by a `producer` to write a byte to the next available space in a standard IO
 * stream.
 * @param stream The stream to be written to.
 * @param d The byte to be written.
 * @return 0 on success, 1 if there is no space available.
 */
int prod_add( serial_fifo_t *stream, uint8_t d )
{
    // Error checking
    if ( stream == NULL )
    {
        return 1;
    }

    // Disable interrupts
    CLI();

    // Check if there is space to write data
    if ( IS_FULL( stream ) )
    {
        // Enable interrupts
        STI();

        return 1;
    }

    // Write the data
    *stream->prod++ = d;

    // if ( stream->idle_flag )
    //{
    //     tx_next();
    // }

    // Wrap around
    if ( stream->prod >= ( &stream->buff[SERIAL_BUFF_SIZE] ) )
    {
        stream->prod = &stream->buff[0];
    }

    // Enable interrupts
    STI();

    return 0;
}

/* Public Functions */

void serial_init( void )
{
    // Initialize the TX buffer
    serial_fifo_init( serial_tx_ptr );
}

int serial_write( const char *buff, int len )
{
    int i;

    // Error checking
    if ( buff == NULL )
    {
        return 1;
    }
    if ( len < 1 )
    {
        // If len is negative, it will double as an error code :)
        return len;
    }

    // Write the data in `buff` to the serial TX FIFO buffer
    for ( i = 0; i < len; ++i )
    {
        prod_add( serial_tx_ptr, buff[i] );
    }

    return 0;
}

/*** End of File ***/