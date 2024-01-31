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
# include "isr_common.h"

/* Defines */

# define IRQ1_KEYBOARD ( 0x21U )

/* Public Functions */

void interrupt_handler( int irq, int error );

void exception_handler( int error );

driver_status_t IRQ_init( void );

void IRQ_set_mask( int irq );

void IRQ_clear_mask( int irq );

int IRQ_get_mask( int IRQline );

void IRQ_end_of_interrupt( int irq );

void IRQ_set_handler( uint16_t irq, irq_handler_t handler, void* arg );

#endif /* IRQ_HANDLER_H */

/*** End of File ***/