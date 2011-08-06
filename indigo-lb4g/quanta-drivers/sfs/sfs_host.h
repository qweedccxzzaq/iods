/*
 * sfs_host.h
 * 
 * Header file for SFS implementation for testing on Linux host
 * 
 * Copyright (c) 2009, Leland Stanford Junior University
 *
 * Distributed under the OpenFlow license.
 *
 * No warranty implied or expressed.  Use at your own risk.
 */

#if !defined(OPENFLOW_SFS_HOST_H)
#define OPENFLOW_SFS_HOST_H

#include "sfs.h"

/* Default offset (1M), SFS allocation size (256K), tot flash size (8M) */
#define SFS_DEFAULT_OFFSET      (4 * 256 * 1024)
#define SFS_ALLOC_BYTES         (256 * 1024)
#define SFS_FLASH_BYTES         (8 * 1024 * 1024)

#define SFS_SAFE_OFFSET(base) 1

extern ssize_t fake_flash_write(uint32_t base_offset, 
                                const unsigned char *buf, uint32_t len);

extern ssize_t fake_flash_read(uint32_t base_offset, 
                               unsigned char *buf, uint32_t len);

#endif /* !defined(OPENFLOW_SFS_HOST_H) */

