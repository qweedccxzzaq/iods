/*
 * sfs.h
 * 
 * Stupid file system
 * 
 * For a flash systems where we have raw read/write access, but
 * no mtd, etc.
 *
 * Very simple structure that allows the writing or extraction of
 * a directory of files into a flash sector
 * 
 * Copyright (c) 2009, Leland Stanford Junior University
 *
 * Distributed under the OpenFlow license.
 *
 * No warranty implied or expressed.  Use at your own risk.
 */

/*
 * SFS structure:
 *    Magic bytes (8)
 *    Header crc (4 bytes):  crc-32 across SFS header block except magic and crc
 *    Sequence of file-header + file-content
 *
 * Start of files are 4-byte aligned
 * Names must be at least one character long.
 */

#if !defined(OPENFLOW_SFS_H)
#define OPENFLOW_SFS_H

#include "crc32.h"

#define SFS_MAGIC {'S', 'F', 'S', ' ', 0x5f, 0x50, 0x01, 0x02}
#define SFS_MAGIC_BYTES 8

#define SFS_VERSION 0x1

#define SFS_FILENAME_MAX 32 /* File name length max w/ null termination */
#define SFS_FILES_MAX 64

/* Description of a file */
typedef struct sfs_file_header_s {
    char name[SFS_FILENAME_MAX];  /* Null terminated, null extended */
    uint32_t offset;         /* Relative to base_offset; 4 byte aligned */
    uint32_t length;         /* Bytes in file */
    crc_t crc;               /* Integrity check for file contents w/ header */
} sfs_file_header_t;

#define PAD_BYTES 100  /* Keeps 16 byte aligned, plus space for expansion */

/* Control block for an SFS file system; Keep this 16-byte aligned */
typedef struct sfs_header_s {
    char magic[SFS_MAGIC_BYTES];               /* SFS signature, see above */
    uint32_t hdr_crc;                          /* CRC for header */
    uint32_t version;                          /* Version of SFS */
    uint32_t file_count;                       /* Number of files active */
    uint32_t cur_bytes;                        /* Used bytes in FS w/ header */
    uint32_t tot_bytes;                        /* Tot bytes in FS w/ header */
    unsigned char reserved[PAD_BYTES];         /* For future expansion */
    sfs_file_header_t files[SFS_FILES_MAX];    /* Pointers to files */
} sfs_header_t;

typedef struct sfs_ram_fs_s {
    sfs_header_t *header;    /* Pointer to header in sfs_ram below */
    unsigned char *data_buf; /* Pointer to ram buffer with filesystem */
} sfs_ram_fs_t;

/* Driver functions for HW specific read/write ops; return bytes written/read */
typedef ssize_t (*sfs_hw_flash_write_f)(uint32_t base_offset, 
                                        const unsigned char *buf, uint32_t len);
typedef ssize_t (*sfs_hw_flash_read_f)(uint32_t base_offset, 
                                       unsigned char *buf, uint32_t len);

/*
 * sfs_config:            Set the HW flash read/write routines
 * sfs_ram_fs_init:       Init a RAM SFS
 * sfs_flash_read         Read into a RAM SFS from flash
 * sfs_flash_write        Write a RAM SFS to flash
 * sfs_file_append        Add a file to a RAM SFS
 * sfs_file_extract_all   Extract all files from a RAM SFS to a directory
 * sfs_dump               Debug dump routine
 */

extern int sfs_config(sfs_hw_flash_read_f f_read, 
                      sfs_hw_flash_write_f f_write);

extern int sfs_ram_fs_init(sfs_ram_fs_t *ram_fs, unsigned char *buf, int size);

extern int sfs_flash_read(uint32_t base, sfs_ram_fs_t *ram_fs);
extern int sfs_flash_write(uint32_t base, sfs_ram_fs_t *ram_fs);

extern int sfs_file_append(const char *filename, sfs_ram_fs_t *ram_fs);
extern int sfs_file_extract_all(const char *prefix, sfs_ram_fs_t *ram_fs);

extern void sfs_dump(sfs_ram_fs_t *ram_fs);

/****************************************************************/

#define SFS_DEBUG_NONE -1
#define SFS_DEBUG_ERR 0
#define SFS_DEBUG_WARN 1
#define SFS_DEBUG_VERB 2

extern int sfs_debug; /* Really, a library variable */
extern int sfs_debug_get(void);
extern void sfs_debug_set(int dbg);

#define SFS_DEBUG(lvl, fmt, args...) \
    if (sfs_debug >= (lvl)) printf(fmt, ## args)

#define SFS_ERR(fmt, args...) SFS_DEBUG(SFS_DEBUG_ERR, fmt, ## args) 
#define SFS_WARN(fmt, args...) SFS_DEBUG(SFS_DEBUG_WARN, fmt, ## args) 
#define SFS_VERB(fmt, args...) SFS_DEBUG(SFS_DEBUG_VERB, fmt, ## args)
#endif /*  !defined(OPENFLOW_SFS_H) */
