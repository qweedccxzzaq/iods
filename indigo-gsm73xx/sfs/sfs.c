/*
 * sfs.c
 * 
 * Stupid file system implementation
 * 
 * Copyright (c) 2009, Leland Stanford Junior University
 *
 * Distributed under the OpenFlow license.
 *
 * No warranty implied or expressed.  Use at your own risk.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>

#include "sfs.h"
#include "crc32.h"

/****************************************************************
 *
 * Library configurable functions
 *
 ****************************************************************/

static sfs_hw_flash_write_f sfs_hw_flash_write;
static sfs_hw_flash_read_f sfs_hw_flash_read;

/****************************************************************
 * 
 * Internal defines 
 *
 ****************************************************************/

int sfs_debug = SFS_DEBUG_ERR;

int
sfs_debug_get(void) 
{
    return sfs_debug;
}

void
sfs_debug_set(int dbg)
{
    sfs_debug = dbg;
}

/* Align to 16-byte boundary */
#define LEN_ALIGN(len) (((len) + 15) & 0xfffffff0)

#define HEADER_CRC_START(rfs) ((unsigned char *)(&((rfs)->header->file_count)))
#define HEADER_CRC_BYTES(rfs)  \
    (sizeof(sfs_file_header_t) * SFS_FILES_MAX + sizeof(uint32_t))
#define HEADER_CRC_GEN(rfs)    \
    (crc_update(0, HEADER_CRC_START(rfs), HEADER_CRC_BYTES(rfs)))

#define RFS_FILE(rfs, fidx) ((rfs)->header->files[fidx])
#define FILE_CRC_START(rfs, fidx) ((rfs)->data_buf + RFS_FILE(rfs, fidx).offset)
#define FILE_CRC_BYTES(rfs, fidx) (RFS_FILE(rfs, fidx).length)
#define FILE_CRC_GEN(rfs, fidx)    \
    (crc_update(0, FILE_CRC_START(rfs, fidx), FILE_CRC_BYTES(rfs, fidx)))

static unsigned char sfs_magic[SFS_MAGIC_BYTES] = SFS_MAGIC;


/* Check a RAM version of the filesystem; Return 0 if okay; < 0 on error */
static int 
sfs_integrity_check(sfs_ram_fs_t *ram_fs, int check_files)
{
    int i;
    crc_t crc;
    sfs_file_header_t *f;
    uint32_t prev_file_end = 0;
    sfs_header_t *hdr;

    hdr = ram_fs->header;

    /* Check magic */
    if (memcmp(sfs_magic, hdr->magic, SFS_MAGIC_BYTES) != 0) { 
        SFS_ERR("Failed SFS signature check\n");
        return -1;
   }

    /* Generate header crc and check */
    crc = HEADER_CRC_GEN(ram_fs);
    if (crc != hdr->hdr_crc) {
        SFS_ERR("Failed CRC check:  %d != %d\n", hdr->hdr_crc, crc);
        return -1;
    }

    if (hdr->version != SFS_VERSION) {
        SFS_ERR("Failed version check:  FS is version %d\n", 
              hdr->version);
        return -1;
    }

    if (hdr->file_count > SFS_FILES_MAX) {
        SFS_ERR("Failed integrity:  Too many files %d\n", 
              hdr->file_count);
        return -1;
    }

    if (check_files) {
        /* Iterate across files, generate crc and check */
        for (i = 0; i < hdr->file_count; i++) {
            f = &hdr->files[i];
            if (f->length > hdr->cur_bytes) {
                SFS_ERR("ERROR in length of file %d: %d\n", i, f->length);
                return -1;
            }
            if (f->offset < prev_file_end) {
                SFS_ERR("ERROR: offset 0x%x of file %d overlaps previous file\n",
                      f->offset, i);
                return -1;
            }
            crc = FILE_CRC_GEN(ram_fs, i);
            if (crc != f->crc) {
                SFS_ERR("Failed file CRC check for %d:  %d != %d\n", 
                      i, f->crc, crc);
                return -1;
            }
            prev_file_end = f->length + f->offset;
        }
    }

    return 0;
}

void 
sfs_dump(sfs_ram_fs_t *ram_fs)
{
    int i;
    sfs_header_t *hdr;

    hdr = ram_fs->header;
    printf("Dump of ram SFS at %p\n", ram_fs);
    printf("  FS pointer %p, header pointer %p\n", ram_fs->data_buf,
           ram_fs->header);
    if ((void *)(ram_fs->header) != (void *)(ram_fs->data_buf)) {
        printf("WARNING:  These should be equal\n");
    }
    printf("  Header:\n");
    printf("    FS Currently contains %d bytes\n", hdr->cur_bytes);
    printf("    FS has %d bytes allocated\n", hdr->tot_bytes);
    printf("    magic: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
           hdr->magic[0], hdr->magic[1], hdr->magic[2], hdr->magic[3],
           hdr->magic[4], hdr->magic[5], hdr->magic[6], hdr->magic[7]);
    printf("    crc: %d, 0x%x\n", hdr->hdr_crc, hdr->hdr_crc);
    printf("    %d files\n", hdr->file_count);
    printf("  Files:\n");
    for (i = 0; i < hdr->file_count; i++) {
        sfs_file_header_t *f;
        unsigned char *ptr;

        f = &hdr->files[i];
        ptr = &ram_fs->data_buf[f->offset];
        printf("    File %d: %s\n", i, f->name);
        printf("      Offset %d. Length %d. crc 0x%x\n", f->offset,
               f->length, f->crc);
        printf("      Data[0:15]: %02x%02x%02x%02x %02x%02x%02x%02x "
               "%02x%02x%02x%02x %02x%02x%02x%02x\n", ptr[0], ptr[1], 
               ptr[2], ptr[3], ptr[4], ptr[5], ptr[6], ptr[7], ptr[8], 
               ptr[9], ptr[10], ptr[11], ptr[12], ptr[13], ptr[14], ptr[15]);
    }
}

/* FIXME */
#define IS_ALIGNED(buf) 1

/*
 * Configure the SFS flash routines.
 */

int
sfs_config(sfs_hw_flash_read_f f_read, sfs_hw_flash_write_f f_write)
{
    if (f_write == NULL || f_read == NULL) {
        SFS_ERR("sfs_config requires fn pntrs for flash read and commmit\n");
        return -1;
    }
    sfs_hw_flash_write = f_write;
    sfs_hw_flash_read = f_read;

    return 0;
}

int
sfs_ram_fs_init(sfs_ram_fs_t *ram_fs, unsigned char *buf, int size)
{
    sfs_header_t *hdr;

    if (!IS_ALIGNED(buf)) {
        SFS_ERR("ERROR in sfs_ram_fs_init: buf is not aligned\n");
        return -1;
    }

    memset(buf, 0, size);
    ram_fs->header = (sfs_header_t *)buf;
    ram_fs->data_buf = buf;
    hdr = ram_fs->header;
    hdr->cur_bytes = sizeof(sfs_header_t);
    hdr->tot_bytes = size;
    memcpy(hdr->magic, sfs_magic, SFS_MAGIC_BYTES);
    hdr->version = SFS_VERSION;
    hdr->hdr_crc = HEADER_CRC_GEN(ram_fs);

    return 0;
}

/* FS create routines */

#define CHUNK_BYTES 1024

/* Add the contents of file named filename to the ram_fs structure */
int
sfs_file_append(const char *filename, sfs_ram_fs_t *ram_fs)
{
    sfs_file_header_t *f;
    sfs_header_t *hdr;
    unsigned char *ptr;
    uint32_t file_bytes = 0;
    ssize_t read_bytes;
    uint32_t free_bytes;
    size_t to_read;
    int fd;

    SFS_VERB("Adding file %s to SFS\n", filename);
    if (sfs_integrity_check(ram_fs, 1) < 0) {
        SFS_ERR("Failed integrity check\n");
        return -1;
    }
    hdr = ram_fs->header;
    if (hdr->file_count >= SFS_FILES_MAX) {
        SFS_ERR("Too many files in FS.\n");
        return -1;
    }
    if (strlen(filename) > SFS_FILENAME_MAX) {
        SFS_ERR("Filename %s too long\n", filename);
        return -1;
    }
    f = &hdr->files[hdr->file_count];
    ptr = &ram_fs->data_buf[hdr->cur_bytes];
    free_bytes = hdr->tot_bytes - hdr->cur_bytes;

    fd = open(filename, O_RDONLY);
    if (fd < 0) {
        SFS_ERR("Failed to open file %s\n", filename);
        return -1;
    }
    while (1) {
        to_read = free_bytes < CHUNK_BYTES ? free_bytes : CHUNK_BYTES;
        read_bytes = read(fd, ptr, to_read);
        if (read_bytes < 0) {
            SFS_ERR("Failure reading from %s\n", filename);
            close(fd);
            return -1;
        } else if (read_bytes == 0) {
            break;
        }

        free_bytes -= read_bytes;
        ptr += read_bytes;
        file_bytes += read_bytes;
    }
    close(fd);

    SFS_VERB("Read in %d bytes for file %s\n", file_bytes, filename);

    f->offset = hdr->cur_bytes;
    f->length = file_bytes;
    f->crc = FILE_CRC_GEN(ram_fs, hdr->file_count);
    strcpy(f->name, filename);
    hdr->cur_bytes += LEN_ALIGN(file_bytes);
    hdr->file_count++;
    hdr->hdr_crc = HEADER_CRC_GEN(ram_fs);

    return 0;
}

static int
extract_file(char *full_name, char *name, sfs_file_header_t *f, 
             unsigned char *buf)
{
    unsigned char *ptr;
    int fd;
    ssize_t bytes = 0;

    ptr = &buf[f->offset];
    strcpy(name, f->name);
    SFS_VERB("Writing file %s\n", full_name);

    fd = open(full_name, O_WRONLY | O_CREAT | O_TRUNC);
    if (fd < 0) {
        SFS_ERR("Failed to open file %s for writing\n", full_name);
        return -1;
    }
    bytes = write(fd, ptr, f->length);
    if (bytes != f->length) {
        SFS_WARN("Warning: Wrote %d bytes, not %d bytes\n", 
                 (int)bytes, f->length);
    }

    close(fd);

    return 0;
}

static char full_name[256];

/* Extract all files from ram_fs to the dir given by prefix (may be NULL) */
int
sfs_file_extract_all(const char *prefix, sfs_ram_fs_t *ram_fs)
{
    sfs_header_t *hdr;
    char *name;
    int i;

    name = full_name;
    if (prefix && prefix[0]) {
        int slen;

        slen = strlen(prefix);
        if (slen + SFS_FILENAME_MAX + 1 > 256) {
            SFS_ERR("Prefix too long for filename\n");
            return -1;
        }
        strcpy(full_name, prefix);
        if (prefix[slen - 1] != '/') {
            full_name[slen] = '/';
            name = &full_name[slen + 1];
        } else {
            name = &full_name[slen];
        }
    }

    if (sfs_integrity_check(ram_fs, 1) < 0) {
        SFS_WARN("Failed integrity check\n");
    }

    hdr = ram_fs->header;
    for (i = 0; i < hdr->file_count; i++) {
        if (extract_file(full_name, name, &hdr->files[i], 
                         ram_fs->data_buf) < 0) {
            SFS_ERR("SFS extraction failure for file %d\n", i);
            return -1;
        }
    }

    return 0;
}

#define HEADER_BYTES          sizeof(sfs_header_t)
#define FILE_START(ptr)       ((ptr) + HEADER_BYTES)
#define LAST_FILE(hdr)        ((hdr)->files[(hdr)->file_count - 1])
#define TOT_FILE_BYTES(hdr)   \
    LAST_FILE(hdr).offset + LAST_FILE(hdr).length - HEADER_BYTES

/*
 * Read an SFS from flash into a RAM SFS 
 */
int
sfs_flash_read(uint32_t base, sfs_ram_fs_t *ram_fs)
{
    int to_read;
    sfs_header_t *hdr;
    ssize_t rb;

    /* Read in the header */
    to_read = sizeof(sfs_header_t);
    if (!IS_ALIGNED(to_read)) {
        SFS_WARN("WARNING: Read length is not aligned\n");
    }
    rb = sfs_hw_flash_read(base, ram_fs->data_buf, to_read);
    if (rb != to_read) {
        if (rb < 0) {
            SFS_ERR("Read header:  Error on read\n");
        } else {
            SFS_ERR("Read header:  Only read %d of %d bytes\n", 
                  (int)rb, (int)to_read);
        }
        return -1;
    }
    if (sfs_integrity_check(ram_fs, 0) != 0) {
        SFS_ERR("Failed header integrity check.\n");
        return -1;
    }

    hdr = ram_fs->header;
    if (hdr->file_count == 0) { /* No files */
        SFS_WARN("Note: No files in file system; header okay\n");
        return 0;
    }

    /* Read in the rest of the file system */
    to_read = LEN_ALIGN(TOT_FILE_BYTES(hdr));
    rb = sfs_hw_flash_read(FILE_START(base), FILE_START(ram_fs->data_buf), 
                           to_read);
    if (rb != to_read) {
        SFS_ERR("Read files:  Only read %d of %d bytes\n", (int)rb, (int)to_read);
        return -1;
    }
    hdr->cur_bytes = sizeof(sfs_header_t) + LEN_ALIGN(rb);

    if (sfs_integrity_check(ram_fs, 1) != 0) {
        SFS_ERR("Failed file system integrity check.\n");
        return -1;
    }

    return 0;
}

/*
 * Write out a RAM SFS to flash
 */
int
sfs_flash_write(uint32_t base, sfs_ram_fs_t *ram_fs)
{
    ssize_t wb;

    ram_fs->header->hdr_crc = HEADER_CRC_GEN(ram_fs);

    if (sfs_integrity_check(ram_fs, 1) < 0) {
        SFS_ERR("sfs_flash_write: Failed integrity check\n");
        return -1;
    }
    wb = sfs_hw_flash_write(base, ram_fs->data_buf, ram_fs->header->cur_bytes);
    if (wb != ram_fs->header->cur_bytes) {
        SFS_ERR("sfs_flash_write: ERROR only wrote %d of %d bytes\n",
              (int)wb, ram_fs->header->cur_bytes);
        return -1;
    }

    return 0;
}

