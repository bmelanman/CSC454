/** @file string.c
 *
 * @brief String manipulation functions.
 *
 * @author Bryce Melander
 * @date Jan-08-2024
 *
 * @copyright (c) 2024 by Bryce Melander under MIT License. All rights reserved.
 * (See http://opensource.org/licenses/MIT for more details.)
 */

#include "string.h"

/* Private Defines and Macros */

#define uchar unsigned char

/* Public Functions */

void *memset( void *dst, int c, size_t n )
{
    if ( n != 0 )
    {
        uchar *d = dst;

        do
        {
            *d++ = (uchar)c;
        } while ( --n != 0 );
    }

    return dst;
}

void *memcpy( void *dest, const void *src, size_t n )
{
    if ( n != 0 )
    {
        uchar *d = dest;
        const uchar *s = src;

        while ( n-- ) *d++ = *s++;
    }

    return dest;
}

size_t strnlen( const char *s, size_t maxlen )
{
    if ( s == NULL || maxlen == 0 || *s == '\0' )
    {
        return 0;
    }

    size_t len = 0;

    while ( *s++ != '\0' && ++len < maxlen )
        ;

    return len;
}

size_t strlen( const char *s ) { return strnlen( s, MAX_STR_LEN ); }

char *strncat( char *dest, const char *src, size_t n )
{
    if ( dest == NULL || src == NULL )
    {
        return NULL;
    }

    if ( *src == '\0' || n == 0 )
    {
        return dest;
    }

    char *dest_end = dest + strlen( dest );

    strncpy( dest_end, src, n );

    return dest;
}

char *strcat( char *dest, const char *src )
{
    if ( dest == NULL || src == NULL )
    {
        return NULL;
    }

    if ( *src == '\0' )
    {
        return dest;
    }

    size_t len = strlen( dest );

    strncpy( dest + len, src, strlen( src ) );

    return dest;
}

int strncmp( const char *s1, const char *s2, size_t n )
{
    if ( s1 == NULL || s2 == NULL )
    {
        return -1;
    }

    if ( n == 0 )
    {
        return 0;
    }

    uchar c1, c2;

    do
    {
        c1 = *s1++;
        c2 = *s2++;

        if ( c1 == '\0' )
        {
            return c1 - c2;
        }

    } while ( c1 == c2 && --n != 0 );

    return c1 - c2;
}

int strcmp( const char *s1, const char *s2 )
{
    if ( s1 == NULL || s2 == NULL )
    {
        return -1;
    }

    size_t len1 = strlen( s1 );
    size_t len2 = strlen( s2 );

    if ( len1 > len2 )
    {
        return strncmp( s1, s2, len1 );
    }

    return strncmp( s1, s2, len2 );
}

char *strncpy( char *dest, const char *src, size_t n )
{
    if ( dest == NULL || src == NULL )
    {
        return NULL;
    }

    if ( *src == '\0' || n == 0 )
    {
        return dest;
    }

    size_t i = 0;

    do
    {
        dest[i] = *src++;
    } while ( *src != '\0' && ++i < n );

    return dest;
}

char *strcpy( char *dest, const char *src )
{
    if ( dest == NULL || src == NULL )
    {
        return NULL;
    }

    if ( *src == '\0' )
    {
        return dest;
    }

    strncpy( dest, src, strlen( src ) );

    return dest;
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

// char *strdup( const char *s ) { return strndup( s, MAX_STR_LEN ); }

const char *strchr( const char *s, int c )
{
    do
    {
        // Check if we found the character before checking if we reached the end of the string,
        // just in case the character is '\0'
        if ( *s == c )
        {
            return s;
        }

    } while ( *s++ != '\0' );

    return NULL;
}

/*** End of File ***/