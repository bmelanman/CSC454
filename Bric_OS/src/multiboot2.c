/** @file multiboot2.c
 *
 * @brief Read and parse Multiboot2 headers and tags.
 *
 * @author Bryce Melander
 * @date Feb-01-2024
 *
 * @copyright (c) 2024 by Bryce Melander under MIT License. All rights reserved.
 * (See http://opensource.org/licenses/MIT for more details.)
 */

#include "multiboot2.h"

/* Private Includes */

#include "common.h"

/* Private Defines and Macros */

#define MULTIBOOT_MAGIC_NUM ( 0xE85250D6U )

#define TAG_INFO_SIZE   ( sizeof( mb_tag_t ) )
#define MMAP_TAG_SIZE   ( sizeof( mb_mmap_t ) )
#define MMAP_ENTRY_SIZE ( sizeof( mb_mmap_entry_t ) )

/* Private Types and Enums */

/* Global Variables */

/* Private Functions */

#define VERIFY_MAGIC( mb_head ) ( ( mb_head ).magic == MULTIBOOT_MAGIC_NUM )
#define VERIFY_CHECKSUM( mb_head )                                                                 \
    ( ( 0x100000000 - ( ( mb_head ).magic + ( mb_head ).architecture + ( mb_head ).header_length ) \
      ) == ( mb_head ).checksum )

#define IS_END_TAG( tag ) ( ( tag ).type == 0 && ( tag ).flags == 0 && ( tag ).size == 8 )

/* Public Functions */

int multiboot_parse_tags( struct multiboot_header multiboot_header )
{
    // Verify magic number
    if ( !VERIFY_MAGIC( multiboot_header ) )
    {
        return 1;
    }

    // Verify checksum
    if ( !VERIFY_CHECKSUM( multiboot_header ) )
    {
        return 1;
    }

    // uint32_t i;

    // Print the header fields
    printk( "Multiboot2 Header:     \n" );
    printk( "  Magic.............%x \n", multiboot_header.magic );
    printk( "  Arch..............%u \n", multiboot_header.architecture );
    printk( "  Header Length.....%u \n", multiboot_header.header_length );
    printk( "  Checksum..........%x \n", multiboot_header.checksum );
    printk( "                       \n" );

    // Parse the tags!
    // uint8_t *curr_tag_addr = (uint8_t *)multiboot_header.tags;

    //// First tag is the memory map
    // mb_mmap_t *mmap_tag = (mb_mmap_t *)curr_tag_addr;

    //// Verify fields
    // printk( "Memory Map Tag:        \n" );
    // printk( "  Type..............%u \n", mmap_tag->type );
    // printk( "  Size..............%u \n", mmap_tag->size );
    // printk( "  Entry Size........%u \n", mmap_tag->entry_size );
    // printk( "  Entry Version.....%u \n", mmap_tag->entry_version );
    // printk( "                       \n" );

    //// Calculate the number of entries
    //// uint32_t num_entries = ( mmap_tag->size - TAG_INFO_SIZE ) / mmap_tag->entry_size;

    //// if ( num_entries == 0 )
    ////{
    ////     OS_ERROR( "No memory map entries found!\n" );
    ////     return 1;
    //// }

    //// Print the entries
    // for ( i = 0; i < 0; i++ )
    //{
    //     mb_mmap_entry_t *entry = &mmap_tag->entries[i];

    //    printk( "Memory Map Entry %u:   \n", i );
    //    printk( "  Address...........%u \n", entry->addr );
    //    printk( "  Length............%u \n", entry->len );
    //    printk( "  Type..............%u \n", entry->type );
    //    printk( "  Reserved..........%u \n", entry->reserved );
    //    printk( "                       \n" );
    //}

    return 0;
}

/*** End of File ***/