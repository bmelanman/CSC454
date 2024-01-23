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

// VGA Character Colors
# define VGA_COLOR_BLACK   0x00
# define VGA_COLOR_BLUE    0x01
# define VGA_COLOR_GREEN   0x02
# define VGA_COLOR_CYAN    0x03
# define VGA_COLOR_RED     0x04
# define VGA_COLOR_MAGENTA 0x05
# define VGA_COLOR_BROWN   0x06
# define VGA_COLOR_WHITE   0x07

/* Macros */

/* Typedefs */

/* Public Functions */

driver_status_t VGA_driver_init( void );

void VGA_clear( void );

void VGA_display_char( char );

void VGA_display_str( const char* );

#endif /* VGA_DRIVER_H */

/*** End of File ***/