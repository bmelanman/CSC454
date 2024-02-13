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

/* Private Defines and Macros */

#define MULTIBOOT2_HEADER_MAGIC ( 0xE85250D6U )

#define MULTIBOOT_TAG_TYPE_END          ( 0U )
#define MULTIBOOT_TAG_TYPE_MMAP         ( 6U )
#define MULTIBOOT_TAG_TYPE_ELF_SECTIONS ( 9U )

#define VERIFY_MAGIC( mb_head ) ( ( mb_head ).magic == MULTIBOOT2_HEADER_MAGIC )
#define VERIFY_CHECKSUM( mb_head )                                                                 \
    ( ( 0x100000000 - ( ( mb_head ).magic + ( mb_head ).architecture + ( mb_head ).header_length ) \
      ) == ( mb_head ).checksum )

#define IS_END_TAG( tag ) ( ( tag ).type == 0 && ( tag ).flags == 0 && ( tag ).size == 8 )

/* Private Types and Structs */

typedef struct multiboot_header_s
{
    uint32_t magic;
    uint32_t architecture;
    uint32_t header_length;
    uint32_t checksum;
} mb_header_t;

struct multiboot_tag
{
    uint32_t type;
    uint32_t size;
};

typedef struct mb_mmap_tag_tag_s
{
    uint32_t type;
    uint32_t size;
    uint32_t entry_size;
    uint32_t entry_version;
    mb_mmap_entry_t entries[0];
} mb_mmap_tag_t;

typedef struct mb_elf_sections_tag_s
{
    uint32_t type;
    uint32_t size;
    uint32_t num;
    uint32_t entsize;
    uint32_t shndx;
    char sections[0];
} mb_elf_sections_tag_t;

/* Global Variables */

/* Private Functions */

void parse_mmap( mb_tag_t *tag )
{
    mb_mmap_entry_t *mmap;

    printk( "Memory Map Tag:    0x%X, Size 0x%X\n", tag->type, tag->size );

    for ( mmap = ( (mb_mmap_tag_t *)tag )->entries; (uint8_t *)mmap < (uint8_t *)tag + tag->size;
          mmap = (mb_mmap_entry_t *)( (unsigned long)mmap + ( (mb_mmap_tag_t *)tag )->entry_size ) )
    {
        printk( "  type .......... " );

        switch ( mmap->type )
        {
            case MULTIBOOT_MEMORY_AVAILABLE:
                printk( "RAM (Available)\n" );
                break;

            case MULTIBOOT_MEMORY_ACPI_RECLAIMABLE:
                printk( "ACPI Memory (Reclaimable)\n" );
                break;

            default:
                printk( "Reserved\n" );
                break;
        }

        printk(
            "  base_addr ..... 0x%lX \n"
            "  length ........ 0x%lX \n"
            "  \n",
            mmap->addr, mmap->len
        );
    }
}

void print_elf_type( uint32_t type )
{
    switch ( type )
    {
        case 0:
            printk( "  NULL\n\n" );
            break;
        case 0x1:
            printk( "  PROGBITS: Program data\n" );
            break;
        case 0x2:
            printk( "  SYMTAB: Symbol table\n" );
            break;
        case 0x3:
            printk( "  STRTAB: String table\n" );
            break;
        case 0x4:
            printk( "  RELA: Relocation entries with addends\n" );
            break;
        case 0x5:
            printk( "  HASH: Symbol hash table\n" );
            break;
        case 0x6:
            printk( "  DYNAMIC: Dynamic linking information\n" );
            break;
        case 0x7:
            printk( "  NOTE: Note sections\n" );
            break;
        case 0x8:
            printk( "  NOBITS: BSS\n" );
            break;
        case 0x9:
            printk( "  REL: Relocation entries, no addends\n" );
            break;
        case 0xA:
            printk( "  SHLIB: Reserved\n" );
            break;
        case 0xB:
            printk( "  DYNSYM: Dynamic linker symbol table\n" );
            break;
        case 0xE:
            printk( "  INIT_ARRAY: Array of constructors\n" );
            break;
        case 0xF:
            printk( "  FINI_ARRAY: Array of destructors\n" );
            break;
        case 0x10:
            printk( "  PREINIT_ARRAY: Array of pre-constructors\n" );
            break;
        case 0x11:
            printk( "  GROUP: Section group\n" );
            break;
        case 0x12:
            printk( "  SYMTAB_SHNDX: Extended section indeces\n" );
            break;
        case 0x13:
            printk( "  NUM: Number of defined types\n" );
            break;
        case 0x60000000:
            printk( "  LOOS: Start of OS-specific\n" );
            break;
        default:
            printk( "  UNKNOWN\n" );
            break;
    }
}

void parse_ELF( mb_tag_t *tag )
{
    mb_elf_sections_tag_t *elf = (mb_elf_sections_tag_t *)tag;

    elf_shdr_tbl_t *sect = (elf_shdr_tbl_t *)elf->sections;

    uint32_t i, num_entries = elf->num;

    printk( "ELF Sections Tag: 0x%X, Size 0x%X\n", tag->type, tag->size );
    printk( "  \n" );

    for ( i = 0; i < num_entries; ++i )
    {
        // Only print the section if it has the SHF_ALLOC flag
        if ( sect->flags & 0x2 )
        {
            print_elf_type( sect->type );

            printk(
                "  name ....... 0x%X\n"
                "  type ....... 0x%X\n"
                "  flags ...... 0x%lX\n"
                "  addr ....... 0x%lX\n"
                "  offset ..... 0x%lX\n"
                "  size ....... 0x%lX\n"
                "  link ....... 0x%X\n"
                "  info ....... 0x%X\n"
                "  addralign .. 0x%lX\n"
                "  entsize .... 0x%lX\n"
                "  \n",
                sect->name, sect->type, sect->flags, sect->addr, sect->offset, sect->size,
                sect->link, sect->info, sect->addralign, sect->entsize
            );
        }

        ++sect;
    }
}

/* Public Functions */

/*  Check if MAGIC is valid and print the Multiboot information structure
   pointed by ADDR. */
int parse_multiboot2( unsigned long magic, unsigned long addr )
{
    /* Mask the bottom 32 bits of both inputs */
    magic &= 0xFFFFFFFF;
    addr &= 0xFFFFFFFF;

    // Check bootloader comliance
    if ( magic != MULTIBOOT2_BOOTLOADER_MAGIC )
    {
        printk( "Invalid magic number: %lX\n", magic );
        return 1;
    }

    // Check if the address is valid
    if ( addr == 0 )
    {
        printk( "Invalid mbi: %lX\n", addr );
        return 1;
    }

    // Check if the address is aligned
    if ( addr & 0b111 )
    {
        printk( "Unaligned mbi: %lX\n", addr );
        return 1;
    }

    printk(
        "Multiboot2 Header:     \n"
        "  magic ........ 0x%lX\n"
        "  addr ......... 0x%lX\n"
        "  \n",
        magic, addr
    );

    mb_tag_t *tag;

    for ( tag = (mb_tag_t *)( addr ) + 1; tag->type != MULTIBOOT_TAG_TYPE_END;
          tag = (mb_tag_t *)( (uint8_t *)tag + ( ( tag->size + 7 ) & ~7 ) ) )
    {
        io_wait_n( 0x100000 );

        switch ( tag->type )
        {
            case MULTIBOOT_TAG_TYPE_MMAP:
                parse_mmap( tag );
                break;
            case MULTIBOOT_TAG_TYPE_ELF_SECTIONS:
                parse_ELF( tag );
                break;
            default:
                // Ignore all other tags
                continue;
        }

        printk( "\n" );
    }

    printk( "Done!\n" );

    return 0;
}

mb_tag_t *find_multiboot2_tag( unsigned long addr, uint32_t tag_type )
{
    mb_tag_t *tag, *section_addr = NULL;

    for ( tag = (mb_tag_t *)( addr ) + 1; tag->type != MULTIBOOT_TAG_TYPE_END;
          tag = (mb_tag_t *)( (uint8_t *)tag + ( ( tag->size + 7 ) & ~7 ) ) )
    {
        if ( tag->type == tag_type )
        {
            section_addr = tag;
            break;
        }
    }

    if ( tag->type != tag_type )
    {
        OS_ERROR( "Tag could not be found!\n" );
    }

    return section_addr;
}

void get_multiboot2_mmap_info(
    void *tag_addr, mb_mmap_entry_t **mmap_entreies, uint32_t *num_entries
)
{
    mb_tag_t *tag = find_multiboot2_tag( (unsigned long)tag_addr, MULTIBOOT_TAG_TYPE_MMAP );

    *mmap_entreies = ( (mb_mmap_tag_t *)tag )->entries;
    *num_entries = ( tag->size - sizeof( mb_mmap_tag_t ) ) / ( (mb_mmap_tag_t *)tag )->entry_size;
}

void get_multiboot2_elf_info(
    void *tag_addr, elf_shdr_tbl_t **elf_sections, uint32_t *num_sections
)
{
    mb_tag_t *tag = find_multiboot2_tag( (unsigned long)tag_addr, MULTIBOOT_TAG_TYPE_ELF_SECTIONS );

    *elf_sections = (elf_shdr_tbl_t *)( (mb_elf_sections_tag_t *)tag )->sections;
    *num_sections = ( (mb_elf_sections_tag_t *)tag )->num;
}

/*** End of File ***/