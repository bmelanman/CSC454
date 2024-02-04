/** @file irq_handler.c
 *
 * @brief Interrupt Request (IRQ) handler implementation.
 *
 * @author Bryce Melander
 * @date Jan-25-2024
 *
 * @copyright (c) 2024 by Bryce Melander under MIT License. All rights reserved.
 * (See http://opensource.org/licenses/MIT for more details.)
 */

#include "irq_handler.h"

/* Includes */

#include "idt.h"
#include "pic.h"
#include "vga_driver.h"

/* Private Defines and Macros */

// Interrupt Descriptor Table Struct
typedef struct
{
    irq_handler_t handler;
    void* arg;
} irq_handler_entry_t;

/* Global Variables */

irq_handler_entry_t irq_handler_table[IDT_MAX_IRQ];

/* Private Functions */

void print_exception( int irq )
{
    OS_ERROR(
        "Exception Occurred!\n"
        "IRQ: %d - ",
        irq
    );

    switch ( irq )
    {
        case IRQ0_DIV0:
            printk( "Divide by zero error\n" );
            break;
        case IRQ1_DEBUG:
            printk( "Debug exception\n" );
            break;
        case IRQ2_NMI:
            printk( "Non-maskable interrupt\n" );
            break;
        case IRQ3_BREAKPOINT:
            printk( "Breakpoint exception\n" );
            break;
        case IRQ4_OVERFLOW:
            printk( "Overflow exception\n" );
            break;
        case IRQ5_BOUND_RANGE:
            printk( "Bound range exceeded exception\n" );
            break;
        case IRQ6_INVALID_OPCODE:
            printk( "Invalid opcode exception\n" );
            break;
        case IRQ7_DEVICE_NOT_AVAILABLE:
            printk( "Device not available exception\n" );
            break;
        case IRQ8_DOUBLE_FAULT:
            printk( "Double fault exception\n" );
            break;
        case IRQ9_COPROC_SEG_OVERRUN:
            printk( "Coprocessor segment overrun exception\n" );
            break;
        case IRQ10_PAGE_FAULT:
            printk( "Page fault exception\n" );
            break;
        case IRQ11_GENERAL_PROTECTION:
            printk( "General protection exception\n" );
            break;
        case IRQ12_COPROCESSOR_FAULT:
            printk( "Coprocessor fault exception\n" );
            break;
        case IRQ13_ALIGNMENT_CHECK:
            printk( "Alignment check exception\n" );
            break;
        case IRQ14_MACHINE_CHECK:
            printk( "Machine check exception\n" );
            break;
        case IRQ15_SIMD_FLOATING_POINT:
            printk( "SIMD floating point exception\n" );
            break;
        case IRQ16_VIRTUALIZATION:
            printk( "Virtualization exception\n" );
            break;
        case IRQ17_CONTROL_PROTECTION:
            printk( "Control protection exception\n" );
            break;
        case IRQ30_SECURITY:
            printk( "Security exception\n" );
            break;
        default:
            printk( "We should absolutely never be here!!!\n" );
            break;
    }
}

__noreturn void exception_handler( int irq )
{
    // Print the exception
    print_exception( irq );

    // Halt
    HLT();

    // Should never get here
    while ( 1 )
        ;
}

void interrupt_handler( int irq, int error )
{
    // Validate the IRQ
    if ( IS_VALID_IRQ( (uint16_t)irq ) )
    {
        if ( irq_handler_table[irq].handler != NULL )
        {
            // Call the IRQ handler
            irq_handler_table[irq].handler( irq, error, irq_handler_table[irq].arg );
        }
        else
        {
            // Unhandled interrupt error
            OS_ERROR( "Unhandled interrupt!!! IRQ: %d\n", irq );
        }
    }

    IRQ_end_of_interrupt( irq );
}

void default_handler( int __unused irq, __unused int error, __unused void* arg )
{
    // Do nothing!
}

/* Public Functions */

driver_status_t IRQ_init( void )
{
    // Disable interrupts
    CLI();

    // Initialize the IDT
    idt_init();

    // Remap the PICs to the specified offsets
    PIC_init();

    // DEBUG: Install the default handler for IRQ 32 to ignore it
    IRQ_set_handler( IRQ32_TIMER, default_handler, NULL );

    return SUCCESS;
}

int IRQ_set_handler( uint16_t irq, irq_handler_t handler, void* arg )
{
    // Validate the IRQ
    if ( !IS_VALID_IRQ( irq ) )
    {
        OS_ERROR( "Invalid IRQ: %d\n", irq );
        return -1;
    }

    // Set the handler and arg in the table
    irq_handler_table[irq].handler = handler;
    irq_handler_table[irq].arg = arg;

    return 0;
}

/*** End of File ***/