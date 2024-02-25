///** @file fs.c
// *
// * @brief
// *
// * @author Bryce Melander
// * @date Feb-22-2024
// *
// * @copyright (c) 2024 by Bryce Melander under MIT License. All rights reserved.
// * (See http://opensource.org/licenses/MIT for more details.)
// */

// #include "fs.h"

//// Inode Class
////
//// Needs to support:
////   File Operations
////    - read
////    - seek
////    - write
////    - close
////   Dir Operations
////    - touch and mkdir
////    - rm and rmdir
////    - rename
////    - readdir

// #define SUPERBLK_MAGIC_LEN ( 1021U )

///* Superblock: The master block for the filesystem */
// #define SUPERBLOCK_BLK_NUM ( 0U )
// typedef struct __superblock_struct
//{
//     uint32_t type;
//     uint32_t magic[SUPERBLK_MAGIC_LEN];
//     uint32_t root_inode;
//     uint32_t freelist_head;
// } superblock_t;

// typedef struct __inode_struct
//{
//     uint64_t type;
//     uint64_t mode;
//     uint64_t nlink;
//     uint64_t uid;
//     uint64_t gid;
//     uint64_t rdev;
//     uint64_t usr_flags;
//     uint64_t t_acc_sec;
//     uint64_t t_acc_nsec;
//     uint64_t t_mod_sec;
//     uint64_t t_mod_nsec;
//     uint64_t t_stat_sec;
//     uint64_t t_stat_nsec;
//     uint64_t size;
//     // uint64_t nblks;
//     // uint8_t data[INODE_DATA_LEN];
//     // uint32_t nxt_blk;
//     uint64_t inode_id;
//     void* superblock;
// } inode_t;

///*** End of File ***/