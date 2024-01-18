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
#include "scan_code_set_2_to_ascii.h"

/* Private Defines and Macros */

#define BUFFER_EMPTY ( 0U )

// Status Register
#define STATUS_REGISTER_ADDR ( (uintptr_t)0x64 )

// Status Register Masks (Bits 4 & 5 are unused)
#define OUTPUT_BUFFER_STATUS_MSK( stat ) ( ( stat ) & ( 0b00000001 ) )  // Must be FULL(1) to read
#define INPUT_BUFFER_STATUS_MSK( stat )  ( ( stat ) & ( 0b00000010 ) )  // Must be EMPTY(0) to write
#define SYSTEM_FLAG_MSK( stat )          ( ( stat ) & ( 0b00000100 ) )  // 0 = normal, 1 = error
#define COMMAND_DATA_MSK( stat )         ( ( stat ) & ( 0b00001000 ) )  // 0 = DEVICE, 1 = CONTLR
#define TIMEOUT_ERROR_MSK( stat )        ( ( stat ) & ( 0b01000000 ) )  // 0 = normal, 1 = error
#define PARITY_ERROR_MSK( stat )         ( ( stat ) & ( 0b10000000 ) )  // 0 = normal, 1 = error

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

/* Private Typedefs */

/* Global Variables */

/* Private Functions */

// TODO: Check Command/Data flag in Status Register

uint8_t status_register_read( void )
{
    // Read the status register
    return ASM_inb( STATUS_REGISTER_ADDR );
}

uint8_t data_port_read( void )
{
    // Read the byte from the data port
    return ASM_inb( DATA_PORT_ADDR );
}

void data_port_write( uint8_t byte )
{
    // Write the byte to the data port
    ASM_outb( DATA_PORT_ADDR, byte );
}

void command_register_cmd( uint8_t command )
{
    // Write the command to the command register
    ASM_outb( COMMAND_REGISTER_ADDR, command );

    // Wait
    sleep( 1 );
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

#define MAX_RETRIES ( 30U )

uint8_t keyboard_read( void )
{
    // Poll the status register until the output buffer is full
    while ( OUTPUT_BUFFER_STATUS_MSK( status_register_read() ) == BUFFER_EMPTY )
    {
        // Wait
        sleep( 1 );
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
        sleep( 1 );
    }

    // Write the byte to the data port
    data_port_write( byte );

    // Wait
    sleep( 1 );
}

/* Public Functions */

driver_status_t keyboard_driver_polling_init( void )
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

    // Write the configuration byte back to the data port
    command_register_cmd_write( CMD_WRITE_BYTE_0, config_byte );

    // Perform a self test on the PS/2 controller and read the result
    status_byte = command_register_cmd_read( CMD_CONTLR_SELF_TEST );
    if ( status_byte != CONTLR_SELF_TEST_OK )
    {
        // Send an error message
        printk(
            "ERR: PS/2 Controller Self Test Failed! "
            "Exit code = 0x%X\n\n",
            status_byte
        );

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

        printk(
            "ERR: PS/2 Port 1 Interface Test Failed! "
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
        printk( "ERR: PS/2 Port 1 Reset Failed! Exit code = 0x%X\n\n", status_byte );

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

        printk( "ERR: Get scan code returned 0x%X\n", status_byte );
        return FAILURE;
    }

    // Verify the scan code set
    keyboard_write( KBD_CMD_GET_SET_SCAN_CODE_SET );
    keyboard_write( KBD_CMD_GET_SCAN_CODE_SET );

    if ( keyboard_read() != KBD_ACK )
    {
        printk( "ERR: Get scan code returned 0x%X\n\n", status_byte );
        return FAILURE;
    }

    status_byte = keyboard_read();

    if ( status_byte != SCAN_CODE_SET2 )
    {
        printk( "ERR: Get scan code returned 0x%X\n\n", status_byte );
        return FAILURE;
    }

    // Enable port 1
    command_register_cmd( CMD_PORT_1_ENABLE );

    // Return success
    return SUCCESS;
}

char keyboard_driver_polling_get_char( void )
{
    // Read the byte from the data port and convert it to ASCII
    return scan_code_set2_to_ascii( keyboard_read() );
}

/*** End of File ***/