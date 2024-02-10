/** @file multiboot2.h
 *
 * @brief Multiboot2 header and tag definitions.
 *
 * @author Free Software Foundation, with modifications by Bryce Melander
 * @date Feb-01-2024
 *
 * @copyright (c) 2024 by Bryce Melander under MIT License. All rights reserved.
 * (See http://opensource.org/licenses/MIT for more details.)
 */

#ifndef MULTIBOOT2_H
# define MULTIBOOT2_H

# include "common.h"

# define __multiboot __attribute__( ( section( ".multiboot" ) ) )

/* Public Defines and Macros */

# define MULTIBOOT2_BOOTLOADER_MAGIC ( 0x36D76289U )

# define MULTIBOOT_MEMORY_AVAILABLE        ( 1U )
# define MULTIBOOT_MEMORY_RESERVED         ( 2U )
# define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE ( 3U )
# define MULTIBOOT_MEMORY_NVS              ( 4U )
# define MULTIBOOT_MEMORY_BADRAM           ( 5U )

# define MEM_IS_AVAILABLE( entry ) ( ( entry ).type == MULTIBOOT_MEMORY_AVAILABLE )

/* Public Types and Structs */

typedef struct multiboot_tag mb_tag_t;

typedef struct mmap_entry_s
{
    uint64_t addr;
    uint64_t len;
    uint32_t type;
    uint32_t zero;
} mb_mmap_entry_t;

/* Public Functions */

int parse_multiboot2( unsigned long magic, unsigned long addr );

void parse_ELF( mb_tag_t *tag );

void get_multiboot2_mmap_info( void *tag, mb_mmap_entry_t **mmap_entreies, uint32_t *num_entries );

#endif /* MULTIBOOT2_H */

/*** End of File ***/