/** @file vga_driver.c
 *
 * @brief VGA driver implementation.
 *
 * @author Bryce Melander
 * @date Jan-08-2024
 *
 * @copyright (c) 2024 by Bryce Melander under MIT License. All rights reserved.
 * (See http://opensource.org/licenses/MIT for more details.)
 */

#include "vga_driver.h"

// TODO: Add support for arrow keys
// TODO: Add support for commands (i.e. ^C, ^D, etc.)

/* Includes */

#include "irq_handler.h"

/* Private Defines and Macros */

#define VGA_BUFFER ( (uint16_t *)0xB8000 )

#define VGA_DATA_PORT_A ( 0x3D4U )
#define VGA_DATA_PORT_B ( 0x3D5U )

#define CONVERT_POS( row, col )     ( ( ( row ) * VGA_NUM_COLS ) + ( col ) )
#define CONVERT_CH_ATTR( ch, attr ) ( (uint16_t)( ( (uint8_t)( attr ) << 8 ) | (uint8_t)( ch ) ) )
#define VGA_PUTC( ch, attrs )                                             \
    ( VGA_BUFFER[CONVERT_POS( vga_cursor.pos.row, vga_cursor.pos.col )] = \
          CONVERT_CH_ATTR( ( ch ), ( attrs ) ) )
#define VGA_PUTC_DEFAULT( ch ) VGA_PUTC( ( ch ), VGA_CHAR_DEFAULT_ATTR )
#define VGA_CLEAR_CHAR()       VGA_PUTC( 0, VGA_CHAR_DEFAULT_ATTR )

#define TAB_LENGTH ( 4U )

/* Private Typedefs */

typedef struct
{
    uint8_t ch;    // Character
    uint8_t attr;  // Attributes
} vga_char_t;

typedef struct
{
    uint8_t row;  // Row index
    uint8_t col;  // Column index
} vga_pos_t;

typedef struct
{
    vga_pos_t pos;        // Row and column indexes
    vga_char_t vga_char;  // Character and attributes
} vga_cursor_t;

#define ON  ( 1U )
#define OFF ( 0U )

// Default VGA Character
#define VGA_CHAR_DEFAULT_ATTR VGA_CHAR_ATTR( OFF, OFF, VGA_BLACK, VGA_WHITE )

// Cursor
#define VGA_update_cursor() VGA_set_cursor_pos( vga_cursor.pos )

/* Private Global Variables */

static vga_cursor_t vga_cursor = {
    .pos = { .row = 0, .col = 0 }, .vga_char = { .ch = '_', .attr = VGA_CHAR_DEFAULT_ATTR }
};

/* Private Functions */

// TODO: Add support for escape sequences
// TODO: Add support for moving the cursor (i.e. move it to delete a previous character, etc.)

#define LOW_BYTE( x )  ( (uint8_t)( ( x ) & 0xFF ) )
#define HIGH_BYTE( x ) ( (uint8_t)( ( ( x ) >> 8 ) & 0xFF ) )

void VGA_set_cursor_pos( vga_pos_t pos )
{
    // Calculate the position
    uint16_t pos_idx = CONVERT_POS( pos.row, pos.col );

    // Send the low byte
    outb( VGA_DATA_PORT_A, 0xF );
    outb( VGA_DATA_PORT_B, LOW_BYTE( pos_idx ) );

    // Send the high byte
    outb( VGA_DATA_PORT_A, 0xE );
    outb( VGA_DATA_PORT_B, HIGH_BYTE( pos_idx ) );
}

vga_pos_t VGA_get_cursor_pos( void )
{
    uint16_t pos = 0;

    outb( VGA_DATA_PORT_A, 0x0F );
    pos |= inb( VGA_DATA_PORT_B );

    outb( VGA_DATA_PORT_A, 0x0E );
    pos |= ( ( (uint16_t)inb( VGA_DATA_PORT_B ) ) << 8 );

    vga_pos_t cursor_pos = {
        .row = (uint8_t)( pos / VGA_NUM_COLS ), .col = (uint8_t)( pos % VGA_NUM_COLS )
    };

    return cursor_pos;
}

void VGA_enable_cursor( void )
{
    // Update the cursor position
    VGA_update_cursor();

    // Set the cursor scanline start
    outb( VGA_DATA_PORT_A, 0x0A );
    outb( VGA_DATA_PORT_B, ( inb( VGA_DATA_PORT_B ) & 0xC0 ) | 0x00U );

    // Set the cursor scanline end
    outb( VGA_DATA_PORT_A, 0x0B );
    outb( VGA_DATA_PORT_B, ( inb( VGA_DATA_PORT_B ) & 0xE0 ) | VGA_NUM_ROWS );

    // Make sure the current character cell
    // VGA_PUTC( '\0', vga_cursor.vga_char.attr );
}

void VGA_disable_cursor( void )
{
    outb( VGA_DATA_PORT_A, 0xA );
    outb( VGA_DATA_PORT_B, 0x20 );

    VGA_CLEAR_CHAR();
}

char VGA_get_char( uint8_t row, uint8_t col )
{
    return (char)( (vga_char_t *)( VGA_BUFFER + CONVERT_POS( row, col ) ) )->ch;
}

char VGA_get_char_at_cursor( void )
{
    return VGA_get_char( vga_cursor.pos.row, vga_cursor.pos.col );
}

void VGA_scroll( uint8_t lines )
{
    // Trivial case
    if ( lines == 0 )
    {
        return;
    }

    // Check if we're scrolling more than the screen size
    if ( lines >= VGA_NUM_ROWS )
    {
        // Clear the VGA buffer
        VGA_clear();

        // Set the cursor to the top left
        vga_cursor.pos.row = 0;
        vga_cursor.pos.col = 0;

        // Update the cursor
        VGA_update_cursor();

        return;
    }

    // Copy the VGA buffer
    memcpy(
        VGA_BUFFER, ( VGA_BUFFER + ( (uint16_t)( lines * VGA_NUM_COLS ) ) ),
        (uint16_t)( ( VGA_TOTAL_SIZE - ( lines * VGA_NUM_COLS ) ) * 2 )
    );

    // Clear the bottom lines
    memset(
        ( VGA_BUFFER + ( (uint16_t)( ( VGA_NUM_ROWS - lines ) * VGA_NUM_COLS ) ) ), 0,
        (uint16_t)( ( lines * VGA_NUM_COLS ) * 2 )
    );

    // Move the cursor appropriately
    vga_cursor.pos.row = ( vga_cursor.pos.row > lines ) ? ( vga_cursor.pos.row - lines ) : 0;

    // Update the cursor
    VGA_update_cursor();
}

void VGA_process_backspace( void )
{
    // Check if we're at the beginning of the line
    if ( vga_cursor.pos.col == 0 )
    {
        // Check if we're at the beginning of the screen
        if ( vga_cursor.pos.row == 0 )
        {
            return;
        }

        // Move the cursor to the end of the previous line
        vga_cursor.pos.row--;
        vga_cursor.pos.col = VGA_NUM_COLS;
    }

    // Move the cursor back further until we find the last character
    do
    {
        vga_cursor.pos.col--;
    } while ( VGA_get_char_at_cursor() == '\0' && vga_cursor.pos.col > 0 );

    // Clear the character
    VGA_CLEAR_CHAR();
}

void VGA_cursor_home( void )
{
    vga_cursor.pos.row = 0;
    vga_cursor.pos.col = 0;

    VGA_update_cursor();
}

/* Public Functions */

driver_status_t VGA_driver_init( void )
{
    // Clear the screen
    VGA_clear();

    // Enable the cursor
    VGA_enable_cursor();

    return SUCCESS;
}

void VGA_clear( void )
{
    // Clear the VGA buffer (2 bytes per character)
    memset( VGA_BUFFER, 0, (uint16_t)( VGA_TOTAL_SIZE * 2 ) );

    // Reset the cursor position
    vga_cursor.pos.row = 0;
    vga_cursor.pos.col = 0;

    // Update the cursor
    VGA_update_cursor();
}

void VGA_display_attr_char( char c, uint8_t attr )
{
    // Disable interrupts
    IRQ_disable();

    // Handle special characters
    switch ( c )
    {
        // Return
        case CR:
            // Use a space as a placeholder
            VGA_PUTC( ' ', attr );
            // Reset the cursor position
            vga_cursor.pos.col = 0;

            break;

        // Newline
        case LF:
            // If we're currently at the beginning of a line, and the previous line is full but
            // doesnt end with a newline, we can ignore this newline
            if ( vga_cursor.pos.col == 0 && vga_cursor.pos.row > 0 &&
                 VGA_get_char( vga_cursor.pos.row - 1, VGA_NUM_COLS - 1 ) != '\0' &&
                 VGA_get_char( vga_cursor.pos.row - 1, VGA_NUM_COLS - 1 ) != LF )
            {
                break;
            }
            // Use a space as a placeholder
            VGA_PUTC( ' ', attr );
            // Trick the logic into thinking we need to move to the next line
            vga_cursor.pos.col = VGA_NUM_COLS;

            break;

        // Tab
        case TAB:
            // Use a space as a placeholder
            VGA_PUTC( ' ', attr );
            // Increment the cursor until we're at the next tab stop
            while ( vga_cursor.pos.col % TAB_LENGTH != 0 )
            {
                vga_cursor.pos.col++;
            }

            break;

        // Backspace & Delete
        case BS:
        // TODO: Implement delete properly ( requires moving characters to the left :/ )
        case DEL:
            VGA_process_backspace();

            break;

        // Escape Character
        case ESC:
            // TODO: Handle escape sequences
            VGA_display_str( "Escape sequences are not yet supported! :(\n" );

            return;

        // Regular characters
        default:
            // Print the character
            VGA_PUTC( c, attr );
            // Increment the cursor
            vga_cursor.pos.col++;

            break;
    }

    // Check if we need to move to the next line
    if ( vga_cursor.pos.col >= VGA_NUM_COLS )
    {
        vga_cursor.pos.col = 0;
        vga_cursor.pos.row++;

        // Check if we need to scroll
        if ( vga_cursor.pos.row >= VGA_NUM_ROWS )
        {
            VGA_scroll( 1 );
            vga_cursor.pos.row = VGA_NUM_ROWS - 1;
        }
    }

    // Re-enable interrupts (if they were enabled to begin with)
    IRQ_reenable();
}

void VGA_display_attr_str( const char *s, uint8_t attr )
{
    size_t i, len = strlen( s );

    // Clear the current cursor
    VGA_disable_cursor();

    // Print the string
    for ( i = 0; i < len; ++i )
    {
        VGA_display_attr_char( s[i], attr );
    }

    // Update the cursor
    VGA_enable_cursor();
}

void VGA_display_char( char c ) { VGA_display_str( &c ); }

void VGA_display_str( const char *s ) { VGA_display_attr_str( s, VGA_CHAR_DEFAULT_ATTR ); }

void VGA_display_attr_char_pos( uint8_t x, uint8_t y, char c, uint8_t fg_color, uint8_t bg_color )
{
    // Make sure the positions stay inside the screen
    vga_pos_t p = {
        .row = ( ( x < VGA_NUM_COLS ) ? x : ( VGA_NUM_COLS - 1 ) ),
        .col = ( ( y < VGA_NUM_ROWS ) ? y : ( VGA_NUM_ROWS - 1 ) )
    };

    // Move the cursor
    VGA_disable_cursor();
    VGA_set_cursor_pos( p );

    // Display the character
    VGA_display_attr_char( c, VGA_CHAR_ATTR( false, false, bg_color, fg_color ) );

    // Re-enable the cursor
    VGA_enable_cursor();
}

/*** End of File ***/