/** @file printk.c
 *
 * @brief A description of the module's purpose.
 *
 * @author Bryce Melander
 * @date Jan-09-2024
 *
 * @copyright (c) 2024 by Bryce Melander under MIT License. All rights reserved.
 * (See http://opensource.org/licenses/MIT for more details.)
 */

#include "printk.h"

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "vga_driver.h"

/* Private Defines and Macros */

#define uint   unsigned int
#define llong  long long
#define ullong unsigned llong

typedef enum {
    LOWERCASE = 0U,
    UPPERCASE = 32U  // 'a' - 'A' = 32
} char_case_t;

/* Private Functions */

#define print_specifier( sp ) \
    do                        \
    {                         \
        print_char( '%' );    \
        print_char( sp );     \
    } while ( 0 )

#define print_char( c ) VGA_display_char( c )
#define print_str( s )  VGA_display_str( s )

void print_llu( ullong n )
{
    if ( n / 10 )
    {
        print_llu( n / 10 );
    }

    VGA_display_char( (char)( ( n % 10 ) + '0' ) );
}

void print_ll( llong n )
{
    if ( n < 0 )
    {
        VGA_display_char( '-' );
        n = -n;
    }

    print_llu( (ullong)n );
}

void print_hex( uint64_t n, char_case_t char_case )
{
    if ( n / 16 )
    {
        print_hex( n / 16, char_case );
    }

    uint8_t digit = (uint8_t)( n % 16 );

    if ( digit < 10 )
    {
        VGA_display_char( (char)( digit + '0' ) );
    }
    else
    {
        VGA_display_char( (char)( ( digit - 10 ) + ( 'a' - (int)char_case ) ) );
    }
}

void print_oct( uint64_t n )
{
    if ( n / 8 )
    {
        print_oct( n / 8 );
    }

    VGA_display_char( (char)( ( n % 8 ) + '0' ) );
}

#define parse_int( sp, args )   parse_specifier( sp, args, int )
#define parse_long( sp, args )  parse_specifier( sp, args, long )
#define parse_llong( sp, args ) parse_specifier( sp, args, llong )

#define parse_specifier( sp, args, type )                                                     \
    do                                                                                        \
    {                                                                                         \
        switch ( sp )                                                                         \
        {                                                                                     \
            case 'd':                                                                         \
            case 'i':                                                                         \
                print_ll( (llong)va_arg( args, type ) );                                      \
                break;                                                                        \
                                                                                              \
            case 'u':                                                                         \
                print_llu( (ullong)va_arg( args, unsigned type ) ); /* NOLINT */              \
                break;                                                                        \
                                                                                              \
            case 'x':                                                                         \
                print_hex( (uint64_t)va_arg( args, unsigned type ), LOWERCASE ); /* NOLINT */ \
                break;                                                                        \
                                                                                              \
            case 'X':                                                                         \
                print_hex( (uint64_t)va_arg( args, unsigned type ), UPPERCASE ); /* NOLINT */ \
                break;                                                                        \
                                                                                              \
            case 'o':                                                                         \
                print_oct( (uint64_t)va_arg( args, unsigned type ) ); /* NOLINT */            \
                break;                                                                        \
                                                                                              \
            default:                                                                          \
                print_specifier( sp );                                                        \
                break;                                                                        \
        }                                                                                     \
    } while ( 0 )

/* Public Functions */

__attribute__( ( format( printf, 1, 2 ) ) ) int printk( const char *fmt, ... )
{
    va_list args;
    va_start( args, fmt );

    size_t i, len = strlen( fmt );

    for ( i = 0; i < len; i++ )
    {
        switch ( fmt[i] )
        {
            case '%':
                switch ( fmt[i + 1] )
                {
                    case '%':
                        print_char( '%' );
                        break;

                    case 'c':  // Character
                        print_char( (char)va_arg( args, uint ) );
                        break;

                    case 's':  // String
                        print_str( va_arg( args, char * ) );
                        break;

                    case 'p':  // Pointer
                        print_str( "0x" );
                        print_hex( (uint64_t)va_arg( args, void * ), LOWERCASE );
                        break;

                    case 'l':  // Long
                        if ( fmt[i + 2] == 'l' )
                        {
                            parse_llong( fmt[i + 3], args );
                            i++;
                        }
                        else
                        {
                            parse_long( fmt[i + 2], args );
                        }
                        i++;
                        break;

                    case 'h':  // Short
                    case 'q':  // Long Long
                        parse_llong( fmt[i + 2], args );
                        i++;
                        break;

                    default:
                        parse_int( fmt[i + 1], args );
                        i++;
                        break;
                }

                i++;
                break;

            default:
                print_char( fmt[i] );
                break;
        }
    }

    va_end( args );

    return 0;
}

/*** End of File ***/