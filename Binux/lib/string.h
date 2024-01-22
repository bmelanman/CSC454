/** @file string.h
 *
 * @brief A description of the module's purpose.
 *
 * @author Bryce Melander
 * @date Jan-08-2024
 *
 * @copyright (c) 2024 by Bryce Melander under MIT License. All rights reserved.
 * (See http://opensource.org/licenses/MIT for more details.)
 */

#ifndef STRING_H
# define STRING_H

/* Includes */

# include <stddef.h>
# include <stdint.h>

/* Defines */

/* Macros */

# define MAX_STR_LEN ( 65536U )  // 64 KiB

/* Typedefs */

/* Public Functions */

void *memset( void *dst, int c, size_t n );

void *memcpy( void *dest, const void *src, size_t n );

size_t strlen( const char *s );

char *strcpy( char *dest, const char *src );

int strcmp( const char *s1, const char *s2 );

const char *strchr( const char *s, int c );

// char *strdup( const char *s );

char *strcat( char *dest, const char *src );

size_t strnlen( const char *s, size_t maxlen );

char *strncpy( char *dest, const char *src, size_t n );

int strncmp( const char *s1, const char *s2, size_t n );

// char *strndup( const char *s, size_t n );

char *strncat( char *dest, const char *src, size_t n );

#endif /* STRING_H */

/*** End of File ***/