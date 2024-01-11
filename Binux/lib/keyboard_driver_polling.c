/** @file keyboard_driver_polling.c
 *
 * @brief A description of the module's purpose.
 *
 * @author Bryce Melander
 * @date Jan-10-2024
 *
 * @copyright (c) 2024 by Bryce Melander under MIT License. All rights reserved.
 * (See http://opensource.org/licenses/MIT for more details.)
 */

#include "keyboard_driver_polling.h"

#include "printk.h"

/* Private Defines and Macros */

// Status Register
#define STATUS_REGISTER_ADDR ( (uintptr_t)0x64 )

// Status Register Masks (Bits 4 & 5 are unused)
#define OUTPUT_BUFFER_STATUS_MSK( stat ) ( ( stat ) & ( 0b00000001 ) )  // 0 = empty, 1 = full
#define INPUT_BUFFER_STATUS_MSK( stat )  ( ( stat ) & ( 0b00000010 ) )  // ^          ^
#define SYSTEM_FLAG_MSK( stat )          ( ( stat ) & ( 0b00000100 ) )  // 0 = normal, 1 = error
#define COMMAND_DATA_MSK( stat )         ( ( stat ) & ( 0b00001000 ) )  // ^           ^
#define TIMEOUT_ERROR_MSK( stat )        ( ( stat ) & ( 0b01000000 ) )  // ^           ^
#define PARITY_ERROR_MSK( stat )         ( ( stat ) & ( 0b10000000 ) )  // ^           ^

// Command Register
#define COMMAND_REGISTER_ADDR ( (uintptr_t)0x64 )

// Command Register Commands
#define CMD_PORT_1_ENABLE  ( 0xAE )
#define CMD_PORT_1_DISABLE ( 0xAD )

#define CMD_PORT_2_ENABLE  ( 0xA8 )
#define CMD_PORT_2_DISABLE ( 0xA7 )

#define CMD_READ_BYTE_0  ( 0x20 )
#define CMD_WRITE_BYTE_0 ( 0x60 )

#define CMD_CONTLR_SELF_TEST ( 0xAA )
#define CONTLR_SELF_TEST_OK  ( 0x55 )

#define CMD_PORT_1_SELF_TEST ( 0xAB )
#define PORT_1_SELF_TEST_OK  ( 0x00 )

#define CMD_PORT1_RESET ( 0xFE )

// PS/2 Keyboard Commands
#define KBD_CMD_GET_SET_SCAN_CODE_SET ( 0xF0 )
#define KBD_CMD_GET_SCAN_CODE_SET     ( 0x00 )
#define KBD_SCAN_CODE_SET_1           ( 0x01 )
#define KBD_SCAN_CODE_SET_2           ( 0x02 )
#define KBD_SCAN_CODE_SET_3           ( 0x03 )

#define KBD_ACK    ( 0xFA )
#define KBD_RESEND ( 0xFE )

// Data Port
#define DATA_PORT_ADDR ( (uintptr_t)0x60 )

// Configuration Bytes
#define PORT1_CLK_EN ( 0b00000001 )
#define PORT2_CLK_EN ( 0b00000010 )
#define PORT1_INT_EN ( 0b00010000 )
#define PORT2_INT_EN ( 0b00100000 )

// Scan Codes
#define SCAN_CODE_SET2 ( 0x41 )

/* Private Typedefs */

/* Global Variables */

/* Private Functions */

uint8_t status_register_read( void )
{
    // Read the status register
    return *(uint8_t *)( STATUS_REGISTER_ADDR );
}

uint8_t data_port_read( void )
{
    // Poll the status register until the output buffer is full
    while ( OUTPUT_BUFFER_STATUS_MSK( status_register_read() ) == 0 )
    {
        // Sleep for 1 second
        sleep( 1 );
    }

    // Read the byte from the data port
    return *(uint8_t *)( DATA_PORT_ADDR );
}

void data_port_write( uint8_t byte )
{
    // Poll the status register until the input buffer is empty
    while ( INPUT_BUFFER_STATUS_MSK( status_register_read() ) == 0 )
    {
        // Sleep for 1 second
        sleep( 1 );
    }

    // Write the byte to the data port
    *(uint8_t *)( DATA_PORT_ADDR ) = byte;
}

void command_register_cmd( uint8_t command )
{
    // Poll the status register until the input buffer is empty
    while ( INPUT_BUFFER_STATUS_MSK( status_register_read() ) == 0 )
    {
        // Sleep for 1 second
        sleep( 1 );
    }

    // Write the command to the command register
    *(uint8_t *)( COMMAND_REGISTER_ADDR ) = command;
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

uint8_t keyboard_send_cmd( uint8_t command )
{
    // Write the command to the data port
    data_port_write( command );

    // Read the resulting byte from the data port
    return data_port_read();
}

char scan_code_set2_to_ascii( uint8_t scan_code )
{
    // TODO: Implement this function
    return (char)scan_code;
}

/* Public Functions */

driver_status_t keyboard_driver_polling_init( void )
{
    uint8_t config_byte, status_byte;

    // Disable ports 1 & 2
    command_register_cmd( CMD_PORT_1_DISABLE );
    command_register_cmd( CMD_PORT_2_DISABLE );

    // Flush the output buffer
    data_port_read();

    // Read the configuration byte from the controller
    config_byte = command_register_cmd_read( CMD_READ_BYTE_0 );

    // Enable port 1 clock and interrupt
    config_byte |= ( PORT1_CLK_EN | PORT1_INT_EN );
    // Disable port 2 clock and interrupt
    config_byte &= ~( PORT2_CLK_EN | PORT2_INT_EN );

    // Write the configuration byte back to the data port
    command_register_cmd_write( CMD_WRITE_BYTE_0, config_byte );

    // Perform a self test on the PS/2 controller and read the result
    if ( command_register_cmd_read( CMD_CONTLR_SELF_TEST ) != CONTLR_SELF_TEST_OK )
    {
        // Send an error message
        printk( "ERR: PS/2 Controller Self Test Failed!\n" );

        // Return failure
        return FAILURE;
    }

    // Restore the configuration byte (in case the self test reset it)
    command_register_cmd_write( CMD_WRITE_BYTE_0, config_byte );

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

        printk(
            "ERR: PS/2 Port 1 Interface Test Failed! "
            "Exit code = 0x%x ( %s )\n",
            status_byte, status_byte_err_msg
        );

        // Return failure
        return FAILURE;
    }

    // Enable port 1
    command_register_cmd( CMD_PORT_1_ENABLE );

    // Reset port 1
    status_byte = command_register_cmd_read( CMD_PORT1_RESET );
    if ( status_byte != 0xAA )
    {
        // Send an error message
        printk(
            "ERR: PS/2 Port 1 Reset Failed! "
            "(status_byte = 0x%x)\n",
            status_byte
        );

        // Return failure
        return FAILURE;
    }

    // Setup the keyboard's scan code
    do
    {
        // Set the scan code set to Set 2
        status_byte = keyboard_send_cmd( KBD_SCAN_CODE_SET_2 );

        if ( status_byte == KBD_RESEND )
        {
            continue;
        }

        // Verify the scan code set
        status_byte = keyboard_send_cmd( KBD_CMD_GET_SET_SCAN_CODE_SET );

    } while ( status_byte != SCAN_CODE_SET2 );

    // Return success
    return SUCCESS;
}

char keyboard_driver_polling_get_char( void )
{
    // Read the byte from the data port and convert it to ASCII
    return scan_code_set2_to_ascii( data_port_read() );
}

/*** End of File ***/