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

/* Defines */

/* Macros */

/* Typedefs */

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