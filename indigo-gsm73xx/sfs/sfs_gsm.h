/*
 * sfs_gsm.h
 * 
 * Header file for SFS implementation on GSM platform
 * 
 * Copyright (c) 2010, Big Switch Networks
 * Derived from work Copyright (c) 2009, Leland Stanford Junior University
 *
 * Distributed under the OpenFlow license.
 *
 * No warranty implied or expressed.  Use at your own risk.
 */

#if !defined(OPENFLOW_SFS_GSM_H)
#define OPENFLOW_SFS_GSM_H

#include "sfs.h"

/* Default offset is at 15 MB into flash; size is 1 MB */
#define SFS_DEFAULT_OFFSET    (15 * 1024 * 1024)
#define SFS_ALLOC_BYTES       (1024 * 1024)
#define SFS_FLASH_BYTES       (28 * 1024 * 1024)

#define SFS_OFFSET_LOW        (12 * 1024 * 1024)
#define SFS_OFFSET_HIGH       ((20 * 1024 * 1024) - SFS_ALLOC_BYTES)

/* Safe offset is between 12 and 20 MB */
#define SFS_SAFE_OFFSET(base) \
    (((base) >= SFS_OFFSET_LOW) && ((base) < SFS_OFFSET_HIGH))

extern ssize_t gsm_flash_write(uint32_t base_offset, 
                               const unsigned char *data, uint32_t len);
extern ssize_t gsm_flash_read(uint32_t base_offset, unsigned char *buf, 
                              uint32_t len);

#endif /*  !defined(OPENFLOW_SFS_GSM_H) */
