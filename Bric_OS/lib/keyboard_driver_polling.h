/** @file keyboard_driver_polling.h
 *
 * @brief A description of the module's purpose.
 *
 * @author Bryce Melander
 * @date Jan-10-2024
 *
 * @copyright (c) 2024 by Bryce Melander under MIT License. All rights reserved.
 * (See http://opensource.org/licenses/MIT for more details.)
 */

#ifndef KEYBOARD_DRIVER_POLLING_H
# define KEYBOARD_DRIVER_POLLING_H

/* Includes */

# include "common.h"

/* Defines */

/* Macros */

/* Typedefs */

/* Public Functions */

driver_status_t keyboard_driver_polling_init( void );

char keyboard_driver_polling_get_char( void );

#endif /* KEYBOARD_DRIVER_POLLING_H */

/*** End of File ***/