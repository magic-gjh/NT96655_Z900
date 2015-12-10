#ifndef CYGONCE_DISK_H
#define CYGONCE_DISK_H
// ====================================================================
//
//      disk.h
//
//      Device I/O
//
// ====================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Savin Zlobec
// Copyright (C) 2004, 2005 eCosCentric Limited
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
// ====================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     savin
// Original data: gthomas
// Date:          2003-06-09
// Purpose:       Internal interfaces for disk I/O drivers
// Description:
//
//####DESCRIPTIONEND####
//
// ====================================================================

// Disk I/O interfaces

#include <pkgconf/system.h>
#include <pkgconf/io_disk.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/io/devtab.h>
#include <cyg/io/io.h>
#include <cyg/hal/drv_api.h>
#include <string.h> /* memset() */

// ---------------------------------------------------------------------------

typedef struct cyg_disk_partition_t cyg_disk_partition_t;
typedef struct cyg_disk_info_t cyg_disk_info_t;
typedef struct cyg_disk_identify_t cyg_disk_identify_t;

typedef struct disk_channel    disk_channel;
typedef struct disk_controller disk_controller;
typedef struct disk_funs       disk_funs;

// ---------------------------------------------------------------------------

// Pointers into upper-level driver
typedef struct {

    // Initialize the disk
    cyg_bool (*disk_init)(struct cyg_devtab_entry *tab);

    // Disk device has been connected
    Cyg_ErrNo (*disk_connected)(struct cyg_devtab_entry *tab,
                                cyg_disk_identify_t     *ident);

    // Disk device has been disconnected
    Cyg_ErrNo (*disk_disconnected)(struct disk_channel *chan);

    // Lookup disk device
    Cyg_ErrNo (*disk_lookup)(struct cyg_devtab_entry **tab,
                             struct cyg_devtab_entry  *sub_tab,
                             const char               *name);

    // Asynchronous block transfer done
    void (*disk_transfer_done)(struct disk_channel *chan, Cyg_ErrNo res);

} disk_callbacks_t;

#define DISK_CALLBACKS(_l,                      \
                       _init,                   \
                       _connected,              \
                       _disconnected,           \
                       _lookup,                 \
                       _transfer_done)          \
disk_callbacks_t _l = {                         \
    _init,                                      \
    _connected,                                 \
    _disconnected,                              \
    _lookup,                                    \
    _transfer_done                              \
};

extern disk_callbacks_t cyg_io_disk_callbacks;

// ---------------------------------------------------------------------------
// Private data which describes a disk controller

struct disk_controller {
    cyg_drv_mutex_t     lock;           // Per-controller lock
    cyg_drv_cond_t      queue;          // Access wait list
    cyg_drv_cond_t      async;          // Async transfer waits here
    void                *priv;          // Private data
    volatile Cyg_ErrNo  result;         // Last operation result
    cyg_bool            init;           // Initialized?
    volatile cyg_bool   busy;           // Busy?
};

#define DISK_CONTROLLER(_l, _priv)              \
disk_controller _l = {                          \
    priv:       &_priv,                         \
    init:       false,                          \
    busy:       false                           \
}

// ---------------------------------------------------------------------------
// Private data which describes this channel

struct disk_channel {
    disk_funs               *funs;
    disk_callbacks_t        *callbacks;
    void                    *dev_priv;    // device private data
    disk_controller         *controller;  // pointer to controller
    cyg_disk_info_t         *info;        // disk info
    cyg_disk_partition_t    *partition;   // partition data
    cyg_bool                 mbr_support; // true if disk has MBR
    cyg_bool                 valid;       // true if device valid
    cyg_bool                 init;        // true if initialized
    cyg_ucount16             mounts;      // count of number of mounts
};

// Initialization macro for disk channel
#define DISK_CHANNEL(_l,                        \
                     _funs,                     \
                     _dev_priv,                 \
                     _controller,               \
                     _mbr_supp)                 \
cyg_disk_info_t _l##_disk_info;                 \
disk_channel _l = {                             \
    &(_funs),                                   \
    &cyg_io_disk_callbacks,                     \
    &(_dev_priv),                               \
    &(_controller),                             \
    &(_l##_disk_info),                          \
    NULL,                                       \
    _mbr_supp,                                  \
    false,                                      \
    false,                                      \
    0                                           \
};

// Initialization macro for disk channel allocated elsewhere.
#define DISK_CHANNEL_INIT(_dc,                  \
                     _funs,                     \
                     _dev_priv,                 \
                     _controller,               \
                     _disk_info,                \
                     _mbr_supp)                 \
    CYG_MACRO_START                             \
    memset(&(_disk_info), 0, sizeof(cyg_disk_info_t)); \
    (_dc).funs = &(_funs);                      \
    (_dc).callbacks = &cyg_io_disk_callbacks;   \
    (_dc).dev_priv = (_dev_priv);               \
    (_dc).controller = &(_controller);          \
    (_dc).info = &(_disk_info);                 \
    (_dc).partition = NULL;                     \
    (_dc).mbr_support = (_mbr_supp);            \
    (_dc).valid = false;                        \
    (_dc).init = false;                         \
    (_dc).mounts = 0;                           \
    CYG_MACRO_END

// ---------------------------------------------------------------------------
// Low level interface functions

struct disk_funs {

    // Read block data into buf
    Cyg_ErrNo (*read)(disk_channel *priv,
                      void         *buf,
                      cyg_uint32    len,
                      cyg_uint32    block_num);

    // Write block data from buf
    Cyg_ErrNo (*write)(disk_channel *priv,
                       const void   *buf,
                       cyg_uint32    len,
                       cyg_uint32    block_num);

    // Get hardware configuration
    Cyg_ErrNo (*get_config)(disk_channel *priv,
                            cyg_uint32    key,
                            const void   *xbuf,
                            cyg_uint32   *len);

    // Set hardware configuration
    Cyg_ErrNo (*set_config)(disk_channel *priv,
                            cyg_uint32    key,
                            const void   *xbuf,
                            cyg_uint32   *len);
};

#define DISK_FUNS(_l,_read,_write,_get_config,_set_config)           \
disk_funs _l = {                                                     \
  _read,                                                             \
  _write,                                                            \
  _get_config,                                                       \
  _set_config                                                        \
};

extern cyg_devio_table_t cyg_io_disk_devio;

// ---------------------------------------------------------------------------

typedef enum disk_partition_type_e
{
    DISK_EMPTY                          = 0,
    DISK_FAT12                          = 0x1,
    DISK_FAT16_DOS30                    = 0x4,
    DISK_FAT16_DOS33_EXTENDED	        = 0x5,
    DISK_FAT16_DOS331                   = 0x6,
    DISK_NTFS                           = 0x7,
    DISK_FAT32_WIN95 			        = 0xb,
    DISK_FAT32_WIN95_LBA 		        = 0xc,
    DISK_FAT16_WIN95_LBA 		        = 0xe,
    DISK_FAT32_WIN95_EXTENDED           = 0xf
} disk_partition_type_t;

// ---------------------------------------------------------------------------

#include <cyg/io/diskio.h>

// ---------------------------------------------------------------------------
#endif // CYGONCE_DISK_H
