/** @file scan_code_set_2_to_ascii.c
 *
 * @brief A description of the module's purpose.
 *
 * @author Bryce Melander
 * @date Jan-11-2024
 *
 * @copyright (c) 2024 by Bryce Melander under MIT License. All rights reserved.
 * (See http://opensource.org/licenses/MIT for more details.)
 */

#include "scan_code_set_2_to_ascii.h"

/* Private Defines and Macros */

/* Global Variables */

// Modifier Keys
#define ESC   ( 0x1B )
#define BS    ( '\b' )
#define TAB   ( '\t' )
#define LF    ( '\n' )
#define SHIFT ( 0x2A )
#define CAPS  ( 0x3A )
#define CTRL  ( 0x1D )
#define ALT   ( 0x38 )

#define KEY_RELEASED ( 0x80 )

// Lowercase Letters, Numbers, and Symbols
char standard_ascii_table[128] = {
    0,    ESC,  '1', '2',  '3', '4', '5', '6', '7', '8', '9', '0',  '-', '=', BS,  // First Row
    TAB,  'q',  'w', 'e',  'r', 't', 'y', 'u', 'i', 'o', 'p', '[',  ']', LF,       // Second Row
    CTRL, 'a',  's', 'd',  'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',           // Third Row
    0,    '\\', 'z', 'x',  'c', 'v', 'b', 'n', 'm', ',', '.', '/',  0,   '*',      // Fourth Row
    ALT,  ' ',  0,   CAPS, 0,   0,   0,   0,   0,   0,   0,   LF,   0,   0,        // Misc Keys
};

// Uppercase Letters, Numbers, and Symbols
char shift_ascii_table[128] = {
    0,    ESC, '!', '@',  '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', BS,  // First Row
    TAB,  'Q', 'W', 'E',  'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', LF,       // Second Row
    CTRL, 'A', 'S', 'D',  'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',           // Third Row
    0,    '|', 'Z', 'X',  'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,   '*',      // Fourth Row
    ALT,  ' ', 0,   CAPS, 0,   0,   0,   0,   0,   0,   0,   LF,  0,   0,        // Misc Keys
};

/* Public Functions */

typedef enum { FSM_WAIT = 0, FSM_NEW_BYTE, FSM_TRANSLATE_BYTE, FSM_OUTPUT_BYTE } fsm_state_t;

char scan_code_set2_to_ascii( uint8_t scan_code )
{
    static fsm_state_t fsm_state = FSM_WAIT;

    static uint8_t scan_code_set_2_bytes[2] = { 0 };
    static uint8_t scan_code_set_2_bytes_index = 0;

    static uint8_t ascii_code = 0;

    switch ( fsm_state )
    {
        case FSM_WAIT:
    }

    /*** End of File ***/