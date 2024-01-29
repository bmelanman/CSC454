/** @file gdt.c
 *
 * @brief A description of the module's purpose.
 *
 * @author Bryce Melander
 * @date Jan-25-2024
 *
 * @copyright (c) 2024 by Bryce Melander under MIT License. All rights reserved.
 * (See http://opensource.org/licenses/MIT for more details.)
 */

#include "gdt.h"

#include "idt.h"

/* Private Defines and Macros */

#define GDT_NUM_ENTRIES ( 6U )
#define GDT_MAX_LIMIT   ( 0xFFFFFU )

#define NULL_DESC_IDX  ( 0U )
#define KMODE_CODE_IDX ( 1U )
#define KMODE_DATA_IDX ( 2U )
#define UMODE_CODE_IDX ( 3U )
#define UMODE_DATA_IDX ( 4U )
#define TSS_IDX        ( 5U )

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

typedef struct __packed
{
    uint32_t limit;
    uint64_t base;
    uint8_t access_byte;
    uint8_t flags;
} gdt_entry_t;

typedef struct __packed
{
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access_byte;
    uint8_t limit_high : 4;
    uint8_t flags : 4;
    uint8_t base_high;
} gdt_t[GDT_NUM_ENTRIES];

typedef struct __packed
{
    uint16_t limit;
    uint64_t base;
} gdt_ptr_t;

typedef struct __packed
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
static __unused gdt_t gdt[GDT_NUM_ENTRIES];

// GDT pointer
static __unused gdt_ptr_t gdt_ptr;

/* Private Functions */

extern void reload_segments( void );

void encode_gdt_entry( uint8_t *target, gdt_entry_t source )
{
    // Check the limit to make sure that it can be encoded
    if ( source.limit > GDT_MAX_LIMIT )
    {
        OS_ERROR(
            "Limit is '0x%X', GDT cannot encode limits larger than 0x%X!", source.limit,
            GDT_MAX_LIMIT
        );
    }

    // Encode the limit
    target[0] = source.limit & 0xFF;
    target[1] = ( source.limit >> 8 ) & 0xFF;
    target[6] = ( source.limit >> 16 ) & 0x0F;

    // Encode the base
    target[2] = source.base & 0xFF;
    target[3] = ( source.base >> 8 ) & 0xFF;
    target[4] = ( source.base >> 16 ) & 0xFF;
    target[7] = ( source.base >> 24 ) & 0xFF;

    // Encode the access byte
    target[5] = source.access_byte;

    // Encode the flags
    target[6] |= ( source.flags << 4 );
}

void tss_init( void )
{
    // Create the TSS
    tss_t tss = { 0 };

    // Set the TSS's stack pointers
    tss.rsp0 = 0x0;
    tss.rsp1 = 0x0;
    tss.rsp2 = 0x0;

    // Set the TSS's ISTs
    tss.ist1 = 0x0;
    tss.ist2 = 0x0;
    tss.ist3 = 0x0;
    tss.ist4 = 0x0;
    tss.ist5 = 0x0;
    tss.ist6 = 0x0;
    tss.ist7 = 0x0;

    // Set the TSS's I/O map base
    tss.iomap_base = sizeof( tss_t );

    // Create the TSS descriptor
    gdt_entry_t tss_descriptor = { 0 };

    // Set the TSS descriptor's base
    tss_descriptor.base = (uint64_t)&tss;

    // Set the TSS descriptor's limit
    tss_descriptor.limit = sizeof( tss_t );

    // Set the TSS descriptor's access byte
    tss_descriptor.access_byte = TASK_STATE_SEG_ACCESS_BYTE;

    // Set the TSS descriptor's flags
    tss_descriptor.flags = TASK_STATE_SEG_FLAGS;

    // Encode the TSS descriptor
    uint8_t tss_descriptor_encoded[8];
    encode_gdt_entry( tss_descriptor_encoded, tss_descriptor );

    // Load the TSS descriptor
    asm volatile( "ltr %0" : : "m"( tss_descriptor_encoded ) );
}

/* Public Functions */

void gdt_init( void )
{
    // Create the GDT entries
    gdt_entry_t gdt_entries[GDT_NUM_ENTRIES];

    // Create the null descriptor
    gdt_entries[NULL_DESC_IDX].base = 0;
    gdt_entries[NULL_DESC_IDX].limit = 0;
    gdt_entries[NULL_DESC_IDX].access_byte = 0;
    gdt_entries[NULL_DESC_IDX].flags = 0;

    // Create the kernel code segment descriptor
    gdt_entries[KMODE_CODE_IDX].base = 0;
    gdt_entries[KMODE_CODE_IDX].limit = GDT_MAX_LIMIT;
    gdt_entries[KMODE_CODE_IDX].access_byte = KMODE_CODE_SEG_ACCESS_BYTE;
    gdt_entries[KMODE_CODE_IDX].flags = CODE_SEG_FLAGS;

    // Create the kernel data segment descriptor
    gdt_entries[KMODE_DATA_IDX].base = 0;
    gdt_entries[KMODE_DATA_IDX].limit = GDT_MAX_LIMIT;
    gdt_entries[KMODE_DATA_IDX].access_byte = KMODE_DATA_SEG_ACCESS_BYTE;
    gdt_entries[KMODE_DATA_IDX].flags = DATA_SEG_FLAGS;

    //// Create the user code segment descriptor
    // gdt_entries[UMODE_CODE_IDX].base = 0;
    // gdt_entries[UMODE_CODE_IDX].limit = GDT_MAX_LIMIT;
    // gdt_entries[UMODE_CODE_IDX].access_byte = UMODE_CODE_SEG_ACCESS_BYTE;
    // gdt_entries[UMODE_CODE_IDX].flags = CODE_SEG_FLAGS;

    //// Create the user data segment descriptor
    // gdt_entries[UMODE_DATA_IDX].base = 0;
    // gdt_entries[UMODE_DATA_IDX].limit = GDT_MAX_LIMIT;
    // gdt_entries[UMODE_DATA_IDX].access_byte = UMODE_DATA_SEG_ACCESS_BYTE;
    // gdt_entries[UMODE_DATA_IDX].flags = DATA_SEG_FLAGS;

    //// Create the TSS descriptor
    // gdt_entries[TSS_IDX].base = (uint64_t)&tss;
    // gdt_entries[TSS_IDX].limit = TASK_STATE_SEG_LIMIT;
    // gdt_entries[TSS_IDX].access_byte = TASK_STATE_SEG_ACCESS_BYTE;
    // gdt_entries[TSS_IDX].flags = TASK_STATE_SEG_FLAGS;

    // Encode the GDT entries into the table
    for ( uint8_t i = 0; i < GDT_NUM_ENTRIES; i++ )
    {
        encode_gdt_entry( (uint8_t *)( &gdt[i] ), gdt_entries[i] );
    }

    // Set the GDT pointer
    gdt_ptr.limit = (uint16_t)( sizeof( gdt ) - 1 );
    gdt_ptr.base = (uint64_t)&gdt;

    // Load the GDT
    asm volatile( "lgdt %0" : : "m"( gdt_ptr ) );

    // Load the segment registers
    // reload_segments();

    // Setup the TSS
    // tss_init();
}

/*** End of File ***/