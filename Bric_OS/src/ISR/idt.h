/** @file idt.h
 *
 * @brief Header file for the Interrupt Descriptor Table (IDT) implementation.
 *
 * @author Bryce Melander
 * @date Jan-25-2024
 *
 * @copyright (c) 2024 by Bryce Melander under MIT License. All rights reserved.
 * (See http://opensource.org/licenses/MIT for more details.)
 */

#ifndef IDT_H
# define IDT_H

/* Includes */

# include "irq_handler.h"

/* Defines */

# define PRESENT_FLAG        ( 0x80U )  // 0b10000000
# define INTERRUPT_GATE_FLAG ( 0x0EU )  // 0b00001110

/* Macros */

/* Typedefs */

// IDT Entry Struct
typedef struct __packed
{
    uint16_t isr_low;    // The lower 16 bits of the ISR's address
    uint16_t kernel_cs;  // GDT segment selector for the CPU to load into CS before calling the ISR
    uint8_t ist;         // The IST in the TSS for the CPU to load into RSP
    uint8_t attributes;  // Type and attributes; see the IDT page
    uint16_t isr_mid;    // The higher 16 bits of the lower 32 bits of the ISR's address
    uint32_t isr_high;   // The higher 32 bits of the ISR's address
    uint32_t reserved;   // Set to zero
} idt_entry_t;

/* Global Variables */

// Table of ISR stub function pointers
extern uint64_t* isr_stub_table[];

/* Public Functions */

// Initialize the IDT
void idt_init( void );

#endif /* IDT_H */

/*** End of File ***/