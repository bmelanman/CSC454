/** @file timer.c
 *
 * @brief
 *
 * @author Bryce Melander
 * @date Feb-04-2024
 *
 * @copyright (c) 2024 by Bryce Melander under MIT License. All rights reserved.
 * (See http://opensource.org/licenses/MIT for more details.)
 */

#include "timer.h"

/* Private Includes */

/* Private Defines and Macros */

/* Private Types and Enums */

/* Global Variables */

bool enabled;

/* Private Functions */

void timer_isr( int __unused irq, int __unused error, void __unused* arg )
{
    // Print a message if the timer is called but not enabled
    if ( !enabled )
    {
        OS_ERROR( "Timer interrupt called but not enabled!\n" );
    }

    // Send an EOI
    IRQ_end_of_interrupt( IRQ32_TIMER );
}

/* Public Functions */

driver_status_t timer_init( bool enable )
{
    // Set the timer interrupt handler
    IRQ_set_handler( IRQ32_TIMER, timer_isr, NULL );

    // Clear the timer interrupt mask if enabled, otherwise set it to disable the interrupt
    if ( enable )
    {
        IRQ_clear_mask( IRQ32_TIMER );
        enabled = true;
    }
    else
    {
        IRQ_set_mask( IRQ32_TIMER );
        enabled = false;
    }

    // Success
    return SUCCESS;
}

/*** End of File ***/