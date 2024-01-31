/** @file stdio.c
 *
 * @brief A description of the module's purpose.
 *
 * @author Bryce Melander
 * @date Jan-30-2024
 *
 * @copyright (c) 2024 by Bryce Melander under MIT License. All rights reserved.
 * (See http://opensource.org/licenses/MIT for more details.)
 */

#include "stdio.h"

/* Includes */

#include "ISR/isr_common.h"
#include "common.h"

/* Private Defines and Macros */

#define STDIO_BUFF_SIZE ( 0x100U )

#define IS_EMPTY( fifo ) ( ( fifo )->prod == ( fifo )->cons )
#define IS_FULL( fifo )                                                          \
    ( ( fifo )->prod == ( fifo )->cons - 1 ||   /* prod is 1 idex behind cons */ \
      ( ( fifo )->cons == &( fifo )->buff[0] && /* cons is at the start */       \
        ( fifo )->prod == &( fifo )->buff[STDIO_BUFF_SIZE - 1] ) ) /* prod is at the end */

/* Private Types and Enums */

typedef uint8_t data_t;

// Circular FIFO buffer
#define STDIO_BUFF_SIZE_ACTUAL ( STDIO_BUFF_SIZE + 1 )
typedef struct
{
    data_t buff[STDIO_BUFF_SIZE_ACTUAL];
    data_t *prod;
    data_t *cons;
    bool idle_flag;
} serial_fifo_t;

/* Global Variables */

serial_fifo_t stdin, stdout, stderr;

/* Private Functions */

void serial_fifo_init( serial_fifo_t *fifo )
{
    fifo->prod = &fifo->buff[0];
    fifo->cons = &fifo->buff[0];
    fifo->idle_flag = true;
}

/* Public Functions */

/**
 * @brief Initialize the standard IO streams (stdin, stdout, stderr)
 */
void stdio_init( void )
{
    serial_fifo_init( &stdin );
    serial_fifo_init( &stdout );
    serial_fifo_init( &stderr );
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
int stdio_cons_get( serial_fifo_t *stream, data_t *d )
{
    // Check if there is data to read
    if ( IS_EMPTY( stream ) )
    {
        return 1;
    }

    // Write the data to the given address
    outb( *stream->cons++, (uintptr_t)d );

    // Wrap around
    if ( stream->cons >= &stream->buff[STDIO_BUFF_SIZE] )
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
int stdio_prod_add( serial_fifo_t *stream, data_t d )
{
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
    if ( stream->prod >= &stream->buff[STDIO_BUFF_SIZE] )
    {
        stream->prod = &stream->buff[0];
    }

    // Enable interrupts
    STI();

    return 0;
}

/*** End of File ***/