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

# include "common.h"

/* Defines */

# define PIC1_OFFSET ( 0x20U )  // Controller PIC interrupt offset

# define IRQ32_TIMER    ( PIC1_OFFSET + 0x0U )  // 0x20 - Timer interrupt
# define IRQ33_KEYBOARD ( PIC1_OFFSET + 0x1U )  // 0x21 - Keyboard interrupt
# define IRQ34_CASCADE  ( PIC1_OFFSET + 0x2U )  // 0x22 - Cascade
# define IRQ35_COM2     ( PIC1_OFFSET + 0x3U )  // 0x23 - COM2 (if enabled)
# define IRQ36_COM1     ( PIC1_OFFSET + 0x4U )  // 0x24 - COM1 (if enabled)
# define IRQ37_LPT2     ( PIC1_OFFSET + 0x5U )  // 0x25 - LPT2 (if enabled)
# define IRQ38_FLOPPY   ( PIC1_OFFSET + 0x6U )  // 0x26 - Floppy Disk
# define IRQ39_LPT1     ( PIC1_OFFSET + 0x7U )  // 0x27 - LPT1 / Unreliable "spurious" interrupt

# define PIC2_OFFSET ( PIC1_OFFSET + 0x8U )  // Peripheral PIC interrupt offset

# define PIC1_MIN_IRQ IRQ32_TIMER  // Minimum IRQ for controller PIC
# define PIC1_MAX_IRQ IRQ39_LPT1   // Maximum IRQ for controller PIC

# define IRQ40_CMOS ( PIC2_OFFSET + 0x0U )  // 0x28 - CMOS real-time clock (if enabled)
# define IRQ41_PERF ( PIC2_OFFSET + 0x1U )  // 0x29 - Free for peripherals / legacy SCSI / NIC
# define IRQ42_PERF ( PIC2_OFFSET + 0x2U )  // 0x2A - Free for peripherals / SCSI / NIC
# define IRQ43_PERF ( PIC2_OFFSET + 0x3U )  // 0x2B - Free for peripherals / SCSI / NIC
# define IRQ44_PERF ( PIC2_OFFSET + 0x4U )  // 0x2C - Free for peripherals / SCSI / NIC
# define IRQ45_PERF ( PIC2_OFFSET + 0x5U )  // 0x2D - Free for peripherals / SCSI / NIC
# define IRQ46_PERF ( PIC2_OFFSET + 0x6U )  // 0x2E - Free for peripherals / SCSI / NIC
# define IRQ47_PERF ( PIC2_OFFSET + 0x7U )  // 0x2F - Free for peripherals / SCSI / NIC

# define PIC2_MIN_IRQ IRQ40_CMOS  // Minimum IRQ for peripheral PIC
# define PIC2_MAX_IRQ IRQ47_PERF  // Maximum IRQ for peripheral PIC

/* Defines */

# define PIC1         ( 0x20U )  // IO base address for controller PIC
# define PIC1_COMMAND PIC1
# define PIC1_DATA    ( PIC1 + 1 )

# define PIC2         ( 0xA0U )  // IO base address for peripheral PIC
# define PIC2_COMMAND PIC2
# define PIC2_DATA    ( PIC2 + 1 )

/* Macros */

# define IS_PIC_IRQ( irq ) ( ( ( irq ) >= PIC1_MIN_IRQ ) && ( ( irq ) <= PIC2_MAX_IRQ ) )

/* Typedefs */

/* Public Functions */

void PIC_init( void );

void PIC_remap( int offset1, int offset2 );

int PIC_set_mask( uint16_t irq );

int PIC_clear_mask( uint16_t irq );

int PIC_get_mask( uint16_t irq );

void PIC_disable( void );

void PIC_send_EOI( unsigned int irq );

#endif /* PIC_H */

/*** End of File ***/