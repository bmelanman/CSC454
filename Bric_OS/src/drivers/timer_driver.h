/** @file timer.h
 *
 * @brief
 *
 * @author Bryce Melander
 * @date Feb-04-2024
 *
 * @copyright (c) 2024 by Bryce Melander under MIT License. All rights reserved.
 * (See http://opensource.org/licenses/MIT for more details.)
 */

#ifndef TIMER_H
# define TIMER_H

/* Includes */

# include "irq_handler.h"

/* Defines */

/* Macros */

/* Typedefs */

/* Public Functions */

driver_status_t timer_init( bool enable );

#endif /* TIMER_H */

/*** End of File ***/