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

# include "common.h"
# include "pic.h"

/* Typedefs */

typedef void ( *irq_handler_t )( int, int, void* );

/* Defines */

# define IRQ0_DIV0                 ( 0x00U )  // 0x0 - Divide by zero
# define IRQ1_DEBUG                ( 0x01U )  // 0x1 - Debug
# define IRQ2_NMI                  ( 0x02U )  // 0x2 - Non-maskable interrupt
# define IRQ3_BREAKPOINT           ( 0x03U )  // 0x3 - Breakpoint
# define IRQ4_OVERFLOW             ( 0x04U )  // 0x4 - Overflow
# define IRQ5_BOUND_RANGE          ( 0x05U )  // 0x5 - Bound range exceeded
# define IRQ6_INVALID_OPCODE       ( 0x06U )  // 0x6 - Invalid opcode
# define IRQ7_DEVICE_NOT_AVAILABLE ( 0x07U )  // 0x7 - Device not available
# define IRQ8_DOUBLE_FAULT         ( 0x08U )  // 0x8 - Double fault
# define IRQ9_COPROC_SEG_OVERRUN   ( 0x09U )  // 0x9 - Coprocessor segment overrun
# define IRQ10_PAGE_FAULT          ( 0x0AU )  // 0xA - Page fault
# define IRQ11_GENERAL_PROTECTION  ( 0x0BU )  // 0xB - General protection fault
# define IRQ12_COPROCESSOR_FAULT   ( 0x0CU )  // 0xC - x87 FPU floating-point error
# define IRQ13_ALIGNMENT_CHECK     ( 0x0DU )  // 0xD - Alignment check
# define IRQ14_MACHINE_CHECK       ( 0x0EU )  // 0xE - Machine check
# define IRQ15_SIMD_FLOATING_POINT ( 0x0FU )  // 0xF - SIMD floating-point exception
# define IRQ16_VIRTUALIZATION      ( 0x10U )  // 0x10 - Virtualization exception
# define IRQ17_CONTROL_PROTECTION  ( 0x11U )  // 0x11 - Control protection exception
/* IRQs 18-29, and 31 are reserved */
# define IRQ30_SECURITY ( 0x1EU )  // 0x1E - Security exception

# define SYS_IRQ_MAX ( 32U )
# define IDT_MAX_IRQ ( PIC2_MAX_IRQ + 1 )

/* Macros */

// Disable all interrupts
# define CLI() asm volatile( "cli" )
// Enable all interrupts
# define STI() asm volatile( "sti" )

// Check if an IRQ is an exception
# define IS_EXCEPTION( irq )                                                                \
        ( irq == 8 ) || ( irq == 10 ) || ( irq == 11 ) || ( irq == 12 ) || ( irq == 13 ) || \
            ( irq == 14 ) || ( irq == 17 ) || ( irq == 30 )
//( irq == 8 ) || ( irq >= 10 && irq <= 12 ) || ( irq >= 13 && irq <= 17 ) || ( irq == 30 )
// Check if an IRQ is valid
# define IS_VALID_IRQ( irq ) ( IS_PIC_IRQ( irq ) && !( IS_EXCEPTION( irq ) ) )

#endif /* ISR_COMMON_H */

/*** End of File ***/