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

#include "common.h"
#include "vga_driver.h"

/* Private Defines and Macros */

#define uint   unsigned int
#define llong  long long
#define ullong unsigned llong

#define BASE_8  ( 8U )
#define BASE_10 ( 10U )
#define BASE_16 ( 16U )

typedef enum {
    LOWERCASE = 0U,
    UPPERCASE = 32U  // 'a' - 'A' = 32
} char_case_t;

/* Private Functions */

#define print_str( s ) vga_display_str( s )

void format_char( char *str, char c )
{
    str[0] = c;
    str[1] = '\0';
}

void format_str( char *str, char *s )
{
    size_t len = strlen( str );

    if ( len == MAX_STR_LEN - 1 )
    {
        OS_ERROR( "String buffer overflow!\n" );
        return;
    }

    strncat( str, s, strlen( s ) );
}

void format_llu_base_n( char *str, ullong n, uint8_t base, uint8_t text_case )
{
    size_t len = strlen( str );

    if ( len == MAX_STR_LEN - 1 )
    {
        OS_ERROR( "String buffer overflow!\n" );
        return;
    }

    uint8_t digit_d = (uint8_t)( n % base );
    char digit_c;

    if ( digit_d < 10 )
    {
        digit_c = (char)( digit_d + '0' );
    }
    else
    {
        digit_c = (char)( digit_d - 10 + 'A' + text_case );
    }

    str[len] = digit_c;
    str[len + 1] = '\0';

    if ( n / base )
    {
        format_llu_base_n( str, n / base, base, text_case );
    }
}

void format_ll_base_n( char *str, llong n, uint8_t base, uint8_t text_case )
{
    if ( n < 0 )
    {
        str[0] = '-';
        format_llu_base_n( str + 1, (ullong)( -n ), base, text_case );
    }
    else
    {
        format_llu_base_n( str, (ullong)n, base, text_case );
    }
}

#define format_ll( str, n )  format_ll_base_n( str, n, BASE_10, LOWERCASE )
#define format_llu( str, n ) format_llu_base_n( str, n, BASE_10, LOWERCASE )

#define format_hex( str, n, text_case ) format_llu_base_n( str, n, BASE_16, text_case )
#define format_oct( str, n )            format_llu_base_n( str, n, BASE_8, LOWERCASE )

#define parse_specifier( str, sp, args, type )                                           \
    do                                                                                   \
    {                                                                                    \
        switch ( sp )                                                                    \
        {                                                                                \
            case 'd':                                                                    \
            case 'i':                                                                    \
                format_ll( str, (llong)va_arg( args, type ) );                           \
                break;                                                                   \
                                                                                         \
            case 'u':                                                                    \
                format_llu( str, (ullong)va_arg( args, unsigned type ) ); /* NOLINT */   \
                break;                                                                   \
                                                                                         \
            case 'x':                                                                    \
                format_hex(                                                              \
                    str, (uint64_t)va_arg( args, unsigned type ), LOWERCASE /* NOLINT */ \
                );                                                                       \
                break;                                                                   \
                                                                                         \
            case 'X':                                                                    \
                format_hex(                                                              \
                    str, (uint64_t)va_arg( args, unsigned type ), UPPERCASE /* NOLINT */ \
                );                                                                       \
                break;                                                                   \
                                                                                         \
            case 'o':                                                                    \
                format_oct( str, (uint64_t)va_arg( args, unsigned type ) ); /* NOLINT */ \
                break;                                                                   \
                                                                                         \
            default:                                                                     \
                format_char( str, '%' );                                                 \
                format_char( str, sp );                                                  \
                break;                                                                   \
        }                                                                                \
    } while ( 0 )

/* Public Functions */

__attribute__( ( format( printf, 1, 2 ) ) ) int printk( const char *fmt, ... )
{
    va_list args;
    va_start( args, fmt );

    size_t str_idx, len = strlen( fmt );

    char str[MAX_STR_LEN] = { 0 };

    for ( str_idx = 0; str_idx < len; str_idx++ )
    {
        switch ( fmt[str_idx] )
        {
            case '%':
                switch ( fmt[str_idx + 1] )
                {
                    case '%':
                        format_char( str, '%' );
                        break;

                    case 'c':  // Character
                        format_char( str, (char)va_arg( args, uint ) );
                        break;

                    case 's':  // String
                        format_str( str, va_arg( args, char * ) );
                        break;

                    case 'p':  // Pointer
                        format_str( str, "0x" );
                        format_hex( str, (uint64_t)va_arg( args, void * ), LOWERCASE );
                        break;

                    case 'l':  // Long
                        switch ( fmt[str_idx + 2] )
                        {
                            case 'l':
                                parse_specifier( str, fmt[str_idx + 3], args, llong );
                                str_idx++;
                                break;

                            default:
                                parse_specifier( str, fmt[str_idx + 2], args, long );
                                break;
                        }
                        str_idx++;
                        break;

                    case 'q':  // Long Long NOLINT
                        parse_specifier( str, fmt[str_idx + 2], args, llong );
                        str_idx++;
                        break;

                    case 'h':  // Short
                        parse_specifier( str, fmt[str_idx + 2], args, int );
                        str_idx++;
                        break;

                    default:
                        parse_specifier( str, fmt[str_idx + 1], args, int );
                        break;
                }

                str_idx++;
                break;

            default:
                format_char( str, fmt[str_idx] );
                break;
        }

        if ( strlen( str ) == MAX_STR_LEN - 1 )
        {
            OS_ERROR( "String buffer overflow!\n" );

            va_end( args );
            return 1;
        }
    }

    print_str( str );

    va_end( args );

    return 0;
}

/*** End of File ***/