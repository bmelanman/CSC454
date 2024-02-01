/** @file vga_driver.h
 *
 * @brief A description of the module's purpose.
 *
 * @author Bryce Melander
 * @date Jan-08-2024
 *
 * @copyright (c) 2024 by Bryce Melander under MIT License. All rights reserved.
 * (See http://opensource.org/licenses/MIT for more details.)
 */

#ifndef VGA_DRIVER_H
# define VGA_DRIVER_H

/* Includes */

# include "common.h"

/* Defines */

// Special Characters
# define ESC ( 0x1B )
# define BS  ( '\b' )
# define TAB ( '\t' )
# define LF  ( '\n' )
# define CR  ( '\r' )
# define NUL ( '\0' )

// VGA Character Colors
# define VGA_COLOR_BLACK   0x00
# define VGA_COLOR_BLUE    0x01
# define VGA_COLOR_GREEN   0x02
# define VGA_COLOR_CYAN    0x03
# define VGA_COLOR_RED     0x04
# define VGA_COLOR_MAGENTA 0x05
# define VGA_COLOR_BROWN   0x06
# define VGA_COLOR_WHITE   0x07

// VGA Character Attributes
# define VGA_CHAR_BLINK_OFFSET 7
# define VGA_CHAR_BKGND_OFFSET 4
# define VGA_CHAR_INTNS_OFFSET 3
# define VGA_CHAR_FRGND_OFFSET 0

/*
 * @brief Create a VGA character attribute
 * @param blink  Blinking flag
 * @param intns  Intensity flag
 * @param bkgnd  Background color
 * @param frgnd  Foreground color
 */
# define VGA_CHAR_ATTR( blink, intns, bkgnd, frgnd )            \
        ( ( ( (uint8_t)( blink ) ) << VGA_CHAR_BLINK_OFFSET ) | \
          ( ( (uint8_t)( intns ) ) << VGA_CHAR_INTNS_OFFSET ) | \
          ( ( (uint8_t)( bkgnd ) ) << VGA_CHAR_BKGND_OFFSET ) | \
          ( ( (uint8_t)( frgnd ) ) << VGA_CHAR_FRGND_OFFSET ) )

/* Macros */

/* Typedefs */

/* Public Functions */

driver_status_t VGA_driver_init( void );

void VGA_clear( void );

void VGA_display_char( char );

void VGA_display_str( const char* );

#endif /* VGA_DRIVER_H */

/*** End of File ***/