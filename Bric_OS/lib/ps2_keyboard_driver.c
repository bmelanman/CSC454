/** @file ps2_keyboard_driver.c
 *
 * @brief A description of the module's purpose.
 *
 * @author Bryce Melander
 * @date Jan-10-2024
 *
 * @copyright (c) 2024 by Bryce Melander under MIT License. All rights reserved.
 * (See http://opensource.org/licenses/MIT for more details.)
 */

#include "ps2_keyboard_driver.h"

#include "common.h"
#include "printk.h"
#include "vga_driver.h"

#pragma region "Polling Keyboard Driver"

#pragma region "Private Defines and Macros"

#define IO_WAIT_LEN ( 100U )

// Status Register
#define STATUS_REGISTER_ADDR ( (uintptr_t)0x64 )

// Status Register Masks (Bits 4 & 5 are unused)
#define OUTPUT_BUFFER_STATUS_MSK( stat ) ( ( stat ) & ( 0b00000001 ) )  // Must be FULL(1) to read
#define INPUT_BUFFER_STATUS_MSK( stat )  ( ( stat ) & ( 0b00000010 ) )  // Must be EMPTY(0) to write
#define SYSTEM_FLAG_MSK( stat )          ( ( stat ) & ( 0b00000100 ) )  // 0 = normal, 1 = error
#define COMMAND_DATA_MSK( stat )         ( ( stat ) & ( 0b00001000 ) )  // 0 = DEVICE, 1 = CONTLR
#define TIMEOUT_ERROR_MSK( stat )        ( ( stat ) & ( 0b01000000 ) )  // 0 = normal, 1 = error
#define PARITY_ERROR_MSK( stat )         ( ( stat ) & ( 0b10000000 ) )  // 0 = normal, 1 = error

#define BUFFER_EMPTY ( 0U )

#define DEVICE_COMMAND ( 0U )
#define CONTLR_COMMAND ( 1U )

// Command Register
#define COMMAND_REGISTER_ADDR ( (uintptr_t)0x64 )

// Command Register Commands
#define CMD_PORT_1_ENABLE  ( 0xAEU )
#define CMD_PORT_1_DISABLE ( 0xADU )

#define CMD_PORT_2_ENABLE  ( 0xA8U )
#define CMD_PORT_2_DISABLE ( 0xA7U )

#define CMD_READ_BYTE_0  ( 0x20U )
#define CMD_WRITE_BYTE_0 ( 0x60U )

#define CMD_CONTLR_SELF_TEST ( 0xAAU )
#define CMD_PORT_1_SELF_TEST ( 0xABU )

#define CMD_PORT1_RESET ( 0xFFU )

#define CONTLR_SELF_TEST_OK ( 0x55U )
#define PORT_1_SELF_TEST_OK ( 0x00U )

// PS/2 Keyboard Commands
#define KBD_CMD_GET_SET_SCAN_CODE_SET ( 0xF0U )
#define KBD_CMD_GET_SCAN_CODE_SET     ( 0x00U )
#define KBD_SCAN_CODE_SET_1           ( 0x01U )
#define KBD_SCAN_CODE_SET_2           ( 0x02U )
#define KBD_SCAN_CODE_SET_3           ( 0x03U )

#define KBD_ACK    ( 0xFAU )
#define KBD_RESEND ( 0xFEU )

// Data Port
#define DATA_PORT_ADDR ( (uintptr_t)0x60 )

// Configuration Bytes
#define PORT1_INT_EN      ( 0b00000001U )
#define PORT2_INT_EN      ( 0b00000010U )
#define SYSTEM_FLAG       ( 0b00000100U )
#define PORT1_CLK_DISABLE ( 0b00010000U )
#define PORT2_CLK_DISABLE ( 0b00100000U )

// Scan Codes
#define SCAN_CODE_SET2 ( 0x41U )

#pragma endregion

#pragma region "Private Functions"

// TODO: Check Command/Data flag in Status Register?

uint8_t status_register_read( void )
{
    // Read the status register
    return inb( STATUS_REGISTER_ADDR );
}

uint8_t data_port_read( void )
{
    // Read the byte from the data port
    return inb( DATA_PORT_ADDR );
}

void data_port_write( uint8_t byte )
{
    // Write the byte to the data port
    outb( DATA_PORT_ADDR, byte );
}

void command_register_cmd( uint8_t command )
{
    // Write the command to the command register
    outb( COMMAND_REGISTER_ADDR, command );

    // Wait
    io_wait_n( IO_WAIT_LEN );
}

uint8_t command_register_cmd_read( uint8_t command )
{
    // Write the command
    command_register_cmd( command );

    // Read the resulting byte from the data port
    return data_port_read();
}

void command_register_cmd_write( uint8_t command, uint8_t byte )
{
    // Write the command
    command_register_cmd( command );

    // Write the byte to the data port
    data_port_write( byte );
}

uint8_t keyboard_read( void )
{
    // Poll the status register until the output buffer is full
    while ( OUTPUT_BUFFER_STATUS_MSK( status_register_read() ) == BUFFER_EMPTY )
    {
        // Wait
        io_wait_n( IO_WAIT_LEN );
    }

    // Read the resulting byte from the data port
    return data_port_read();
}

void keyboard_write( uint8_t byte )
{
    // Poll the status register until the input buffer is empty
    while ( INPUT_BUFFER_STATUS_MSK( status_register_read() ) != BUFFER_EMPTY )
    {
        // Wait
        io_wait_n( IO_WAIT_LEN );
    }

    // Write the byte to the data port
    data_port_write( byte );

    // Wait
    io_wait_n( IO_WAIT_LEN );
}

#pragma endregion

/* Public Functions */

void ps2_keyboard_driver_interrupt_handler( int irq, int error, void *arg )
{
    // Ignore unused parameters
    (void)irq;
    (void)error;
    (void)arg;

    // Read the scan code
    uint8_t curr_code = keyboard_read();

    // Print the scan code
    OS_INFO( "Scan Code = 0x%X", curr_code );
}

driver_status_t ps2_keyboard_driver_init( int driver_type )
{
    uint8_t config_byte, status_byte;

    // Disable ports 1 & 2
    command_register_cmd( CMD_PORT_1_DISABLE );
    command_register_cmd( CMD_PORT_2_DISABLE );

    // Flush the data buffer
    data_port_read();
    data_port_read();

    // Read the configuration byte from the controller
    config_byte = command_register_cmd_read( CMD_READ_BYTE_0 );

    // Disable port 2 clock, set the system flag to 1
    config_byte |= ( PORT2_CLK_DISABLE | SYSTEM_FLAG );
    // Enable port 1 clock, disable all interrupts
    config_byte &= ~( PORT1_CLK_DISABLE | PORT1_INT_EN | PORT2_INT_EN );

    // Enable interrupts if specified
    if ( driver_type == PS2_DRIVER_IRQ )
    {
        // Enable port 1 interrupt
        config_byte |= PORT1_INT_EN;
    }

    // Write the configuration byte back to the data port
    command_register_cmd_write( CMD_WRITE_BYTE_0, config_byte );

    // Perform a self test on the PS/2 controller and read the result
    status_byte = command_register_cmd_read( CMD_CONTLR_SELF_TEST );
    if ( status_byte != CONTLR_SELF_TEST_OK )
    {
        // Send an error message
        OS_ERROR( "PS/2 Controller Self Test Failed! Exit code = 0x%X\n\n", status_byte );

        // Return failure
        return FAILURE;
    }

    // Restore the configuration byte (in case the self test reset it)
    if ( command_register_cmd_read( CMD_READ_BYTE_0 ) != config_byte )
    {
        command_register_cmd_write( CMD_WRITE_BYTE_0, config_byte );
    }

    // Perform an interface test on port 1
    status_byte = command_register_cmd_read( CMD_PORT_1_SELF_TEST );
    if ( status_byte != PORT_1_SELF_TEST_OK )
    {
        // Send an error message
        char status_byte_err_msg[32] = { 0 };

        switch ( status_byte )
        {
            case 0x01:
                strcpy( status_byte_err_msg, "clock line stuck low" );
                break;
            case 0x02:
                strcpy( status_byte_err_msg, "clock line stuck high" );
                break;
            case 0x03:
                strcpy( status_byte_err_msg, "data line stuck low" );
                break;
            case 0x04:
                strcpy( status_byte_err_msg, "data line stuck high" );
                break;
            default:
                strcpy( status_byte_err_msg, "unknown error" );
                break;
        }

        OS_ERROR(
            "PS/2 Port 1 Interface Test Failed! "
            "Exit code = 0x%X ( %s )\n\n",
            status_byte, status_byte_err_msg
        );

        // Return failure
        return FAILURE;
    }

    // Reset port 1
    status_byte = command_register_cmd_read( CMD_PORT1_RESET );
    if ( status_byte != 0 )
    {
        // Send an error message
        OS_ERROR( "PS/2 Port 1 Reset Failed! Exit code = 0x%X\n\n", status_byte );

        // Return failure
        return FAILURE;
    }

    // Setup the keyboard's scan code
    while ( 1 )
    {
        keyboard_write( KBD_CMD_GET_SET_SCAN_CODE_SET );
        keyboard_write( KBD_SCAN_CODE_SET_2 );
        status_byte = keyboard_read();

        if ( status_byte == KBD_RESEND )
        {
            continue;
        }

        if ( status_byte == KBD_ACK )
        {
            break;
        }

        OS_ERROR( "Get scan code returned 0x%X\n", status_byte );
        return FAILURE;
    }

    // Verify the scan code set
    keyboard_write( KBD_CMD_GET_SET_SCAN_CODE_SET );
    keyboard_write( KBD_CMD_GET_SCAN_CODE_SET );

    if ( keyboard_read() != KBD_ACK )
    {
        OS_ERROR( "Get scan code returned 0x%X\n\n", status_byte );
        return FAILURE;
    }

    status_byte = keyboard_read();

    if ( status_byte != SCAN_CODE_SET2 )
    {
        OS_ERROR( "Get scan code returned 0x%X\n\n", status_byte );
        return FAILURE;
    }

    // Enable port 1
    command_register_cmd( CMD_PORT_1_ENABLE );

    // Return success
    return SUCCESS;
}

#pragma endregion

#pragma region "Scan Code Set 2 to ASCII"

#pragma region "Private Defines and Macros"

#define NUL ( '\0' )

// Special Characters
#define ESC ( 0x1B )
#define BS  ( '\b' )
#define TAB ( '\t' )
#define LF  ( '\n' )

// Modifier Keys
#define CAPS   ( 0x3AU )
#define L_SHFT ( 0x2AU )
#define R_SHFT ( 0x36U )
#define CTRL   ( 0x1DU )
#define ALT    ( 0x38U )

// Check if the scan code is a modifier key
#define IS_SHFT( x ) ( ( x ) == L_SHFT || ( x ) == R_SHFT )
#define IS_CAPS( x ) ( ( x ) == CAPS )
#define IS_CTRL( x ) ( ( x ) == CTRL )
#define IS_ALT( x )  ( ( x ) == ALT )

#define IS_MOD( x ) ( IS_SHFT( x ) || IS_CAPS( x ) || IS_CTRL( x ) || IS_ALT( x ) )

#define KEY_RELEASED ( 0x80U )

#define IS_CODE_KEY_RELEASE( curr ) ( ( curr ) & KEY_RELEASED )

// Uppercase and lowercase keys
#define ASCII_TABLE_SIZE 128

// Standard ASCII Table with/without the shift key pressed
char standard_ascii_table_LOWER[ASCII_TABLE_SIZE] = {
    NUL,  ESC,  '1', '2',  '3', '4', '5', '6', '7', '8', '9', '0',  '-', '=', BS,  // First Row
    TAB,  'q',  'w', 'e',  'r', 't', 'y', 'u', 'i', 'o', 'p', '[',  ']', LF,       // Second Row
    CTRL, 'a',  's', 'd',  'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',           // Third Row
    NUL,  '\\', 'z', 'x',  'c', 'v', 'b', 'n', 'm', ',', '.', '/',  NUL, '*',      // Fourth Row
    ALT,  ' ',  NUL, CAPS, NUL, NUL, NUL, NUL, NUL, NUL, NUL, LF,   NUL, NUL,      // Misc Keys
};

char standard_ascii_table_UPPER[ASCII_TABLE_SIZE] = {
    NUL,  ESC, '!', '@',  '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', BS,  // First Row
    TAB,  'Q', 'W', 'E',  'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', LF,       // Second Row
    CTRL, 'A', 'S', 'D',  'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',           // Third Row
    NUL,  '|', 'Z', 'X',  'C', 'V', 'B', 'N', 'M', '<', '>', '?', NUL, '*',      // Fourth Row
    ALT,  ' ', NUL, CAPS, NUL, NUL, NUL, NUL, NUL, NUL, NUL, LF,  NUL, NUL,      // Misc Keys
};

// Caps Lock ASCII Table with/without the shift key pressed
char caps_ascii_table_LOWER[ASCII_TABLE_SIZE] = {
    NUL,  ESC,  '1', '2',  '3', '4', '5', '6', '7', '8', '9', '0',  '-', '=', BS,  // First Row
    TAB,  'Q',  'W', 'E',  'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[',  ']', LF,       // Second Row
    CTRL, 'A',  'S', 'D',  'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`',           // Third Row
    NUL,  '\\', 'Z', 'X',  'C', 'V', 'B', 'N', 'M', ',', '.', '/',  NUL, '*',      // Fourth Row
    ALT,  ' ',  NUL, CAPS, NUL, NUL, NUL, NUL, NUL, NUL, NUL, LF,   NUL, NUL,      // Misc Keys
};

char caps_ascii_table_UPPER[ASCII_TABLE_SIZE] = {
    NUL,  ESC, '!', '@',  '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', BS,  // First Row
    TAB,  'q', 'w', 'e',  'r', 't', 'y', 'u', 'i', 'o', 'p', '{', '}', LF,       // Second Row
    CTRL, 'a', 's', 'd',  'f', 'g', 'h', 'j', 'k', 'l', ':', '"', '~',           // Third Row
    NUL,  '|', 'z', 'x',  'c', 'v', 'b', 'n', 'm', '<', '>', '?', NUL, '*',      // Fourth Row
    ALT,  ' ', NUL, CAPS, NUL, NUL, NUL, NUL, NUL, NUL, NUL, LF,  NUL, NUL,      // Misc Keys
};

#pragma endregion

#pragma region "Global Variables"

typedef enum { RELEASED = 0, PRESSED = 1 } key_state_t;

char *ascii_table_LOWER = standard_ascii_table_LOWER;
char *ascii_table_UPPER = standard_ascii_table_UPPER;

static key_state_t key_state = RELEASED;
static key_state_t caps_state = RELEASED;
static key_state_t shift_state = RELEASED;
static key_state_t ctrl_state = RELEASED;
static key_state_t alt_state = RELEASED;

static uint8_t prev_code = 0, curr_code = 0;

#pragma endregion

void VGA_display_hex_str( const char *s, uint8_t byte )
{
    uint8_t nibble;

    VGA_display_str( s );
    VGA_display_str( "0x" );

    // Print the high nibble
    nibble = ( byte & 0xF0 ) >> 4;
    if ( nibble < 10 )
    {
        VGA_display_char( (char)( nibble + '0' ) );
    }
    else
    {
        VGA_display_char( (char)( nibble - 10 + 'A' ) );
    }

    // Print the low nibble
    nibble = byte & 0x0F;
    if ( nibble < 10 )
    {
        VGA_display_char( (char)( nibble + '0' ) );
    }
    else
    {
        VGA_display_char( (char)( nibble - 10 + 'A' ) );
    }

    VGA_display_char( '\n' );
}

char polling_keyboard_get_char( void )
{
    char key = NUL;

    // TODO: Fix key release issues on modifier keys
    // TODO: Fix how backspace works in the vga driver

    while ( true )
    {
        // Save the previous scan code
        prev_code = curr_code;

        // Wait for a scan code
        curr_code = keyboard_read();

        // DEBUG: Print the scan code
        // VGA_display_hex_str( "Scan Code = ", curr_code );

        // Check if the key was pressed or released
        if ( IS_CODE_KEY_RELEASE( curr_code ) )
        {
            // Key was released
            key_state = RELEASED;

            switch ( curr_code - KEY_RELEASED )
            {
                case L_SHFT:
                case R_SHFT:
                    // Shift key was released
                    shift_state = RELEASED;
                    break;

                case CAPS:
                    // Caps lock key was released
                    caps_state = RELEASED;

                    // Update the ASCII table
                    ascii_table_LOWER = standard_ascii_table_LOWER;
                    ascii_table_UPPER = standard_ascii_table_UPPER;
                    break;

                case CTRL:
                    // Control key was released
                    ctrl_state = RELEASED;
                    break;

                case ALT:
                    // Alt key was released
                    alt_state = RELEASED;
                    break;

                default:
                    break;
            }

            continue;
        }

        // Key was pressed
        key_state = PRESSED;

        // Check if the shift key was pressed/released
        if ( IS_SHFT( curr_code ) )
        {
            // Key was pressed
            shift_state = PRESSED;

            continue;
        }

        // Check if the caps lock key was pressed/released
        if ( IS_CAPS( curr_code ) )
        {
            // Toggle the caps lock state
            caps_state = PRESSED;

            // Update the ASCII table
            ascii_table_LOWER = caps_ascii_table_LOWER;
            ascii_table_UPPER = caps_ascii_table_UPPER;

            continue;
        }

        // Make sure the key is valid
        if ( curr_code >= ASCII_TABLE_SIZE )
        {
            OS_ERROR( "Invalid scan code? Code = 0x%X\n", curr_code );
            continue;
        }

        // Check if the shift key was also pressed
        if ( shift_state == PRESSED )
        {
            // Return the uppercase character
            key = ascii_table_UPPER[curr_code];
        }
        else
        {
            // Return the lowercase character
            key = ascii_table_LOWER[curr_code];
        }

        break;
    }

    // Return the key!
    return key;
}

#pragma endregion

/*** End of File ***/