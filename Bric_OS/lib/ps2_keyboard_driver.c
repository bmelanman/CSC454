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

/* Includes */

#include "common.h"
#include "irq_handler.h"
#include "printk.h"
#include "vga_driver.h"

/* Private Defines and Macros */
#pragma region

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

#define KEY_RELEASED ( 0x80U )

// Modifier Keys
#define CAPS ( 0x3AU )
#define LSFT ( 0x2AU )
#define RSFT ( 0x36U )
#define CTRL ( 0x1DU )
#define ALT  ( 0x38U )

// Arrow Keys
#define UP  ( 0x48 )
#define DN  ( 0x4B )
#define LFT ( 0x4D )
#define RHT ( 0x50 )

// Function Keys
#define F1  ( 'F' )
#define F2  ( 'F' )
#define F3  ( 'F' )
#define F4  ( 'F' )
#define F5  ( 'F' )
#define F6  ( 'F' )
#define F7  ( 'F' )
#define F8  ( 'F' )
#define F9  ( 'F' )
#define F10 ( 'F' )

// Check if the scan code is a modifier key
#define IS_SHFT( x )  ( ( x ) == LSFT || ( x ) == RSFT )
#define IS_CAPS( x )  ( ( x ) == CAPS )
#define IS_CTRL( x )  ( ( x ) == CTRL )
#define IS_ALT( x )   ( ( x ) == ALT )
#define IS_ARROW( x ) ( ( x ) == UP || ( x ) == DN || ( x ) == LFT || ( x ) == RHT )

#define IS_MOD( x )                 ( IS_SHFT( x ) || IS_CAPS( x ) || IS_CTRL( x ) || IS_ALT( x ) || IS_ARROW( x ) )
#define IS_CODE_KEY_RELEASE( curr ) ( ( curr ) & KEY_RELEASED )

#define ASCII_TABLE_SIZE 128

#pragma endregion

/* Typedefs */

typedef enum { RELEASED = 0, PRESSED = 1 } key_state_t;

/* Global Variables */
#pragma region

// Standard ASCII Table with/without the shift key pressed
char standard_ascii_table_LOWER[ASCII_TABLE_SIZE] = {
    NUL,  ESC, '1',  '2',  '3', '4',  '5', '6',  // 0x00 - 0x07
    '7',  '8', '9',  '0',  '-', '=',  BS,  TAB,  // 0x08 - 0x0F
    'q',  'w', 'e',  'r',  't', 'y',  'u', 'i',  // 0x10 - 0x17
    'o',  'p', '[',  ']',  LF,  CTRL, 'a', 's',  // 0x18 - 0x1F
    'd',  'f', 'g',  'h',  'j', 'k',  'l', ';',  // 0x20 - 0x27
    '\'', '`', NUL,  '\\', 'z', 'x',  'c', 'v',  // 0x28 - 0x2F
    'b',  'n', 'm',  ',',  '.', '/',  NUL, '*',  // 0x30 - 0x37
    ALT,  ' ', CAPS, F1,   F2,  F3,   F4,  F5,   // 0x38 - 0x3F
    F6,   F7,  F8,   F9,   F10, NUL,  NUL, NUL,  // 0x40 - 0x47
    UP,   NUL, NUL,  LFT,  NUL, RHT,  NUL, NUL,  // 0x48 - 0x4F
    DN,   NUL, NUL,  NUL,  NUL, NUL,  NUL, NUL,  // 0x50 - 0x57
    NUL,  NUL, NUL,  NUL,  NUL, NUL,  NUL, NUL,  // 0x58 - 0x5F
    NUL,  NUL, NUL,  NUL,  NUL, NUL,  NUL, NUL,  // 0x60 - 0x67
    NUL,  NUL, NUL,  NUL,  NUL, NUL,  NUL, NUL,  // 0x68 - 0x6F
    NUL,  NUL, NUL,  NUL,  NUL, NUL,  NUL, NUL,  // 0x70 - 0x77
    NUL,  NUL, NUL,  NUL,  NUL, NUL,  NUL, NUL   // 0x78 - 0x7F
};

char standard_ascii_table_UPPER[ASCII_TABLE_SIZE] = {
    NUL, ESC, '!',  '@', '#', '$',  '%', '^',  // 0x00 - 0x07
    '&', '*', '(',  ')', '_', '+',  BS,  TAB,  // 0x08 - 0x0F
    'Q', 'W', 'E',  'R', 'T', 'Y',  'U', 'I',  // 0x10 - 0x17
    'O', 'P', '{',  '}', LF,  CTRL, 'A', 'S',  // 0x18 - 0x1F
    'D', 'F', 'G',  'H', 'J', 'K',  'L', ':',  // 0x20 - 0x27
    '"', '~', NUL,  '|', 'Z', 'X',  'C', 'V',  // 0x28 - 0x2F
    'B', 'N', 'M',  '<', '>', '?',  NUL, '*',  // 0x30 - 0x37
    ALT, ' ', CAPS, F1,  F2,  F3,   F4,  F5,   // 0x38 - 0x3F
    F6,  F7,  F8,   F9,  F10, NUL,  NUL, NUL,  // 0x40 - 0x47
    UP,  NUL, NUL,  LFT, NUL, RHT,  NUL, NUL,  // 0x48 - 0x4F
    DN,  NUL, NUL,  NUL, NUL, NUL,  NUL, NUL,  // 0x50 - 0x57
    NUL, NUL, NUL,  NUL, NUL, NUL,  NUL, NUL,  // 0x58 - 0x5F
    NUL, NUL, NUL,  NUL, NUL, NUL,  NUL, NUL,  // 0x60 - 0x67
    NUL, NUL, NUL,  NUL, NUL, NUL,  NUL, NUL,  // 0x68 - 0x6F
    NUL, NUL, NUL,  NUL, NUL, NUL,  NUL, NUL,  // 0x70 - 0x77
    NUL, NUL, NUL,  NUL, NUL, NUL,  NUL, NUL   // 0x78 - 0x7F
};

// Caps Lock ASCII Table with/without the shift key pressed
char caps_ascii_table_LOWER[ASCII_TABLE_SIZE] = {
    NUL,  ESC, '1',  '2',  '3', '4',  '5', '6',  // 0x00 - 0x07
    '7',  '8', '9',  '0',  '-', '=',  BS,  TAB,  // 0x08 - 0x0F
    'Q',  'W', 'E',  'R',  'T', 'Y',  'U', 'I',  // 0x10 - 0x17
    'O',  'P', '[',  ']',  LF,  CTRL, 'A', 'S',  // 0x18 - 0x1F
    'D',  'F', 'G',  'H',  'J', 'K',  'L', ';',  // 0x20 - 0x27
    '\'', '`', NUL,  '\\', 'Z', 'X',  'C', 'V',  // 0x28 - 0x2F
    'B',  'N', 'M',  ',',  '.', '/',  NUL, '*',  // 0x30 - 0x37
    ALT,  ' ', CAPS, F1,   F2,  F3,   F4,  F5,   // 0x38 - 0x3F
    F6,   F7,  F8,   F9,   F10, NUL,  NUL, NUL,  // 0x40 - 0x47
    UP,   NUL, NUL,  LFT,  NUL, RHT,  NUL, NUL,  // 0x48 - 0x4F
    DN,   NUL, NUL,  NUL,  NUL, NUL,  NUL, NUL,  // 0x50 - 0x57
    NUL,  NUL, NUL,  NUL,  NUL, NUL,  NUL, NUL,  // 0x58 - 0x5F
    NUL,  NUL, NUL,  NUL,  NUL, NUL,  NUL, NUL,  // 0x60 - 0x67
    NUL,  NUL, NUL,  NUL,  NUL, NUL,  NUL, NUL,  // 0x68 - 0x6F
    NUL,  NUL, NUL,  NUL,  NUL, NUL,  NUL, NUL,  // 0x70 - 0x77
    NUL,  NUL, NUL,  NUL,  NUL, NUL,  NUL, NUL   // 0x78 - 0x7F
};

char caps_ascii_table_UPPER[ASCII_TABLE_SIZE] = {
    NUL, ESC, '!',  '@', '#', '$',  '%', '^',  // 0x00 - 0x07
    '&', '*', '(',  ')', '_', '+',  BS,  TAB,  // 0x08 - 0x0F
    'q', 'w', 'e',  'r', 't', 'y',  'u', 'i',  // 0x10 - 0x17
    'o', 'p', '{',  '}', LF,  CTRL, 'a', 's',  // 0x18 - 0x1F
    'd', 'f', 'g',  'h', 'j', 'k',  'l', ':',  // 0x20 - 0x27
    '"', '~', NUL,  '|', 'z', 'x',  'c', 'v',  // 0x28 - 0x2F
    'b', 'n', 'm',  '<', '>', '?',  NUL, '*',  // 0x30 - 0x37
    ALT, ' ', CAPS, F1,  F2,  F3,   F4,  F5,   // 0x38 - 0x3F
    F6,  F7,  F8,   F9,  F10, NUL,  NUL, NUL,  // 0x40 - 0x47
    UP,  NUL, NUL,  LFT, NUL, RHT,  NUL, NUL,  // 0x48 - 0x4F
    DN,  NUL, NUL,  NUL, NUL, NUL,  NUL, NUL,  // 0x50 - 0x57
    NUL, NUL, NUL,  NUL, NUL, NUL,  NUL, NUL,  // 0x58 - 0x5F
    NUL, NUL, NUL,  NUL, NUL, NUL,  NUL, NUL,  // 0x60 - 0x67
    NUL, NUL, NUL,  NUL, NUL, NUL,  NUL, NUL,  // 0x68 - 0x6F
    NUL, NUL, NUL,  NUL, NUL, NUL,  NUL, NUL,  // 0x70 - 0x77
    NUL, NUL, NUL,  NUL, NUL, NUL,  NUL, NUL   // 0x78 - 0x7F
};

char *ascii_table_LOWER = standard_ascii_table_LOWER;
char *ascii_table_UPPER = standard_ascii_table_UPPER;

static key_state_t key_state = RELEASED;
static key_state_t caps_state = RELEASED;
static key_state_t shift_state = RELEASED;
static key_state_t ctrl_state = RELEASED;
static key_state_t alt_state = RELEASED;

static uint8_t prev_code = 0, curr_code = 0;

#pragma endregion

/* Private Functions */
#pragma region

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

int process_scan_code( int scan_code )
{
    char key = NUL;

    // TODO: Caps lock turns off when shift is pressed?

    // Save the previous scan code
    prev_code = curr_code;

    // Get the new scan code
    curr_code = scan_code;

    // DEBUG: Print the scan code
    // VGA_display_hex_str( "Scan Code = ", curr_code );

    // Check if the key was pressed or released
    if ( IS_CODE_KEY_RELEASE( curr_code ) )
    {
        // Key was released
        key_state = RELEASED;

        switch ( curr_code - KEY_RELEASED )
        {
            case LSFT:
            case RSFT:
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

        return NO_CHAR;
    }

    // Key was pressed
    key_state = PRESSED;

    switch ( curr_code )
    {
        case LSFT:
        case RSFT:
            // Key was pressed
            shift_state = PRESSED;

            return NO_CHAR;

        case CAPS:
            // Toggle the caps lock state
            caps_state = PRESSED;

            // Update the ASCII table
            ascii_table_LOWER = caps_ascii_table_LOWER;
            ascii_table_UPPER = caps_ascii_table_UPPER;

            return NO_CHAR;

        case CTRL:
            // Key was pressed
            ctrl_state = PRESSED;

            return NO_CHAR;

        case ALT:
            // Key was pressed
            alt_state = PRESSED;

            return NO_CHAR;

        case UP:
        case DN:
        case LFT:
        case RHT:
            // TODO: Implement arrow keys
            return NO_CHAR;

        default:
            // Make sure the key is valid
            if ( curr_code >= ASCII_TABLE_SIZE )
            {
                OS_ERROR( "Invalid scan code? Code = 0x%X\n", curr_code );
                return NO_CHAR;
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

            // Return the key!
            return key;
    }
}

#pragma endregion

/* Public Functions */

driver_status_t ps2_keyboard_driver_init( bool irq_enable )
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
    if ( irq_enable )
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

    // Enable interrupts if specified
    if ( irq_enable )
    {
        // Set and enable the interrupt handler for the keyboard
        IRQ_set_handler( IRQ1_KEYBOARD, ps2_keyboard_driver_interrupt_handler, NULL );

        // Enable port 1 interrupt
        IRQ_clear_mask( IRQ1_KEYBOARD );
    }

    // Return success
    return SUCCESS;
}

void ps2_keyboard_driver_interrupt_handler( int irq, int error, void *arg )
{
    // Ignore inputs
    (void)irq;
    (void)error;
    (void)arg;

    // Read the scan code
    uint8_t curr_code = keyboard_read();

    // Process the scan code
    int key = process_scan_code( curr_code );

    // Add the character to the buffer
    if ( key != NO_CHAR )
    {
        // TODO: Add the character to the stdio buffer

        // Display the character on the screen
        VGA_display_char( (char)key );
    }
}

char polling_keyboard_get_char( void )
{
    int scan_code, key;

    // Poll the keyboard until a key is pressed
    do
    {
        // Read the scan code
        scan_code = keyboard_read();

        // Process the scan code
        key = process_scan_code( scan_code );

    } while ( key == NO_CHAR );

    // Return the character
    return (char)key;
}

#pragma endregion

/*** End of File ***/