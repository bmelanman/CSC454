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

# include "common.h"
# include "pic.h"

/* Typedefs */

typedef void ( *irq_handler_t )( int, int, void* );

/* Defines */

# define IRQ0_DIV0                 ( 0x00U )  // 0 - Divide by zero
# define IRQ1_DEBUG                ( 0x01U )  // 1 - Debug
# define IRQ2_NMI                  ( 0x02U )  // 2 - Non-maskable interrupt
# define IRQ3_BREAKPOINT           ( 0x03U )  // 3 - Breakpoint
# define IRQ4_OVERFLOW             ( 0x04U )  // 4 - Overflow
# define IRQ5_BOUND_RANGE          ( 0x05U )  // 5 - Bound range exceeded
# define IRQ6_INVALID_OPCODE       ( 0x06U )  // 6 - Invalid opcode
# define IRQ7_DEVICE_NOT_AVAILABLE ( 0x07U )  // 7 - Device not available
# define IRQ8_DOUBLE_FAULT         ( 0x08U )  // 8 - Double fault
# define IRQ9_COPROC_SEG_OVERRUN   ( 0x09U )  // 9 - Coprocessor segment overrun
# define IRQ10_INVALID_TSS         ( 0x0AU )  // 10 - Invalid TSS
# define IRQ11_SEGMENT_NOT_PRESENT ( 0x0BU )  // 11 - Segment not present
# define IRQ12_STACK_SEG_FAULT     ( 0x0CU )  // 12 - Stack-segment fault
# define IRQ13_GEN_PROT_FAULT      ( 0x0DU )  // 13 - General protection fault
# define IRQ14_PAGE_FAULT          ( 0x0EU )  // 14 - Page fault
# define IRQ15_RESERVED            ( 0x0FU )  // 15 - Reserved
# define IRQ16_FPU_EXCEPTION       ( 0x10U )  // 16 - x87 floating-point exception
# define IRQ17_ALIGNMENT_CHECK     ( 0x11U )  // 17 - Alignment check
# define IRQ18_MACHINE_CHECK       ( 0x12U )  // 18 - Machine check
# define IRQ19_SIMD_FP_EXCEPTION   ( 0x13U )  // 19 - SIMD floating-point exception
# define IRQ20_VIRT_EXCEPTION      ( 0x14U )  // 20 - Virtualization exception
# define IRQ21_CTRL_PROT_EXCEPTION ( 0x15U )  // 21 - Control protection exception
/* Interrupts 22-27 are reserved */
# define IRQ28_HV_INJECT_EXCEPTION ( 0x1CU )  // 28 - Hypervisor injection exception
# define IRQ29_VMM_COMM_EXCEPTION  ( 0x1DU )  // 29 - VMM communication exception
# define IRQ30_SECURITY            ( 0x1EU )  // 30 - Security exception
/* Interrupt 31 is reserved */

# define SYS_IRQ_MAX ( 32U )
# define IDT_MAX_IRQ ( PIC2_MAX_IRQ + 1 )

/* Macros */

// Disable an IRQ
# define IRQ_set_mask( irq ) PIC_set_mask( irq )
// Enable an IRQ
# define IRQ_clear_mask( irq ) PIC_clear_mask( irq )

// Get the mask of an IRQ
# define IRQ_get_mask( irq ) PIC_get_mask( irq )

// Send an End of Interrupt (EOI)
# define IRQ_end_of_interrupt( irq ) PIC_send_EOI( irq )

// Check if an IRQ is an exception
# define IS_EXCEPTION( irq )                                                            \
        ( irq == IRQ8_DOUBLE_FAULT ) || ( irq == IRQ10_INVALID_TSS ) ||                 \
            ( irq == IRQ11_SEGMENT_NOT_PRESENT ) || ( irq == IRQ12_STACK_SEG_FAULT ) || \
            ( irq == IRQ13_GEN_PROT_FAULT ) || ( irq == IRQ14_PAGE_FAULT ) ||           \
            ( irq == IRQ17_ALIGNMENT_CHECK ) || ( irq == IRQ30_SECURITY )

// Check if an IRQ is valid
# define IS_VALID_IRQ( irq ) ( IS_PIC_IRQ( irq ) && !( IS_EXCEPTION( irq ) ) )

/* Public Functions */

void exception_handler( int irq, int error, void* arg );

void interrupt_handler( int irq, int error );

driver_status_t IRQ_init( void );

int IRQ_set_handler( uint16_t irq, irq_handler_t handler, void* arg );

int IRQ_set_exception_handler( uint16_t irq, irq_handler_t handler, void* arg );

void IRQ_enable( void );

void IRQ_disable( void );

void IRQ_reenable( void );

int IRQs_are_enabled( void );

#endif /* IRQ_HANDLER_H */

/*** End of File ***/