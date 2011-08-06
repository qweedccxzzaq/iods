/*
 * sfs_lb4g.h
 * 
 * Header file for SFS implementation on LB4G platform
 * 
 * Copyright (c) 2009, Leland Stanford Junior University
 *
 * Distributed under the OpenFlow license.
 *
 * No warranty implied or expressed.  Use at your own risk.
 */

#if !defined(OPENFLOW_SFS_LB4G_H)
#define OPENFLOW_SFS_LB4G_H

#include "sfs.h"

/* Default offset (8M), SFS allocation size (256K), tot flash size (32M) */
#define SFS_DEFAULT_OFFSET    0x800000
#define SFS_ALLOC_BYTES       (256 * 1024)
#define SFS_FLASH_BYTES       (32 * 1024 * 1024)

#define SFS_SAFE_OFFSET(base) \
    (((base) >= 0x800000) && ((base) < (0x1000000 - SFS_ALLOC_BYTES)))

extern ssize_t lb4g_flash_write(uint32_t base_offset, 
                                const unsigned char *data, uint32_t len);
extern ssize_t lb4g_flash_read(uint32_t base_offset, unsigned char *buf, 
                               uint32_t len);

#endif /*  !defined(OPENFLOW_SFS_LB4G_H) */
