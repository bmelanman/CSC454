/** @file types.h
 *
 * @brief
 *
 * @author Bryce Melander
 * @date Feb-22-2024
 *
 * @copyright (c) 2024 by Bryce Melander under MIT License. All rights reserved.
 * (See http://opensource.org/licenses/MIT for more details.)
 */

#ifndef TYPES_H
# define TYPES_H

# include <stdint.h>

typedef enum { SUCCESS = 0, FAILURE = 1 } driver_status_t;

typedef unsigned int uint;

typedef uint64_t ino64_t; /* 64-bit inode type */
typedef ino64_t ino_t;    /* inode type        */

typedef int64_t blkcnt_t;  /* Total blocks */
typedef int32_t blksize_t; /* Block size   */

typedef int32_t dev_t;   /* Device number         */
typedef uint16_t mode_t; /* File attributes       */
typedef int32_t pid_t;   /* Process and group IDs */

typedef uint32_t id_t; /* uid_t and gid_t */
typedef id_t uid_t;    /*                 */
typedef id_t gid_t;    /*                 */

typedef long off_t;       /* Used for file sizes */
typedef long long loff_t; /*                     */
typedef int64_t off64_t;  /*                     */

typedef uint32_t sigset_t; /* signal set */

typedef long int time64_t;   /* 64-bit signed time in seconds */
typedef long long ntime64_t; /* 64-bit signed time in nanoseconds */

// POSIX.1b time value structure
struct timespec
{
    time64_t tv_sec;   /* Seconds.  */
    ntime64_t tv_nsec; /* Nanoseconds.  */
};

typedef unsigned char uuid_t[16];

typedef unsigned int fsfilcnt_t; /* Used by statvfs and fstatvfs */
typedef unsigned int fsblkcnt_t; /* Used by statvfs and fstatvfs */

#endif /* TYPES_H */

/*** End of File ***/