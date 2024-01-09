/** @file vga_driver.c
 *
 * @brief A description of the module's purpose.
 *
 * @author Bryce Melander
 * @date Jan-08-2024
 *
 * @copyright (c) 2024 by Bryce Melander under MIT License. All rights reserved.
 * (See http://opensource.org/licenses/MIT for more details.)
 */

#include "vga_driver.h"

#include <string.h>

/* Private Defines and Macros */

#define __unused        __attribute__( ( unused ) )
#define __packed        __attribute__( ( packed ) )
#define __format_printf __attribute__( ( format( printf, 1, 2 ) ) )

#define VGA_BUFFER ( (uint16_t *)0xB8000 )

#define VGA_PUTC( ch, attrs )                                        \
    ( VGA_BUFFER[GET_CURSOR_POS( vga_cursor.row, vga_cursor.col )] = \
          ( (uint16_t)( ( ( (uint8_t)( attrs ) << 8 ) | (uint8_t)( ch ) ) ) ) )

#define VGA_WIDTH  ( 80U )
#define VGA_HEIGHT ( 25U )
#define VGA_SIZE   ( VGA_WIDTH * VGA_HEIGHT )

#define GET_CURSOR_POS( row, col ) ( ( ( row ) * VGA_WIDTH ) + ( col ) )

/* Private Typedefs */

typedef struct
{
    uint8_t row;
    uint8_t col;
    uint8_t ch;
    uint8_t attr;
} vga_cursor_t;

typedef struct
{
    uint8_t ch;
    uint8_t attr;
} vga_char_t;

// VGA Character Attributes
#define VGA_CHAR_BLINK_OFFSET 7
#define VGA_CHAR_BKGND_OFFSET 4
#define VGA_CHAR_INTNS_OFFSET 3
#define VGA_CHAR_FRGND_OFFSET 0

#define VGA_CHAR_ATTR( blink, intns, bkgnd, frgnd )         \
    ( ( ( (uint8_t)( blink ) ) << VGA_CHAR_BLINK_OFFSET ) | \
      ( ( (uint8_t)( intns ) ) << VGA_CHAR_INTNS_OFFSET ) | \
      ( ( (uint8_t)( bkgnd ) ) << VGA_CHAR_BKGND_OFFSET ) | \
      ( ( (uint8_t)( frgnd ) ) << VGA_CHAR_FRGND_OFFSET ) )

#define ON  ( 1U )
#define OFF ( 0U )

#define BLINK_ON  ( ON )
#define BLINK_OFF ( OFF )

#define ENABLE_BLINK( attr )  ( ( attr ) | ( ON << VGA_CHAR_BLINK_OFFSET ) )
#define DISABLE_BLINK( attr ) ( ( attr ) & ( ~( ON << VGA_CHAR_BLINK_OFFSET ) ) )

#define INTNS_ON  ( ON )
#define INTNS_OFF ( OFF )

#define ENABLE_INTNS( attr )  ( ( attr ) | ( ON << VGA_CHAR_INTNS_OFFSET ) )
#define DISABLE_INTNS( attr ) ( ( attr ) & ( ~( ON << VGA_CHAR_INTNS_OFFSET ) ) )

// Default VGA Character
#define VGA_CHAR_DEFAULT_ATTR \
    VGA_CHAR_ATTR( BLINK_OFF, INTNS_OFF, VGA_COLOR_BLACK, VGA_COLOR_WHITE )

// Cursor
#define PRINT_CURSOR() VGA_PUTC( vga_cursor.ch, vga_cursor.attr )

/* Private Global Variables */

static vga_cursor_t vga_cursor;

/* Private Functions */

vga_char_t *VGA_get_char( uint8_t row, uint8_t col )
{
    return (vga_char_t *)( VGA_BUFFER + GET_CURSOR_POS( row, col ) );
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
        vga_cursor.row = 0;
        vga_cursor.col = 0;

        // Update the cursor
        PRINT_CURSOR();

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
    vga_cursor.row = ( vga_cursor.row > lines ) ? ( vga_cursor.row - lines ) : 0;

    // Update the cursor
    PRINT_CURSOR();
}

/* Public Functions */

void VGA_clear( void )
{
    int i, len = VGA_WIDTH * VGA_HEIGHT;

    // Clear the VGA buffer
    for ( i = 0; i < len; i++ )
    {
        VGA_BUFFER[i] = 0;
    }
}

void VGA_init( void )
{
    // Clear the VGA buffer
    VGA_clear();

    // Set the cursor to the top left
    vga_cursor.row = 0;
    vga_cursor.col = 0;

    // Set the default character and attributes
    vga_cursor.ch = '_';
    vga_cursor.attr = VGA_CHAR_ATTR( BLINK_OFF, INTNS_OFF, VGA_COLOR_BLACK, VGA_COLOR_WHITE );

    // Update the cursor
    PRINT_CURSOR();
}

void VGA_display_char_attr( char c, uint8_t attr )
{
    // Check character
    switch ( c )
    {
        case '\r':
            // Clear the current cursor
            VGA_PUTC( ' ', attr );
            // Reset the cursor position
            vga_cursor.col = 0;

            break;

        case '\n':
            // Clear the current cursor
            VGA_PUTC( ' ', attr );
            // Trick the logic into thinking we need to move to the next line
            vga_cursor.col = VGA_WIDTH;

            break;

        default:
            // Print the character
            VGA_PUTC( c, attr );
            // Increment the cursor
            vga_cursor.col++;

            break;
    }

    // Check if we need to move to the next line
    if ( vga_cursor.col >= VGA_WIDTH )
    {
        vga_cursor.col = 0;
        vga_cursor.row++;

        // Check if we need to scroll
        if ( vga_cursor.row >= VGA_HEIGHT )
        {
            VGA_scroll( 1 );
            vga_cursor.row = VGA_HEIGHT - 1;
        }
    }

    // Update the cursor
    PRINT_CURSOR();
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

void VGA_display_char( char c ) { VGA_display_char_attr( c, VGA_CHAR_DEFAULT_ATTR ); }

void VGA_display_str( const char *s ) { VGA_display_str_attr( s, VGA_CHAR_DEFAULT_ATTR ); }

/*** End of File ***/