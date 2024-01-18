/** @file string.c
 *
 * @brief A description of the module's purpose.
 *
 * @author Bryce Melander
 * @date Jan-08-2024
 *
 * @copyright (c) 2024 by Bryce Melander under MIT License. All rights reserved.
 * (See http://opensource.org/licenses/MIT for more details.)
 */

#include "string.h"

/* Private Defines and Macros */

#define MAX_STR_LEN ( 65536U )  // 64 KiB

/* Global Variables */

/* Private Functions */

/* Public Functions */

void *memset( void *dst, int c, size_t n )
{
    size_t i;

    for ( i = 0; i < n; i++ )
    {
        ( (int *)dst )[i] = c;
    }

    return dst;
}

void *memcpy( void *dest, const void *src, size_t n )
{
    size_t i;

    for ( i = 0; i < n; i++ )
    {
        ( (int *)dest )[i] = ( (int *)src )[i];
    }

    return dest;
}

size_t strnlen( const char *s, size_t maxlen )
{
    size_t len = 0;

    while ( s[len] != '\0' && len < maxlen )
    {
        len++;
    }

    return len;
}

char *strncpy( char *dest, const char *src, size_t n )
{
    memcpy( dest, src, n );

    return dest;
}

int strncmp( const char *s1, const char *s2, size_t n )
{
    size_t i;

    for ( i = 0; i < n; i++ )
    {
        if ( s1[i] != s2[i] )
        {
            return s1[i] - s2[i];
        }
    }

    return 0;
}

/*
char *strndup( const char *s, size_t n )
{
    char dup[n + 1];

    strncpy( dup, s, n );

    dup[n] = '\0';

    return dup;
}
*/

const char *strchr( const char *s, int c )
{
    while ( *s != '\0' )
    {
        if ( *s == c )
        {
            return s;
        }

        s++;
    }

    return NULL;
}

char *strncat( char *dest, const char *src, size_t n )
{
    size_t i, len = strlen( dest );

    for ( i = 0; i < n; i++ )
    {
        dest[len + i] = src[i];
    }

    return dest;
}

size_t strlen( const char *s ) { return strnlen( s, MAX_STR_LEN ); }

char *strcpy( char *dest, const char *src ) { return strncpy( dest, src, MAX_STR_LEN ); }

int strcmp( const char *s1, const char *s2 ) { return strncmp( s1, s2, MAX_STR_LEN ); }

// char *strdup( const char *s ) { return strndup( s, MAX_STR_LEN ); }

char *strcat( char *dest, const char *src ) { return strncat( dest, src, MAX_STR_LEN ); }

/*** End of File ***/