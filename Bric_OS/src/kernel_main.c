/** @file kernel_main.c
 *
 * @brief Main kernel implementation.
 *
 * @author Bryce Melander
 * @date Jan-08-2024
 *
 * @copyright (c) 2024 by Bryce Melander under MIT License. All rights reserved.
 * (See http://opensource.org/licenses/MIT for more details.)
 */

#include "common.h"
#include "irq_handler.h"
#include "multiboot2.h"
#include "ps2_keyboard_driver.h"
#include "serial_io_driver.h"
#include "splash.h"
#include "vga_driver.h"

int system_initialization( void );

int parse_multiboot2( unsigned long magic, unsigned long addr );

int kernel_main( unsigned long magic, unsigned long addr )
{
    // Run initialization
    if ( system_initialization() )
    {
        OS_ERROR( "System initialization failed!\n" );
        HLT();
    }

    if ( parse_multiboot2( magic, addr ) )
    {
        OS_ERROR( "Multiboot2 parsing failed!\n" );
        HLT();
    }

    HLT();

    return 0;
}

int VGA_init( void )
{
    // Initialize the VGA driver
    if ( VGA_driver_init() == FAILURE )
    {
        OS_ERROR( "VGA driver initialization failed!\n" );
        return 1;
    }

    VGA_clear();

    OS_INFO( "VGA driver initialization is complete!\n" );

    return 0;
}

int SER_init( void )
{
    // Initialize the serial driver
    if ( serial_driver_init() == FAILURE )
    {
        OS_ERROR( "Serial driver initialization failed!\n" );
        return 1;
    }

    OS_INFO( "Serial driver initialization is complete!\n" );

    return 0;
}

int ISR_init( void )
{
    // Initialize the ISR
    if ( IRQ_init() == FAILURE )
    {
        OS_ERROR( "ISR initialization failed!\n" );
        return 1;
    }

    OS_INFO( "ISR initialization is complete!\n" );

    return 0;
}

int KB_init( void )
{
    // Initialize the keyboard driver
    if ( ps2_keyboard_driver_init( true ) == FAILURE )
    {
        OS_ERROR( "Keyboard driver initialization failed!\n" );
        return 1;
    }

    OS_INFO( "Keyboard driver initialization is complete!\n" );

    return 0;
}

int system_initialization( void )
{
    if ( VGA_init() ) return 1;

    if ( SER_init() ) return 1;

    if ( ISR_init() ) return 1;

    if ( KB_init() ) return 1;

    OS_INFO( "System initialization is complete!\n" );

    // Print the splash screen
    splash_screen();

    // Enable interrupts
    IRQ_enable();

    printk( "\n" );

    return 0;
}

/*  Check if MAGIC is valid and print the Multiboot information structure
   pointed by ADDR. */
int parse_multiboot2( unsigned long magic, unsigned long addr )
{
    struct multiboot_tag *tag;
    unsigned size;

    /* Mask the bottom 32 bits of both inputs */
    magic &= 0xFFFFFFFF;
    addr &= 0xFFFFFFFF;

    /*  Am I booted by a Multiboot-compliant boot loader? */
    if ( magic != MULTIBOOT2_BOOTLOADER_MAGIC )
    {
        printk( "Invalid magic number: %lX\n", magic );
        return 1;
    }

    // Check if the address is valid
    if ( addr < 0x100000 )
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

    size = *(unsigned *)addr;
    printk( "Announced mbi size 0x%X\n", size );

    for ( tag = (struct multiboot_tag *)( addr + 8 ); tag->type != MULTIBOOT_TAG_TYPE_END;
          tag = (struct multiboot_tag *)( (multiboot_uint8_t *)tag + ( ( tag->size + 7 ) & ~7 ) ) )
    {
        io_wait_n( 1e6 );

        switch ( tag->type )
        {
            case MULTIBOOT_TAG_TYPE_CMDLINE:
                printk( "Command Line Tag:  0x%X, Size 0x%X\n", tag->type, tag->size );
                printk( "  line = %s\n", ( (struct multiboot_tag_string *)tag )->string );
                break;
            case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:
                printk( "Bootloader Tag:    0x%X, Size 0x%X\n", tag->type, tag->size );
                printk( "  name = %s\n", ( (struct multiboot_tag_string *)tag )->string );
                break;
            case MULTIBOOT_TAG_TYPE_MODULE:
                printk( "Module Tag:        0x%X, Size 0x%X\n", tag->type, tag->size );
                printk(
                    "Module at 0x%X-0x%X. Command line %s\n",
                    ( (struct multiboot_tag_module *)tag )->mod_start,
                    ( (struct multiboot_tag_module *)tag )->mod_end,
                    ( (struct multiboot_tag_module *)tag )->cmdline
                );
                break;
            case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
                printk( "Memory Info Tag:   0x%X, Size 0x%X\n", tag->type, tag->size );
                printk(
                    "  mem_lower = %uKB\n"
                    "  mem_upper = %uKB\n",
                    ( (struct multiboot_tag_basic_meminfo *)tag )->mem_lower,
                    ( (struct multiboot_tag_basic_meminfo *)tag )->mem_upper
                );
                break;
            case MULTIBOOT_TAG_TYPE_BOOTDEV:
                printk( "Boot Device Tag:   0x%X, Size 0x%X\n", tag->type, tag->size );
                printk(
                    "  device = 0x%X,%u,%u\n", ( (struct multiboot_tag_bootdev *)tag )->biosdev,
                    ( (struct multiboot_tag_bootdev *)tag )->slice,
                    ( (struct multiboot_tag_bootdev *)tag )->part
                );
                break;
            case MULTIBOOT_TAG_TYPE_MMAP: {
                multiboot_memory_map_t *mmap;

                printk( "Memory Map Tag:    0x%X, Size 0x%X\n", tag->type, tag->size );

                for ( mmap = ( (struct multiboot_tag_mmap *)tag )->entries;
                      (multiboot_uint8_t *)mmap < (multiboot_uint8_t *)tag + tag->size;
                      mmap = (multiboot_memory_map_t *)( (unsigned long)mmap +
                                                         ( (struct multiboot_tag_mmap *)tag )
                                                             ->entry_size ) )
                    printk(
                        "  base_addr .. 0x%X%X  \n"
                        "  length ..... 0x%X%X  \n"
                        "  type ....... 0x%X    \n"
                        "  \n",
                        (unsigned)( mmap->addr >> 32 ), (unsigned)( mmap->addr & 0xffffffff ),
                        (unsigned)( mmap->len >> 32 ), (unsigned)( mmap->len & 0xffffffff ),
                        (unsigned)mmap->type
                    );
            }
            break;

            case MULTIBOOT_TAG_TYPE_FRAMEBUFFER:
                printk( "Frame Buffer Tag:  0x%X, Size 0x%X\n", tag->type, tag->size );
                break;

            default:
                printk( "Unknown Tag:       0x%X, Size 0x%X\n", tag->type, tag->size );
                break;
        }
    }

    tag = (struct multiboot_tag *)( (multiboot_uint8_t *)tag + ( ( tag->size + 7 ) & ~7 ) );
    printk( "Total mbi size %lX\n", (uintptr_t)( tag - addr ) );

    return 0;
}

/*** End of File ***/