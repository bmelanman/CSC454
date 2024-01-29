/** @file idt.c
 *
 * @brief A description of the module's purpose.
 *
 * @author Bryce Melander
 * @date Jan-25-2024
 *
 * @copyright (c) 2024 by Bryce Melander under MIT License. All rights reserved.
 * (See http://opensource.org/licenses/MIT for more details.)
 */

#include "idt.h"

/* Private Defines and Macros */

#define GDT_OFFSET_KMODE_CODE_SEG ( 0x08 )
#define GDT_OFFSET_KMODE_DATA_SEG ( 0x10 )

/* Global Variables */

// Table of IDT entries
static idt_entry_t idt[IDT_MAX_DESCRIPTORS];

// Table of vectors
static __unused bool vectors[IDT_MAX_DESCRIPTORS];

// IDTR
static idtr_t idtr;

/* Private Functions */

void idt_reload( idtr_t* idtr )
{
    // Set the IDTR
    idtr->limit = sizeof( idt ) - 1;
    idtr->base = (uint64_t)idt;

    // Load the IDTR
    asm volatile( "lidt %0" : : "m"( idtr ) );
}

/* Public Functions */

void idt_init( void )
{
    idtr.base = (uintptr_t)&idt[0];
    idtr.limit = (uint16_t)sizeof( idt_entry_t ) * IDT_MAX_DESCRIPTORS - 1;

    for ( uint8_t vector = 0; vector < 32; vector++ )
    {
        idt_set_descriptor( vector, isr_stub_table[vector], PRESENT_TRAP_GATE );
        vectors[vector] = true;
    }

    // Reload the IDTR
    idt_reload( &idtr );
}

void idt_set_descriptor( uint8_t vector, void* isr, uint8_t flags )
{
    idt_entry_t* descriptor = &idt[vector];

    descriptor->isr_low = (uint64_t)isr & 0x0000FFFF;
    descriptor->kernel_cs = GDT_OFFSET_KMODE_CODE_SEG;

    // TODO: implement IST
    descriptor->ist = 0;

    descriptor->attributes = flags;
    descriptor->isr_mid = ( (uint64_t)isr >> 16 ) & 0x0000FFFF;
    descriptor->isr_high = ( (uint64_t)isr >> 32 ) & 0xFFFFFFFF;
    descriptor->reserved = 0;
}

/*** End of File ***/