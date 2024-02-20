/** @file errno.c
 *
 * @brief Error Codes
 *
 * @author Bryce Melander
 * @date Feb-19-2024
 *
 * @copyright (C) 2008 The Android Open Source Project, All rights reserved.
 * @copyright (c) 2024 by Bryce Melander under MIT License. All rights reserved.
 * (See http://opensource.org/licenses/MIT for more details.)
 */

/* Global Variables */

static int errno;

/* Public Functions */
int *__geterrno( void ) { return &errno; }

/*** End of File ***/