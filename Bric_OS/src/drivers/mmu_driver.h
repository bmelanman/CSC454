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

/* Public Functions */

driver_status_t MMU_init( void *tag_ptr );

void *MMU_pf_alloc( void );

void *MMU_pf_alloc_n( int n );

void MMU_pf_free( void *pf );

void MMU_pf_free_n( void *pf, int n );

#endif /* MMU_DRIVER_H */

/*** End of File ***/