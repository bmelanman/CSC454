/** @file ISR_common.h
 *
 * @brief Commonly used ISR functions.
 *
 * @author Bryce Melander
 * @date Jan-30-2024
 *
 * @copyright (c) 2024 by Bryce Melander under MIT License. All rights reserved.
 * (See http://opensource.org/licenses/MIT for more details.)
 */

#ifndef ISR_COMMON_H
# define ISR_COMMON_H

/* Includes */

/* Typedefs */

typedef void ( *irq_handler_t )( int, int, void* );

/* Defines */

# define IRQ_EXCEPTION_MAX ( 32U )

/* Macros */

# define CLI() asm volatile( "cli" )
# define STI() asm volatile( "sti" )
# define HLT() asm volatile( "hlt" )

#endif /* ISR_COMMON_H */

/*** End of File ***/