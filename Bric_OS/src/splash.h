/** @file splash.h
 *
 * @brief Header for the splash screen.
 *
 * @author Bryce Melander
 * @date Jan-19-2024
 *
 * @copyright (c) 2024 by Bryce Melander under MIT License. All rights reserved.
 * (See http://opensource.org/licenses/MIT for more details.)
 */

#ifndef SPLASH_H
# define SPLASH_H

// ░ - 25% | ▒ - 50% | ▓ - 75% | █ - 100%

// # define SQUARE_25    ( (char)0xB0 )  // ░
// # define SQUARE_50    ( (char)0xB1 )  // ▒
// # define SQUARE_75    ( (char)0xB2 )  // ▓
// # define SQUARE_100   ( (char)0xDB )  // █
// # define HALF_SQR_BTM ( (char)0xDC )  // ▄
// # define HALF_SQR_TOP ( (char)0xDF )  // ▀

# define SQUARE_25    ( '*' )  // ░
# define SQUARE_50    ( '$' )  // ▒
# define SQUARE_75    ( '#' )  // ▓
# define SQUARE_100   ( '0' )  // █
# define HALF_SQR_BTM ( '.' )  // ▄
# define HALF_SQR_TOP ( '"' )  // ▀

void __attribute__( ( weak ) ) splash_screen( void )
{
    // "________________________________________________________________________________"
    // "___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|"
    // "_|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|__"
    // "___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|"
    // "_|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|__"
    // "___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|"
    // "_|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|__"
    // "___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|"
    // "_|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|__"
    // "___|___|___|___|___|___▄▄▄▄|___██▀███__|██_|▄████▄_|___|___|▒█████_|___██████__|" 1
    // "_|___|___|___|___|___|▓██▀▀█▄__██|▒_█|_▓██__██▀_▀█___|___|__▒██▒_|██▒|██_|__▒|__" 2
    // "___|___|___|___|___|__▒██__▄█▒_██_░▄▓__▒██▒|▓█_|__▄|___|___|▒██░__██▒_▒▓██▄|___|" 3
    // "_|___|___|___|___|___|▒██▀█▀_|_██▀▀█▄|_░██▒_▓▓▄_▄██▒_|___|__▒██__|██░|_▒_|_██▒__" 4
    // "___|___|___|___|___|__░▓█__▀█▓_██__|██_░██░|_▓███▀_░_█████▓|░_████▓▒░▒██████▒▒_|" 5
    // "_|___|___|___|___|___|░▒▓███▀▒_▒▓|__▒▓__▓  ░_░▒_▒|_░_▓▒░▒|░_░|▒░▒░▒░_▒_▒▓▒_▒_░__" 6
    // "___|___|___|___|___|__▒░▒__|░▒_|▒▒_░_▒░|▒_░|_░_|▒__|_▒░|▒_░|__░|▒_▒░_░_░▒__░_░_|" 7
    // "_|___|___|___|___|___|_░_|__░░__░░___░__▒|░░_|___|___▒░_░|__░|░ ░|▒__░__░|_░_|__" 8
    // "___|___|___|___|___|___░___|░__|_░_|___|░__░_░_|___|_░_|░__|___|░_░|___|___░___|" 9
    // "_|___|___|___|___|___|___|___░___|___|___|_░_|___|___░___|___|___|___|___|___|__" A
    // "___|___|___|___|___|___|___|░__|___|___|___|___|___|___|___|___|___|___|___|___|" B

    printk(
        "________________________________________________________________________________"
        "___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|"
        "_|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|__"
        "___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|"
        "_|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|__"
        "___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|"
        "_|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|__"
        "___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|"
        "_|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|__"
    );

    // Row 1
    printk(
        "___|___|___|___|___|___%c%c%c%c|___%c%c%c%c%c%c__|%c%c_|%c%c%c%c%c%c_|___|___|%c%c%c%c%c"
        "%c_|___%c%c%c%c%c%c__|",
        // "___|___|___|___|___|___▄▄▄▄|___██▀███__|██_|▄████▄_|___|___|▒█████_|___██████__|" 1
        HALF_SQR_BTM, HALF_SQR_BTM, HALF_SQR_BTM, HALF_SQR_BTM,                      // R1-1
        SQUARE_100, SQUARE_100, HALF_SQR_TOP, SQUARE_100, SQUARE_100, SQUARE_100,    // R1-2
        SQUARE_100, SQUARE_100,                                                      // R1-3
        HALF_SQR_BTM, SQUARE_100, SQUARE_100, SQUARE_100, SQUARE_100, HALF_SQR_BTM,  // R1-4
        SQUARE_50, SQUARE_100, SQUARE_100, SQUARE_100, SQUARE_100, SQUARE_100,       // R1-7
        SQUARE_100, SQUARE_100, SQUARE_100, SQUARE_100, SQUARE_100, SQUARE_100       // ROW 1 END

    );

    // Row 2
    printk(
        "_|___|___|___|___|___|%c%c%c%c%c%c%c__%c%c|%c_%c|_%c%c%c__%c%c%c_%c%c___|___|__%c%c%c%c_|"
        "%c%c%c|%c%c_|__%c|__",
        // "_|___|___|___|___|___|▓██▀▀█▄__██|▒_█|_▓██__██▀_▀█___|___|__▒██▒_|██▒|██_|__▒|__" 2
        SQUARE_75, SQUARE_100, SQUARE_100, HALF_SQR_TOP, HALF_SQR_TOP, SQUARE_100,
        HALF_SQR_BTM,                                  // R2-1
        SQUARE_100, SQUARE_100,                        // R2-2
        SQUARE_50,                                     // R2-3
        SQUARE_100,                                    // R2-4
        SQUARE_75, SQUARE_100, SQUARE_100,             // R2-5
        SQUARE_100, SQUARE_100, HALF_SQR_TOP,          // R2-6
        HALF_SQR_TOP, SQUARE_100,                      // R2-7
        SQUARE_50, SQUARE_100, SQUARE_100, SQUARE_50,  // R2-8
        SQUARE_100, SQUARE_100, SQUARE_50,             // R2-9
        SQUARE_100, SQUARE_100,                        // R2-10
        SQUARE_50                                      // ROW 2 END
    );

    // Row 3
    printk(
        "___|___|___|___|___|__%c%c%c__%c%c%c_%c%c_%c%c%c__%c%c%c%c|%c%c_|__%c|___|___|%c%c%c%c__%c"
        "%c%c_%c%c%c%c%c|___|",

        //                        1    2   3  4    5    6     7         8     9   10
        // "___|___|___|___|___|__▒██__▄█▒_██_░▄▓__▒██▒|▓█_|__▄|___|___|▒██░__██▒_▒▓██▄|___|" 3
        SQUARE_50, SQUARE_100, SQUARE_100,                          // R3-1
        HALF_SQR_TOP, SQUARE_100, SQUARE_50,                        // R3-2
        SQUARE_100, SQUARE_100,                                     // R3-3
        SQUARE_25, HALF_SQR_BTM, SQUARE_75,                         // R3-4
        SQUARE_50, SQUARE_100, SQUARE_100, SQUARE_50,               // R3-5
        SQUARE_75, SQUARE_100,                                      // R3-6
        HALF_SQR_BTM,                                               // R3-7
        SQUARE_50, SQUARE_100, SQUARE_100, SQUARE_25,               // R3-8
        SQUARE_100, SQUARE_100, SQUARE_50,                          // R3-9
        SQUARE_50, SQUARE_75, SQUARE_100, SQUARE_100, HALF_SQR_BTM  // ROW 3 END

    );

    // Row 4
    printk(
        "_|___|___|___|___|___|%c%c%c%c%c%c_|_%c%c%c%c%c%c|_%c%c%c%c_%c%c%c_%c%c%c%c_|___|__%c%c%c_"
        "_|%c%c%c|_%c_|_%c%c%c__",

        //                        1        2       3    4   5           6     7    8   9
        // "_|___|___|___|___|___|▒██▀█▀_|_██▀▀█▄|_░██▒_▓▓▄_▄██▒_|___|__▒██__|██░|_▒_|_██▒__" 4
        SQUARE_50, SQUARE_100, SQUARE_100, HALF_SQR_TOP, SQUARE_100, HALF_SQR_TOP,     // R4-1
        SQUARE_100, SQUARE_100, HALF_SQR_TOP, HALF_SQR_TOP, SQUARE_100, HALF_SQR_BTM,  // R4-2
        SQUARE_25, SQUARE_100, SQUARE_100, SQUARE_50,                                  // R4-3
        SQUARE_75, SQUARE_75, HALF_SQR_BTM,                                            // R4-4
        HALF_SQR_BTM, SQUARE_100, SQUARE_100, SQUARE_50,                               // R4-5
        SQUARE_50, SQUARE_100, SQUARE_100,                                             // R4-6
        SQUARE_100, SQUARE_100, SQUARE_25,                                             // R4-7
        SQUARE_50,                                                                     // R4-8
        SQUARE_100, SQUARE_100, SQUARE_50                                              // ROW 4 END

    );

    // Row 5
    printk(
        "___|___|___|___|___|__%c%c%c__%c%c%c_%c%c__|%c%c_%c%c%c%c|_%c%c%c%c%c_%c_%c%c%c%c%c%c|%c_"
        "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c_|",

        SQUARE_25, SQUARE_75, SQUARE_100, HALF_SQR_TOP, SQUARE_100, SQUARE_75, SQUARE_100,
        SQUARE_100, SQUARE_100, SQUARE_100, SQUARE_25, SQUARE_100, SQUARE_100, SQUARE_25, SQUARE_75,
        SQUARE_100, SQUARE_100, SQUARE_100, HALF_SQR_TOP, SQUARE_25, SQUARE_100, SQUARE_100,
        SQUARE_100, SQUARE_100, SQUARE_100, SQUARE_75, SQUARE_25, SQUARE_100, SQUARE_100,
        SQUARE_100, SQUARE_100, SQUARE_75, SQUARE_50, SQUARE_25, SQUARE_50, SQUARE_100, SQUARE_100,
        SQUARE_100, SQUARE_100, SQUARE_100, SQUARE_100, SQUARE_50, SQUARE_50  // ROW 5 END
    );

    // Row 6
    printk(
        "_|___|___|___|___|___|%c%c%c%c%c%c%c%c_%c%c|__%c%c__%c  "
        "%c_%c%c_%c|_%c_%c%c%c%c|%c_%c|%c%c%c%c%c%c_%c_%c%c%c_%c_%c__",

        SQUARE_25, SQUARE_50, SQUARE_75, SQUARE_100, SQUARE_100, SQUARE_100, HALF_SQR_TOP,
        SQUARE_50, SQUARE_50, SQUARE_75, SQUARE_50, SQUARE_75, SQUARE_75, SQUARE_25, SQUARE_25,
        SQUARE_50, SQUARE_50, SQUARE_25, SQUARE_75, SQUARE_50, SQUARE_25, SQUARE_50, SQUARE_25,
        SQUARE_25, SQUARE_50, SQUARE_25, SQUARE_50, SQUARE_25, SQUARE_50, SQUARE_25, SQUARE_50,
        SQUARE_50, SQUARE_75, SQUARE_50, SQUARE_50, SQUARE_25
    );

    // Row 7
    printk(
        "___|___|___|___|___|__%c%c%c__|%c%c_|%c%c_%c_%c%c|%c_%c|_%c_|%c__|_%c%c|%c_%c|__%c|%c_%c"
        "%c_%c_%c%c__%c_%c_|",

        SQUARE_50, SQUARE_25, SQUARE_50, SQUARE_25, SQUARE_50, SQUARE_50, SQUARE_50, SQUARE_25,
        SQUARE_50, SQUARE_25, SQUARE_50, SQUARE_25, SQUARE_25, SQUARE_50, SQUARE_50, SQUARE_25,
        SQUARE_50, SQUARE_25, SQUARE_25, SQUARE_50, SQUARE_50, SQUARE_25, SQUARE_25, SQUARE_25,
        SQUARE_50, SQUARE_25, SQUARE_25
    );

    // Row 8
    printk(
        "_|___|___|___|___|___|_%c_|__%c%c__%c%c___%c__%c|%c%c_|___|___%c%c_%c|__%c|%c "
        "%c|%c__%c__%c|_%c_|__",
        SQUARE_25, SQUARE_25, SQUARE_25, SQUARE_25, SQUARE_25, SQUARE_25, SQUARE_50, SQUARE_25,
        SQUARE_25, SQUARE_50, SQUARE_25, SQUARE_25, SQUARE_25, SQUARE_25, SQUARE_25, SQUARE_50,
        SQUARE_25, SQUARE_25, SQUARE_25
    );

    // Row 9 - B
    printk(
        "___|___|___|___|___|___%c___|%c__|_%c_|___|%c__%c_%c_|___|_%c_|%c__|___|%c_%c|___|___"
        "%c___|"  // Row 9 End
        "_|___|___|___|___|___|___|___%c___|___|___|_%c_|___|___%c___|___|___|___|___|___|__"
        "___|___|___|___|___|___|___|%c__|___|___|___|___|___|___|___|___|___|___|___|___|"
        "\n",

        SQUARE_25, SQUARE_25, SQUARE_25, SQUARE_25, SQUARE_25, SQUARE_25, SQUARE_25, SQUARE_25,
        SQUARE_25, SQUARE_25, SQUARE_25,  // ROW 9
        SQUARE_25, SQUARE_25, SQUARE_25,  // ROW A
        SQUARE_25                         // ROW B
    );

    printk( "Welcome to Bric_OS!\n\n" );
}

#endif /* SPLASH_H */

/*** End of File ***/