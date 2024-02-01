/** @file pic.h
 *
 * @brief Header for Programmable Interrupt Controller (PIC) implementation.
 *
 * @author Bryce Melander
 * @date Jan-28-2024
 *
 * @copyright (c) 2024 by Bryce Melander under MIT License. All rights reserved.
 * (See http://opensource.org/licenses/MIT for more details.)
 */

#ifndef PIC_H
# define PIC_H

/* Includes */

/* Defines */

# define PIC1         ( 0x20U )  // IO base address for controller PIC
# define PIC1_COMMAND ( PIC1 )
# define PIC1_DATA    ( PIC1 + 1 )

# define PIC2         ( 0xA0U )  // IO base address for peripheral PIC
# define PIC2_COMMAND ( PIC2 )
# define PIC2_DATA    ( PIC2 + 1 )

/* Macros */

/* Typedefs */

/* Public Functions */

void PIC_remap( int offset1, int offset2 );

void PIC_disable( void );

void PIC_sendEOI( unsigned int irq );

#endif /* PIC_H */

/*** End of File ***/