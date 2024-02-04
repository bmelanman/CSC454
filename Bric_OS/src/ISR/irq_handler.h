/** @file irq_handler.h
 *
 * @brief Header for Interrupt Request (IRQ) handler implementation.
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

# include "isr_common.h"

/* Defines */

/* Macros */

// Disable an IRQ
# define IRQ_set_mask( irq ) PIC_set_mask( irq )

// Enable an IRQ
# define IRQ_clear_mask( irq ) PIC_clear_mask( irq )

// Get the mask of an IRQ
# define IRQ_get_mask( irq ) PIC_get_mask( irq )

// Send an End of Interrupt (EOI)
# define IRQ_end_of_interrupt( irq ) PIC_send_EOI( irq )

/* Public Functions */

void interrupt_handler( int irq, int error );

void exception_handler( int irq );

driver_status_t IRQ_init( void );

int IRQ_set_handler( uint16_t irq, irq_handler_t handler, void* arg );

#endif /* IRQ_HANDLER_H */

/*** End of File ***/