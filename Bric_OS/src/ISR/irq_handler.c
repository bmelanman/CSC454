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

#include "gdt.h"
#include "idt.h"
#include "pic.h"
#include "timer_driver.h"
#include "vga_driver.h"

/* Private Defines and Macros */

// Interrupt Descriptor Table Struct
typedef struct
{
    irq_handler_t handler;
    void* arg;
} irq_handler_entry_t;

typedef enum { DISABLED = 0, ENABLED = 1 } IRQ_status_t;

/* Global Variables */

irq_handler_entry_t irq_handler_table[IDT_MAX_IRQ];

static IRQ_status_t IRQ_status_prev = DISABLED;
static IRQ_status_t IRQ_status_curr = DISABLED;

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
        case IRQ10_INVALID_TSS:
            printk( "Invalid TSS exception\n" );
            break;
        case IRQ11_SEGMENT_NOT_PRESENT:
            printk( "Segment not present exception\n" );
            break;
        case IRQ12_STACK_SEG_FAULT:
            printk( "Stack segment exception\n" );
            break;
        case IRQ13_GEN_PROT_FAULT:
            printk( "General protection exception\n" );
            break;
        case IRQ14_PAGE_FAULT:
            printk( "Page fault exception\n" );
            break;
        case IRQ16_FPU_EXCEPTION:
            printk( "x87 floating-point exception\n" );
            break;
        case IRQ17_ALIGNMENT_CHECK:
            printk( "Alignment check exception\n" );
            break;
        case IRQ18_MACHINE_CHECK:
            printk( "Machine check exception\n" );
            break;
        case IRQ19_SIMD_FP_EXCEPTION:
            printk( "SIMD floating point exception\n" );
            break;
        case IRQ20_VIRT_EXCEPTION:
            printk( "Virtualization exception\n" );
            break;
        case IRQ21_CTRL_PROT_EXCEPTION:
            printk( "Control protection exception\n" );
            break;
        case IRQ30_SECURITY:
            printk( "Security exception\n" );
            break;
        default:
            printk( "We should absolutely never be here!?\n" );
            break;
    }
}

__noreturn void exception_handler( int irq, int error )
{
    // Print the exception
    print_exception( irq );

    if ( error != 0 )
    {
        printk( "Selector Error Code: 0x%X\n", error );
        printk( "External: %d\n", ( error & 0x1 ) );  // Bit 1
        printk( "Table: " );                          // Bits 2-3
        switch ( ( error >> 1 ) & 0x3 )
        {
            case 0:
                printk( "GDT\n" );
                break;
            case 1:
            case 3:
                printk( "IDT\n" );
                break;
            case 2:
                printk( "LDT\n" );
                break;
            default:
                printk( "Unknown??\n" );
                break;
        }
        printk( "Index: 0x%X\n", ( error >> 3 ) & 0x1FFF );  // Bits 4-15
    }

    // Halt
    HLT();
}

void interrupt_handler( int irq, int error )
{
    printk( "\n" );
    OS_INFO(
        "Interrupt Occurred!    \n"
        "IRQ:   0x%X            \n"
        "Error: 0x%X            \n"
        "\n",
        irq, error
    );

    // Check for a valid handler
    if ( irq_handler_table[irq].handler != NULL )
    {
        // Call the IRQ handler
        irq_handler_table[irq].handler( irq, error, irq_handler_table[irq].arg );
    }
    // Exception without a handler
    else if ( IS_EXCEPTION( irq ) )
    {
        exception_handler( irq, error );
    }
    // Unhandled IRQ
    else
    {
        OS_ERROR( "Unhandled interrupt!!! IRQ: %d\n", irq );
    }

    // Send EOI if neccessary
    if ( IS_PIC_IRQ( irq ) ) IRQ_end_of_interrupt( irq );
}

/* Public Functions */

driver_status_t IRQ_init( void )
{
    // Disable interrupts
    IRQ_disable();

    // Initialize the new GDT
    gdt_init();

    // Initialize the IDT
    idt_init();

    // Initialize the PIC
    PIC_init();

    // Initialize and disable the timer
    timer_init( false );

    // Set the exception handler as the handler for all exceptions (IRQs 8, 10-14, 17, and 30)
    // irq_handler_table[IRQ8_DOUBLE_FAULT].handler = exception_handler;
    // irq_handler_table[IRQ10_INVALID_TSS].handler = exception_handler;
    // irq_handler_table[IRQ11_SEGMENT_NOT_PRESENT].handler = exception_handler;
    // irq_handler_table[IRQ12_STACK_SEG_FAULT].handler = exception_handler;
    // irq_handler_table[IRQ13_GEN_PROT_FAULT].handler = exception_handler;
    // irq_handler_table[IRQ14_PAGE_FAULT].handler = exception_handler;
    // irq_handler_table[IRQ17_ALIGNMENT_CHECK].handler = exception_handler;
    // irq_handler_table[IRQ30_SECURITY].handler = exception_handler;

    return SUCCESS;
}

int IRQ_set_handler( uint16_t irq, irq_handler_t handler, void* arg )
{
    // Validate the IRQ
    if ( !( IS_VALID_IRQ( irq ) ) )
    {
        OS_ERROR( "Invalid IRQ: %d\n", irq );
        return -1;
    }

    // Set the handler and arg in the table
    irq_handler_table[irq].handler = handler;
    irq_handler_table[irq].arg = arg;

    return 0;
}

int IRQ_set_exception_handler( uint16_t irq, irq_handler_t handler, void* arg )
{
    // Validate the IRQ
    if ( !( IS_EXCEPTION( irq ) ) )
    {
        OS_ERROR( "Invalid exception: %d\n", irq );
        return -1;
    }

    // Set the handler and arg in the table
    irq_handler_table[irq].handler = handler;
    irq_handler_table[irq].arg = arg;

    return 0;
}

int IRQs_are_enabled( void ) { return (int)IRQ_status_curr; }

void IRQ_enable( void )
{
    // Modify the globals BEFORE enabling interrupts
    IRQ_status_prev = IRQ_status_curr;
    IRQ_status_curr = ENABLED;

    // Clear the global interrupt mask
    asm volatile( "cli" );
}

void IRQ_disable( void )
{
    // Only disable interrupts if they are currently enabled
    if ( IRQ_status_curr == ENABLED )
    {
        // Set the global interrupt mask
        asm volatile( "sti" );
    }

    // Modify the globals AFTER disabling interrupts
    IRQ_status_prev = IRQ_status_curr;
    IRQ_status_curr = DISABLED;
}

// Re-enable interrupts only if their most recent state change went from ENABLED to DISABLED.
void IRQ_reenable( void )
{
    if ( IRQ_status_prev == ENABLED && IRQ_status_curr == DISABLED )
    {
        IRQ_enable();
    }
}

/*** End of File ***/