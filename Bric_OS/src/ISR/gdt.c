/** @file gdt.c
 *
 * @brief Global Descriptor Table Implementation.
 *
 * @author Bryce Melander
 * @date Jan-25-2024
 *
 * @copyright (c) 2024 by Bryce Melander under MIT License. All rights reserved.
 * (See http://opensource->org/licenses/MIT for more details.)
 */

#include "gdt.h"

#include "idt.h"

/* Private Defines and Macros */

#define GDT_NUM_ENTRIES ( 6U + 1U )  // 6 entries, plus an extra entry for the TSS

#define NULL_DESC_IDX  ( 0U )
#define KMODE_CODE_IDX ( 1U )
#define KMODE_DATA_IDX ( 2U )
#define UMODE_CODE_IDX ( 3U )
#define UMODE_DATA_IDX ( 4U )
#define TSS_IDX        ( 5U )

#define GDT_ENTRY_LIMIT ( 0xFFFFFU )

#define CODE_SEG_FLAGS ( 0x0AU )
#define DATA_SEG_FLAGS ( 0x0CU )

#define KMODE_CODE_SEG_ACCESS_BYTE ( 0x9AU )
#define KMODE_DATA_SEG_ACCESS_BYTE ( 0x92U )

#define UMODE_CODE_SEG_ACCESS_BYTE ( 0xFAU )
#define UMODE_DATA_SEG_ACCESS_BYTE ( 0xF2U )

#define TASK_STATE_SEG_LIMIT       ( sizeof( tss_t ) - 1 )
#define TASK_STATE_SEG_ACCESS_BYTE ( 0x89U )
#define TASK_STATE_SEG_FLAGS       ( 0x00U )

/* Typedefs */

typedef struct
{
    uint64_t base;
    uint32_t limit;
    uint8_t access_byte;
    uint8_t flags;
} gdt_entry_t;

typedef struct
{
    uint8_t bytes[8];
} gdt_t;

typedef struct __packed
{
    uint16_t limit;
    uint64_t base;
} gdt_ptr_t;

typedef struct
{
    uint32_t reserved0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved1;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iomap_base;
} tss_t;

/* Global Variables */

// Table of GDT entries
static gdt_t gdt[GDT_NUM_ENTRIES];
// Pointer to the GDT
static gdt_ptr_t gdt_ptr =
    { .limit = (uint16_t)( sizeof( gdt ) ),  //
      .base = (uint64_t)&gdt }               //
;

// TSS for the kernel
static tss_t tss;

/* Private Functions */

void encode_gdt_entry( gdt_t *target, gdt_entry_t source )
{
    // Lower 16 bits of the limit
    target->bytes[0] = source.limit & 0xFFU;
    target->bytes[1] = ( source.limit >> 8 ) & 0xFFU;

    // Lower 24 bits of the base
    target->bytes[2] = source.base & 0xFFU;
    target->bytes[3] = ( source.base >> 8 ) & 0xFFU;
    target->bytes[4] = ( source.base >> 16 ) & 0xFFU;

    // Upper 8 bits of the base
    target->bytes[5] = source.access_byte;

    // Upper 4 bits of the limit OR'd with the flags
    target->bytes[6] = ( ( source.limit >> 16 ) & 0x0FU ) | ( ( source.flags << 4 ) & 0xF0U );

    // Upper 8 bits of the base
    target->bytes[7] = ( source.base >> 24 ) & 0xFFU;
}

void tss_init( void )
{
    // Setup the stack pointers
    tss.rsp0 = 0x0;
    tss.rsp1 = 0x0;
    tss.rsp2 = 0x0;

    // Setup the ISTs
    // tss.ist1 = (uint64_t)ist1; // TODO: Implement ISTs
    // tss.ist2 = (uint64_t)ist2;
    // tss.ist3 = (uint64_t)ist3;
    tss.ist4 = 0x0;
    tss.ist5 = 0x0;
    tss.ist6 = 0x0;
    tss.ist7 = 0x0;

    // Setup the I/O map base
    tss.iomap_base = 0;

    // Setup the first 8 bytes of the TSS descriptor entry
    gdt_entry_t tss_entry = { 0 };
    tss_entry.base = (uint64_t)&tss;
    tss_entry.limit = sizeof( tss_t ) - 1;
    tss_entry.access_byte = TASK_STATE_SEG_ACCESS_BYTE;
    tss_entry.flags = TASK_STATE_SEG_FLAGS;
    encode_gdt_entry( &gdt[TSS_IDX], tss_entry );

    // Second 8 bytes
    uint32_t base_very_high = ( ( tss_entry.base >> 32 ) & 0xFFFFFFFF );  // Bits 32 - 63
    memcpy( &gdt[TSS_IDX + 1], &base_very_high, 4 );
}

/* Public Functions */

void gdt_init( void )
{
    gdt_entry_t gdt_entries = { 0 };

    // Null descriptor (0)
    gdt_entries.base = 0;
    gdt_entries.limit = 0;
    gdt_entries.access_byte = 0;
    gdt_entries.flags = 0;
    encode_gdt_entry( &gdt[NULL_DESC_IDX], gdt_entries );

    // Kernel code segment descriptor (1)
    gdt_entries.base = 0;
    gdt_entries.limit = GDT_ENTRY_LIMIT;
    gdt_entries.access_byte = KMODE_CODE_SEG_ACCESS_BYTE;
    gdt_entries.flags = CODE_SEG_FLAGS;
    encode_gdt_entry( &gdt[KMODE_CODE_IDX], gdt_entries );

    // Kernel data segment descriptor (2)
    gdt_entries.base = 0;
    gdt_entries.limit = GDT_ENTRY_LIMIT;
    gdt_entries.access_byte = KMODE_DATA_SEG_ACCESS_BYTE;
    gdt_entries.flags = DATA_SEG_FLAGS;
    encode_gdt_entry( &gdt[KMODE_DATA_IDX], gdt_entries );

    // User code segment descriptor (3)
    gdt_entries.base = 0;
    gdt_entries.limit = GDT_ENTRY_LIMIT;
    gdt_entries.access_byte = UMODE_CODE_SEG_ACCESS_BYTE;
    gdt_entries.flags = CODE_SEG_FLAGS;
    encode_gdt_entry( &gdt[UMODE_CODE_IDX], gdt_entries );

    // User data segment descriptor (4)
    gdt_entries.base = 0;
    gdt_entries.limit = GDT_ENTRY_LIMIT;
    gdt_entries.access_byte = UMODE_DATA_SEG_ACCESS_BYTE;
    gdt_entries.flags = DATA_SEG_FLAGS;
    encode_gdt_entry( &gdt[UMODE_DATA_IDX], gdt_entries );

    // Load the TSS
    tss_init();

    // Load the GDT
    asm volatile( "lgdt %0" : : "m"( gdt_ptr ) );

    // Reload the segment registers
    reload_segments();

    // Load the TSS descriptor
    asm volatile( "ltr %0" : : "r"( GDT_OFFSET_TSS ) );
}

/*** End of File ***/