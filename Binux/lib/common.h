/** @file util.h
 *
 * @brief A description of the module's purpose.
 *
 * @author Bryce Melander
 * @date Jan-09-2024
 *
 * @copyright (c) 2024 by Bryce Melander under MIT License. All rights reserved.
 * (See http://opensource.org/licenses/MIT for more details.)
 */

#ifndef UTIL_H
# define UTIL_H

/* Includes */

# include <stdarg.h>
# include <stddef.h>
# include <stdint.h>
# include <string.h>

/* Defines */

# define TRUE  ( 1U )
# define FALSE ( 0U )

/* Macros */

/* Typedefs */

typedef enum { SUCCESS = 0, FAILURE = 1 } driver_status_t;

/* Public Functions */

# define sleep( sec )                                 \
        do                                            \
        {                                             \
            for ( int i = 0; i < sec; i++ )           \
            {                                         \
                for ( int j = 0; j < 100000000; j++ ) \
                {                                     \
                    __asm__ volatile( "nop" );        \
                }                                     \
            }                                         \
        } while ( 0 )

#endif /* UTIL_H */

/*** End of File ***/