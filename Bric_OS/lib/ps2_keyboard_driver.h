/** @file ps2_keyboard_driver.h
 *
 * @brief A description of the module's purpose.
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

# define NO_CHAR ( -1 )

/* Public Functions */

driver_status_t ps2_keyboard_driver_init( bool irq_enable );

void ps2_keyboard_driver_interrupt_handler( int irq, int error, void *arg );

char IRQ_keyboard_get_char( void );

char polling_keyboard_get_char( void );

#endif /* PS2_KEYBOARD_DRIVER_H */

/*** End of File ***/