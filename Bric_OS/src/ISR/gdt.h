/** @file gdt.h
 *
 * @brief Header for Global Descriptor Table (GDT) implementation.
 *
 * @author Bryce Melander
 * @date Jan-25-2024
 *
 * @copyright (c) 2024 by Bryce Melander under MIT License. All rights reserved.
 * (See http://opensource.org/licenses/MIT for more details.)
 */

#ifndef GDT_H
# define GDT_H

/* Includes */

# include "common.h"

/* Defines */

# define GDT_OFFSET_NULL_DESC      ( 0x00U )
# define GDT_OFFSET_KMODE_CODE_SEG ( 0x08U )
# define GDT_OFFSET_KMODE_DATA_SEG ( 0x10U )
# define GDT_OFFSET_UMODE_CODE_SEG ( 0x18U )
# define GDT_OFFSET_UMODE_DATA_SEG ( 0x20U )
# define GDT_OFFSET_TSS            ( 0x28U )

/* Enternal Functions and Variables */

extern void reload_segments( void );

extern void* ist1;
extern void* ist2;
extern void* ist3;

/* Global Variables */

/* Public Functions */

void gdt_init( void );

void tss_init( void );

#endif /* GDT_H */

/*** End of File ***/