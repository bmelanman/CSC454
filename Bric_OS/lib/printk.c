/** @file printk.c
 *
 * @brief Kernel print function implementation.
 *
 * @author Bryce Melander
 * @date Jan-09-2024
 *
 * @copyright (c) 2024 by Bryce Melander under MIT License. All rights reserved.
 * (See http://opensource.org/licenses/MIT for more details.)
 */

#include "printk.h"

/* Includes */

#include "common.h"
#include "serial_io_driver.h"
#include "vga_driver.h"

/* Private Defines and Macros */

#define uint   unsigned int
#define llong  long long
#define ullong unsigned llong

#define BASE_8  ( 8U )
#define BASE_10 ( 10U )
#define BASE_16 ( 16U )

typedef enum {
    LOWERCASE = 32U,  // 'a' - 'A' = 32
    UPPERCASE = 0U
} char_case_t;

/* Private Functions */

#define print_VGA( s )       VGA_display_str( s )
#define print_UART( s, len ) serial_write( s, len )

void format_char( char *str, char c ) { strncat( str, &c, 1 ); }

void format_str( char *str, char *s )
{
    if ( strlen( str ) == MAX_STR_LEN - 1 )
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

    // Recursively format the number, starting with the most significant digit
    if ( n / base )
    {
        format_llu_base_n( str, n / base, base, text_case );
    }

    uint8_t digit_d = (uint8_t)( n % base );

    if ( digit_d < 10 )
    {
        format_char( str, (char)( digit_d + '0' ) );
    }
    else
    {
        format_char( str, (char)( digit_d - 10 + 'A' + text_case ) );
    }
}

void format_ll_base_n( char *str, llong n, uint8_t base, uint8_t text_case )
{
    if ( n < 0 )
    {
        format_char( str, '-' );
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

void print_clear_buff( char *buff )
{
    size_t len = strlen( buff );

    // Output to VGA
    print_VGA( buff );

    // Output to UART
    print_UART( buff, len );

    memset( buff, 0, len );
}

/* Public Functions */

__attribute__( ( format( printf, 1, 2 ) ) ) int printk( const char *fmt, ... )
{
    va_list args;
    va_start( args, fmt );

    size_t str_idx, fmt_len = strlen( fmt ), output_len = 1024;

    char output_str[output_len];

    memset( output_str, 0, output_len );

    for ( str_idx = 0; str_idx < fmt_len; ++str_idx )
    {
        // Check if the output string is full
        if ( strlen( output_str ) >= ( output_len - 32 ) )
        {
            print_clear_buff( output_str );
        }

        // Check for format specifier (i.e. '%')
        switch ( fmt[str_idx] )
        {
            // Format specifier
            case '%':
                switch ( fmt[str_idx + 1] )
                {
                    case '%':
                        format_char( output_str, '%' );
                        break;

                    case 'c':  // Character
                        format_char( output_str, (char)va_arg( args, uint ) );
                        break;

                    case 's':  // String
                        format_str( output_str, va_arg( args, char * ) );
                        break;

                    case 'p':  // Pointer
                        format_str( output_str, "0x" );
                        format_hex( output_str, (uint64_t)va_arg( args, void * ), LOWERCASE );
                        break;

                    case 'l':  // Long
                        switch ( fmt[str_idx + 2] )
                        {
                            case 'l':
                                parse_specifier( output_str, fmt[str_idx + 3], args, llong );
                                ++str_idx;
                                break;

                            default:
                                parse_specifier( output_str, fmt[str_idx + 2], args, long );
                                break;
                        }
                        ++str_idx;
                        break;

                    case 'q':  // Long Long NOLINT
                        parse_specifier( output_str, fmt[str_idx + 2], args, llong );
                        ++str_idx;
                        break;

                    case 'h':  // Short
                        parse_specifier( output_str, fmt[str_idx + 2], args, int );
                        ++str_idx;
                        break;

                    default:
                        parse_specifier( output_str, fmt[str_idx + 1], args, int );
                        break;
                }

                ++str_idx;
                break;

            // All other characters
            default:
                format_char( output_str, fmt[str_idx] );
                break;
        }
    }

    print_clear_buff( output_str );

    va_end( args );

    return 0;
}

/*** End of File ***/