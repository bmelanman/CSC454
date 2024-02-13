/** @file vga_driver.h
 *
 * @brief Header file for the VGA driver.
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

// VGA buffer sizes
# define VGA_NUM_COLS   ( 80U )
# define VGA_NUM_ROWS   ( 25U )
# define VGA_TOTAL_SIZE ( VGA_NUM_COLS * VGA_NUM_ROWS )

// Special Characters
# define ESC ( 0x1B )
# define BS  ( '\b' )
# define DEL ( 0x7F )
# define TAB ( '\t' )
# define LF  ( '\n' )
# define CR  ( '\r' )
# define NUL ( '\0' )

// VGA Character Colors
# define VGA_BLACK         ( 0x00U )
# define VGA_BLUE          ( 0x01U )
# define VGA_GREEN         ( 0x02U )
# define VGA_CYAN          ( 0x03U )
# define VGA_RED           ( 0x04U )
# define VGA_PURPLE        ( 0x05U )
# define VGA_ORANGE        ( 0x06U )
# define VGA_LIGHT_GREY    ( 0x07U )
# define VGA_DARK_GREY     ( 0x08U )
# define VGA_BRIGHT_BLUE   ( 0x09U )
# define VGA_BRIGHT_GREEN  ( 0x0AU )
# define VGA_BRIGHT_CYAN   ( 0x0BU )
# define VGA_MAGENTA       ( 0x0CU )
# define VGA_BRIGHT_PURPLE ( 0x0DU )
# define VGA_YELLOW        ( 0x0EU )
# define VGA_WHITE         ( 0x0FU )

// VGA Character Attributes
# define VGA_CHAR_BLINK_OFFSET 7
# define VGA_CHAR_BKGND_OFFSET 4
# define VGA_CHAR_INTNS_OFFSET 3
# define VGA_CHAR_FRGND_OFFSET 0

/* Macros */

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

/* Typedefs */

/* Public Functions */

driver_status_t VGA_driver_init( void );

void VGA_clear( void );

void VGA_display_char( char c );

void VGA_display_attr_char( char c, uint8_t attr );

void VGA_display_attr_char_pos( uint8_t x, uint8_t y, char c, uint8_t fg_color, uint8_t bg_color );

void VGA_display_str( const char* s );

void VGA_display_attr_str( const char* s, uint8_t attr );

#endif /* VGA_DRIVER_H */

/*** End of File ***/