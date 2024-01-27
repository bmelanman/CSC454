/** @file irq_handler.h
 *
 * @brief A description of the module's purpose.
 *
 * @author Bryce Melander
 * @date Jan-25-2024
 *
 * @copyright (c) 2024 by Bryce Melander under MIT License. All rights reserved.
 * (See http://opensource.org/licenses/MIT for more details.)
 */

#ifndef IRQ_HANDLER_H
# define IRQ_HANDLER_H

/* Includes */

# include "common.h"

/* Defines */

/* Macros */

# define CLI() asm volatile( "cli" )
# define STI() asm volatile( "sti" )

/* Typedefs */

typedef void ( *irq_handler_t )( int, int, void* );

/* Public Functions */

driver_status_t IRQ_init( void );

void IRQ_set_mask( int irq );

void IRQ_clear_mask( int irq );

int IRQ_get_mask( int IRQline );

void IRQ_end_of_interrupt( int irq );

void IRQ_set_handler( int irq, irq_handler_t handler, __unused void* arg );

#endif /* IRQ_HANDLER_H */

/*** End of File ***/