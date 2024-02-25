///** @file fs.h
// *
// * @brief
// *
// * @author Bryce Melander
// * @date Feb-20-2024
// *
// * @copyright (c) 2024 by Bryce Melander under MIT License. All rights reserved.
// * (See http://opensource.org/licenses/MIT for more details.)
// */

// #ifndef FS_H
// # define FS_H

// # include "block_dev.h"
// # include "common.h"

// typedef struct __file_system_s FileSystem_t;

// struct __file_system_s
//{
//     int ( *probe )( FileSystem_t *self, BlockDevice_t *BD );
// };

// typedef void ( *readdir_callback_t )( void *, const char *, uint32_t );

// struct cpe453fs_ops
//{
//     // This void * is passed as the first argument to all function calls
//     void *arg;

//    // Functions necessary for the first version of the file system assignment,
//    // a read-only file system
//    int ( *getattr )( void *, ino_t inode, struct stat *stbuf );
//    int ( *readdir )( void *, ino_t inode, void *buf, readdir_callback_t cb );
//    int ( *open )( void *, ino_t inode );
//    int ( *read )( void *, ino_t inode, char *buff, size_t size, off_t offset );
//    int ( *readlink )( void *, ino_t inode, char *buff, size_t buff_size );
//    uint32_t ( *root_node )( void * );
//    void ( *set_file_descriptor )( void *, int );

//    // Functions necessary for a read-write file system.  I suggest implementing
//    // them in the order found in this header file.
//    int ( *chmod )( void *, ino_t inode, mode_t new_mode );
//    int ( *chown )( void *, ino_t inode, uid_t new_uid, gid_t new_gid );
//    int ( *utimens )( void *, ino_t inode, const struct timespec tv[2] );
//    int ( *rmdir )( void *, ino_t inode, const char *name );
//    int ( *unlink )( void *, ino_t inode, const char *name );
//    int ( *mknod )( void *, ino_t parent_inode, const char *name, mode_t new_mode, dev_t new_dev
//    ); int ( *symlink )( void *, ino_t parent_inode, const char *name, const char *link_dest );
//    int ( *mkdir )( void *, ino_t parent_inode, const char *name, mode_t new_mode );
//    int ( *link )( void *, ino_t parent_inode, const char *name, uint32_t dest_block );
//    int ( *rename )(
//        void *, uint32_t old_parent, const char *old_name, uint32_t new_parent, const char
//        *new_name
//    );
//    int ( *truncate )( void *, ino_t inode, off_t new_size );
//    int ( *write )( void *, ino_t inode, const char *buff, size_t wr_len, off_t wr_offset );

//    // Optional functions

//    // Called when the file system is first initialized by FUSE
//    void ( *init )( void );
//    // Called when the file system is unmounted, but before the program exits
//    void ( *destroy )( void );
//};

// #endif /* FS_H */

///*** End of File ***/