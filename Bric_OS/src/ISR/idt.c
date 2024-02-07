/** @file idt.c
 *
 * @brief Interrupt Descriptor Table (IDT) implementation.
 *
 * @author Bryce Melander
 * @date Jan-25-2024
 *
 * @copyright (c) 2024 by Bryce Melander under MIT License. All rights reserved.
 * (See http://opensource.org/licenses/MIT for more details.)
 */

#include "idt.h"

/* Includes */

#include "irq_handler.h"

/* Private Defines and Macros */

#define GDT_OFFSET_KMODE_CODE_SEG ( 0x08 )
#define GDT_OFFSET_KMODE_DATA_SEG ( 0x10 )

#define GP_FAULT_IST ( 0x01 )
#define PF_FAULT_IST ( 0x02 )
#define DF_FAULT_IST ( 0x03 )

/* Typedefs and Structs */

// Pointer struct to IDT
typedef struct __packed
{
    uint16_t limit;  // Size of IDT in bytes - 1
    uint64_t base;   // Base address of IDT
} idtr_t;

/* Global Variables */

// Table of IDT entries
static idt_entry_t idt[IDT_MAX_IRQ];

// Table of vectors
static __unused bool vectors[IDT_MAX_IRQ];

/* Private Functions */

void idt_set_descriptor( uint16_t irq, void* isr_addr, uint8_t flags )
{
    idt_entry_t* descriptor = &idt[irq];

    descriptor->isr_low = (uint64_t)isr_addr & 0x0000FFFF;
    descriptor->kernel_cs = GDT_OFFSET_KMODE_CODE_SEG;
    descriptor->ist = 0;
    descriptor->attributes = flags;
    descriptor->isr_mid = ( (uint64_t)isr_addr >> 16 ) & 0x0000FFFF;
    descriptor->isr_high = ( (uint64_t)isr_addr >> 32 ) & 0xFFFFFFFF;
    descriptor->reserved = 0;
}

void idt_set_ist( uint16_t irq, uint8_t ist )
{
    idt_entry_t* descriptor = &idt[irq];

    descriptor->ist = ist;
}

/* Public Functions */

void idt_init( void )
{
    uint32_t irq, flags = ( PRESENT_FLAG | INTERRUPT_GATE_FLAG );

    // Setup the IDT and exception handlers
    for ( irq = 0; irq < IDT_MAX_IRQ; ++irq )
    {
        // Set the descriptor
        idt_set_descriptor( irq, isr_stub_table[irq], flags );
        // Mark the irq as in use
        vectors[irq] = true;
    }

    // Set the ist for #GP, #DF, and #PF
    idt_set_ist( IRQ8_DOUBLE_FAULT, GP_FAULT_IST );
    idt_set_ist( IRQ13_GEN_PROT_FAULT, DF_FAULT_IST );
    idt_set_ist( IRQ14_PAGE_FAULT, PF_FAULT_IST );

    // Setup the idt pointer
    idtr_t idt_ptr = {
        .limit = (uint16_t)sizeof( idt_entry_t ) * ( IDT_MAX_IRQ - 1 ), .base = (uint64_t)&idt[0]
    };

    // Load the idt
    asm volatile( "lidt %0" : : "m"( idt_ptr ) );
}

/*** End of File ***/