/** @file ps2_keyboard_driver.h
 *
 * @brief Header file for the PS/2 keyboard driver.
 *
 * @author Bryce Melander
 * @date Jan-10-2024
 *
 * @copyright (c) 2024 by Bryce Melander under MIT License. All rights reserved.
 * (See http://opensource.org/licenses/MIT for more details.)
 */

#ifndef PS2_KEYBOARD_DRIVER_H
# define PS2_KEYBOARD_DRIVER_H

/* Includes */

# include "common.h"

/* Defines */

/* Public Functions */

driver_status_t ps2_keyboard_driver_init( bool irq_enable );

char polling_keyboard_get_char( void );

#endif /* PS2_KEYBOARD_DRIVER_H */

/*** End of File ***/