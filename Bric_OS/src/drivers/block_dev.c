///** @file block_dev.c
// *
// * @brief
// *
// * @author Bryce Melander
// * @date Feb-20-2024
// *
// * @copyright (c) 2024 by Bryce Melander under MIT License. All rights reserved.
// * (See http://opensource.org/licenses/MIT for more details.)
// */

#include "block_dev.h"

#include "common.h"

// #define ATA_PORT ( 0x01F0U )  // 0x01F0-0x01F7

//// 512 Byte blocks
// #define BLOCK_DEV_SIZE ( 512U )
//// Read ATA as an array of uint16_t containing 256 entries
// #define ATA_READ_LEN ( 256U )

// int BD_read( BlockDevice_t* a, int offset, void* dst );
// int BD_write( BlockDevice_t* a, int offset, void* src );

//
// struct __BD_vtable_s
//{
//    int ( *read )( BlockDevice_t*, int, void* );
//    int ( *write )( BlockDevice_t*, int, void* );
//} BD_vtable = { &BD_read, &BD_write };

// int register_BlockDevice( BlockDevice_t* BD )
//{
//     FILE* curr_fp = FS_head;

//    while ( curr_fp != NULL )
//    {
//        if ( !curr_fp->probe( curr_fp, BD ) )
//        {
//            // probe_partition_MBR( BD );
//        }
//    }

//    return 0;
//}

//// Block Device Constructor
// BlockDevice_t* BlockDevice_init( BlockDevice_t* self )
//{
//     self->blk_size = BLOCK_DEV_SIZE;
//     self->total_len = 0;
//     self->table->read = NULL;
//     self->table->write = NULL;

//    return self;
//}

// int ATAD_read( BlockDevice_t* a, int offset, void* dst );
// int ATAD_write( BlockDevice_t* a, int offset, void* src );

//// ATA Device Constructor
// ATADevice_t* ATADevice_init( ATADevice_t* self, char* name )
//{
//     BlockDevice_init( &self->block_dev );
//     strcpy( self->name, name );  // NOLINT

//    self->block_dev.table->read = &ATAD_read;
//    self->block_dev.table->write = &ATAD_write;

//    return self;
//}

// int ATAD_read( BlockDevice_t* a, int offset, void* dst )  // NOLINT
//{
//     ATADevice_t* self = (ATADevice_t*)a;  // NOLINT

//    uint16_t i, data[ATA_READ_LEN];

//    //! This is extrememly inefficient, use DMA?
//    for ( i = 0; i < ATA_READ_LEN; ++i )
//    {
//        data[i] = inw( ATA_PORT );
//    }

//    return 0;
//}

// int ATAD_write( BlockDevice_t* a, int offset, void* src )  // NOLINT
//{
//     ATADevice_t* self = (ATADevice_t*)a;

//    //! Example
//    char arr[4] = { 0 };
//    self->block_dev.table->read( &self->block_dev, 2, &arr[offset] );

//    return 0;
//}

///*** End of File ***/