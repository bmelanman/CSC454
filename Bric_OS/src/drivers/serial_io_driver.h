/** @file serial_io_driver.h
 *
 * @brief Header file for the serial I/O driver.
 *
 * @author Bryce Melander
 * @date Feb-01-2024
 *
 * @copyright (c) 2024 by Bryce Melander under MIT License. All rights reserved.
 * (See http://opensource.org/licenses/MIT for more details.)
 */

#ifndef SERIAL_IO_DRIVER_H
# define SERIAL_IO_DRIVER_H

/* Includes */

# include "common.h"

/* Defines */

/* Macros */

/* Typedefs */

/* Public Functions */

driver_status_t serial_driver_init( void );

size_t serial_write( const char *buff, size_t len );

void serial_print( const char *str );

#endif /* SERIAL_IO_DRIVER_H */

/*** End of File ***/