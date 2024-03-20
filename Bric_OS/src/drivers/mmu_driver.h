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
    MMU_VADDR_PHYS = 0,
    MMU_VADDR_KHEAP,
    MMU_VADDR_RES,
    MMU_VADDR_IST1,
    MMU_VADDR_IST2,
    MMU_VADDR_IST3,
    MMU_VADDR_IST4,
    MMU_VADDR_KSTACK,
    MMU_VADDR_USTACK,
    MMU_VADDR_UHEAP,
    MMU_VADDR_MAX
} virt_addr_t;

/* Public Functions */

driver_status_t MMU_init( void *tag_ptr );

// Physical Address Functions
void *MMU_pf_alloc( void );
void MMU_pf_free( void *pf );

// Virtual Address Functions
void *MMU_alloc_page( virt_addr_t region );
void *MMU_alloc_pages( uint64_t num_pages, virt_addr_t region );
void MMU_free_page( void *page );

// Heap Functions
void *kbrk( uint64_t increment );
void *sbrk( uint64_t increment );

#endif /* MMU_DRIVER_H */

/*** End of File ***/