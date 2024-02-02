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

#include "common.h"
#include "isr_common.h"

/* Private Defines and Macros */

#define VGA_BUFFER ( (uint16_t *)0xB8000 )

#define VGA_DATA_PORT_A ( 0x3D4U )
#define VGA_DATA_PORT_B ( 0x3D5U )

#define VGA_WIDTH  ( 80U )
#define VGA_HEIGHT ( 25U )
#define VGA_SIZE   ( VGA_WIDTH * VGA_HEIGHT )

#define CONVERT_POS( row, col )     ( ( ( row ) * VGA_WIDTH ) + ( col ) )
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
#define VGA_CHAR_DEFAULT_ATTR VGA_CHAR_ATTR( OFF, OFF, VGA_COLOR_BLACK, VGA_COLOR_WHITE )

// Cursor
#define VGA_update_cursor() VGA_set_cursor_pos( vga_cursor.pos )

/* Private Global Variables */

static vga_cursor_t vga_cursor = {
    .pos = { .row = 0, .col = 0 }, .vga_char = { .ch = '_', .attr = VGA_CHAR_DEFAULT_ATTR }
};

/* Private Functions */

#define LOW_BYTE( x )  ( (uint8_t)( ( x ) & 0xFF ) )
#define HIGH_BYTE( x ) ( (uint8_t)( ( ( x ) >> 8 ) & 0xFF ) )

void VGA_enable_cursor( void )
{
    // Set the cursor scanline start
    outb( VGA_DATA_PORT_A, 0x0A );
    outb( VGA_DATA_PORT_B, ( inb( VGA_DATA_PORT_B ) & 0xC0 ) | 0x00U );

    // Set the cursor scanline end
    outb( VGA_DATA_PORT_A, 0x0B );
    outb( VGA_DATA_PORT_B, ( inb( VGA_DATA_PORT_B ) & 0xE0 ) | VGA_HEIGHT );

    // Make sure the current character cell
    VGA_PUTC( '\0', vga_cursor.vga_char.attr );
}

void VGA_disable_cursor( void )
{
    outb( VGA_DATA_PORT_A, 0xA );
    outb( VGA_DATA_PORT_B, 0x20 );

    VGA_CLEAR_CHAR();
}

vga_pos_t __unused VGA_get_cursor_pos( void )
{
    uint16_t pos = 0;

    outb( VGA_DATA_PORT_A, 0x0F );
    pos |= inb( VGA_DATA_PORT_B );

    outb( VGA_DATA_PORT_A, 0x0E );
    pos |= ( ( (uint16_t)inb( VGA_DATA_PORT_B ) ) << 8 );

    vga_pos_t cursor_pos = {
        .row = (uint8_t)( pos / VGA_WIDTH ), .col = (uint8_t)( pos % VGA_WIDTH )
    };

    return cursor_pos;
}

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
    if ( lines >= VGA_HEIGHT )
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
        VGA_BUFFER, ( VGA_BUFFER + ( (uint16_t)( lines * VGA_WIDTH ) ) ),
        (uint16_t)( ( VGA_SIZE - ( lines * VGA_WIDTH ) ) * 2 )
    );

    // Clear the bottom lines
    memset(
        ( VGA_BUFFER + ( (uint16_t)( ( VGA_HEIGHT - lines ) * VGA_WIDTH ) ) ), 0,
        (uint16_t)( ( lines * VGA_WIDTH ) * 2 )
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
        vga_cursor.pos.col = VGA_WIDTH;
    }

    // Move the cursor back further until we find the last character
    do
    {
        vga_cursor.pos.col--;
    } while ( VGA_get_char_at_cursor() == '\0' && vga_cursor.pos.col > 0 );

    // Clear the character
    VGA_CLEAR_CHAR();
}

/* Public Functions */

void VGA_clear( void )
{
    // Clear the VGA buffer (2 bytes per character)
    memset( VGA_BUFFER, 0, (uint16_t)( VGA_SIZE * 2 ) );

    // Reset the cursor position
    vga_cursor.pos.row = 0;
    vga_cursor.pos.col = 0;

    // Update the cursor
    VGA_update_cursor();
}

driver_status_t VGA_driver_init( void )
{
    // Clear the screen
    VGA_clear();

    // Enable the cursor
    VGA_enable_cursor();

    return SUCCESS;
}

// TODO: Fix any race conditions
// TODO: Add support for escape sequences
// TODO: Add support for moving the cursor (i.e. move it to delete a previous character, etc.)
void VGA_display_char_attr( char c, uint8_t attr )
{
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
            // Use a space as a placeholder
            VGA_PUTC( ' ', attr );
            // Trick the logic into thinking we need to move to the next line
            vga_cursor.pos.col = VGA_WIDTH;

            break;

        // Tab
        case TAB:
            // Use a space as a placeholder
            VGA_PUTC( ' ', attr );
            // Increment the cursor
            vga_cursor.pos.col += TAB_LENGTH;

            break;

        // Backspace & Delete
        case BS:
        // TODO: Implement delete properly
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
    if ( vga_cursor.pos.col >= VGA_WIDTH )
    {
        vga_cursor.pos.col = 0;
        vga_cursor.pos.row++;

        // Check if we need to scroll
        if ( vga_cursor.pos.row >= VGA_HEIGHT )
        {
            VGA_scroll( 1 );
            vga_cursor.pos.row = VGA_HEIGHT - 1;
        }
    }
}

void VGA_display_str_attr( const char *s, uint8_t attr )
{
    size_t i, len = strlen( s );

    // Print the string
    for ( i = 0; i < len; i++ )
    {
        VGA_display_char_attr( s[i], attr );
    }
}

void VGA_display_char( char c )
{
    // Clear the current cursor
    VGA_disable_cursor();

    VGA_display_char_attr( c, VGA_CHAR_DEFAULT_ATTR );

    // Update the cursor
    VGA_enable_cursor();
    VGA_update_cursor();
}

void VGA_display_str( const char *s )
{
    // Clear the current cursor
    VGA_disable_cursor();

    VGA_display_str_attr( s, VGA_CHAR_DEFAULT_ATTR );

    // Update the cursor
    VGA_enable_cursor();
    VGA_update_cursor();
}

/*** End of File ***/