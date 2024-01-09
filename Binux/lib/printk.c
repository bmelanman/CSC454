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

typedef enum {
    LOWERCASE = 0U,
    UPPERCASE = 32U  // 32 = 'a' - 'A'
} char_case_t;

/* Global Variables */

/* Private Functions */

void print_char( char c ) { VGA_display_char( c ); }

void print_str( char *s ) { VGA_display_str( s ); }

void print_uint( unsigned int n )
{
    if ( n / 10 )
    {
        print_uint( n / 10 );
    }

    VGA_display_char( (char)( ( n % 10 ) + '0' ) );
}

void print_int( int n )
{
    if ( n < 0 )
    {
        VGA_display_char( '-' );
        n = -n;
    }

    print_uint( (unsigned int)n );
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

/* Public Functions */

__attribute__( ( format( printf, 1, 2 ) ) ) int printk( const char *fmt, ... )
{
    // TODO: Must implement %h[dux] %l[dux] %(q/ll)[dux]

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

                    case 'd':  // Integer
                    case 'i':
                        print_int( va_arg( args, int ) );
                        break;

                    case 'u':  // Unsigned Integer
                        print_uint( va_arg( args, unsigned int ) );
                        break;

                    case 'c':  // Character
                        print_char( (char)va_arg( args, unsigned int ) );
                        break;

                    case 'x':  // Hexadecimal, Lowercase
                        print_str( "0x" );
                        print_hex( (uint64_t)va_arg( args, unsigned int ), LOWERCASE );
                        break;

                    case 'X':  // Hexadecimal, Uppercase
                        print_str( "0x" );
                        print_hex( (uint64_t)va_arg( args, unsigned int ), UPPERCASE );
                        break;

                    case 'o':  // Octal
                        print_str( "0o" );
                        print_oct( (uint64_t)va_arg( args, unsigned int ) );
                        break;

                    case 's':  // String
                        print_str( va_arg( args, char * ) );
                        break;

                    case 'p':  // Pointer
                        print_str( "0x" );
                        print_hex( (uint64_t)va_arg( args, void * ), LOWERCASE );
                        break;

                    case 'h':  // Short
                        switch ( fmt[i + 2] )
                        {
                            case 'd':
                            case 'i':
                                print_int( (int)va_arg( args, int ) );
                                break;

                            case 'u':
                                print_uint( (uint64_t)va_arg( args, unsigned int ) );
                                break;

                            case 'x':
                                print_str( "0x" );
                                print_hex( (uint64_t)va_arg( args, unsigned int ), LOWERCASE );
                                break;

                            case 'X':
                                print_str( "0x" );
                                print_hex( (uint64_t)va_arg( args, unsigned int ), UPPERCASE );
                                break;

                            case 'o':
                                print_str( "0o" );
                                print_oct( (uint64_t)va_arg( args, unsigned int ) );
                                break;

                            default:
                                print_char( '%' );
                                print_char( 'h' );
                                print_char( fmt[i + 2] );
                                break;
                        }

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