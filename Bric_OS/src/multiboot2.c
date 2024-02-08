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

typedef struct multiboot_header mb_header_t;

/* Global Variables */

/* Private Functions */

#define VERIFY_MAGIC( mb_head ) ( ( mb_head ).magic == MULTIBOOT_MAGIC_NUM )
#define VERIFY_CHECKSUM( mb_head )                                                                 \
    ( ( 0x100000000 - ( ( mb_head ).magic + ( mb_head ).architecture + ( mb_head ).header_length ) \
      ) == ( mb_head ).checksum )

#define IS_END_TAG( tag ) ( ( tag ).type == 0 && ( tag ).flags == 0 && ( tag ).size == 8 )

/* Public Functions */

int multiboot_parse_tags( void *tag_ptr )
{
    // Read the multiboot header
    mb_header_t *mb_head = (mb_header_t *)tag_ptr;

    // Verify the magic number
    if ( !VERIFY_MAGIC( *mb_head ) )
    {
        return -1;
    }

    // Verify the checksum
    if ( !VERIFY_CHECKSUM( *mb_head ) )
    {
        return -1;
    }

    // Print the header info
    printk( "Multiboot Header:      \n" );
    printk( "  Magic Number...0x%X    \n", mb_head->magic );
    printk( "  Architecture...0x%X    \n", mb_head->architecture );
    printk( "  Header Length..0x%X    \n", mb_head->header_length );
    printk( "  Checksum.......0x%X    \n", mb_head->checksum );
    printk( "                       \n" );

    return 0;
}

/*** End of File ***/