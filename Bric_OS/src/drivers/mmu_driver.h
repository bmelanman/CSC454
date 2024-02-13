/** @file mmu_driver.h
 *
 * @brief
 *
 * @author Bryce Melander
 * @date Feb-07-2024
 *
 * @copyright (c) 2024 by Bryce Melander under MIT License. All rights reserved.
 * (See http://opensource.org/licenses/MIT for more details.)
 */

#ifndef MMU_DRIVER_H
# define MMU_DRIVER_H

/* Includes */

# include "common.h"
# include "multiboot2.h"

/* Defines */

# define PAGE_SIZE ( 4096U )  // 4 KB Pages

/* Macros */

/* Typedefs */

typedef enum virt_addr_t {
    VIRT_ADDR_PHYSICAL_MAP = 0,
    VIRT_ADDR_KERNEL_HEAP,
    VIRT_ADDR_RESERVED,
    VIRT_ADDR_IST1,
    VIRT_ADDR_IST2,
    VIRT_ADDR_IST3,
    VIRT_ADDR_IST4,
    VIRT_ADDR_KERNEL_STACK,
    VIRT_ADDR_USER_HEAP,
    VIRT_ADDR_MAX
} virt_addr_t;

/* Public Functions */

driver_status_t MMU_init( void *tag_ptr );

// Physical Address Functions
void *MMU_pf_alloc( void );
void MMU_pf_free( void *pf );

// Virtual Address Functions
void *MMU_alloc_region( virt_addr_t region );
void MMU_free_page( void *page );

#endif /* MMU_DRIVER_H */

/*** End of File ***/