/** @file tests.h
 *
 * @brief Header for kernel test functions.
 *
 * @author Bryce Melander
 * @date Jan-18-2024
 *
 * @copyright (c) 2024 by Bryce Melander under MIT License. All rights reserved.
 * (See http://opensource.org/licenses/MIT for more details.)
 */

#ifndef TESTS_H
# define TESTS_H

/* Public Functions */

// tests.c
void test_printk( void );

// kmalloc_tests.c
int test_kmalloc( void );
int test_kcalloc( void );
int test_krealloc( void );
int test_kfree( void );
int test_all( void );

#endif /* TESTS_H */

/*** End of File ***/