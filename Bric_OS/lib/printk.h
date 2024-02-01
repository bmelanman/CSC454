/** @file printk.h
 *
 * @brief Header for kernel print function implementation.
 *
 * @author Bryce Melander
 * @date Jan-09-2024
 *
 * @copyright (c) 2024 by Bryce Melander under MIT License. All rights reserved.
 * (See http://opensource.org/licenses/MIT for more details.)
 */

#ifndef PRINTK_H
# define PRINTK_H

/* Includes */

/* Defines */

/* Macros */

/* Typedefs */

/* Public Functions */

__attribute__( ( format( printf, 1, 2 ) ) ) int printk( const char *fmt, ... );

#endif /* PRINTK_H */

/*** End of File ***/