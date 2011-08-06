/*
 * sfs_gsm.c
 * 
 * SFS functions for GSM platform
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
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

#include <mtd/mtd-user.h>

#include "sfs.h"
#include "sfs_gsm.h"

static char *dev_name = "/dev/mtd0";

static int
erase_bytes(int fd, struct mtd_info_user *mtd, int base_offset, uint32_t len)
{
	struct erase_info_user erase;

    if (base_offset % mtd->erasesize) {
        SFS_ERR("Bad offset %d.  Erase size %d\n", base_offset, mtd->erasesize);
        return -1;
    }

	erase.start = base_offset;
	erase.length = len & ~(mtd->erasesize - 1);
	if (len % mtd->erasesize) {
        erase.length += mtd->erasesize;
    }

    SFS_WARN("Erasing %d bytes (actual %d) from %d\n", len, erase.length,
        erase.start);

#if !defined(NO_FLASH_WRITE)
    if (ioctl(fd, MEMERASE, &erase) < 0) {
        SFS_ERR("ERROR: Erasing from %d\n", erase.start);
        return -1;
    }
#endif

    return 0;
}

ssize_t
gsm_flash_write(uint32_t base_offset, const unsigned char *data, uint32_t len)
{
    int fd;
    int bytes;
    int result;
    int written;
    int remaining;
	struct mtd_info_user mtd;

    if ((fd = open(dev_name, O_RDWR | O_SYNC)) < 0) {
        SFS_ERR("gsm flash write: Failed to open %s\n", dev_name);
        return -1;
    }

    SFS_VERB("gsm flash write: Opened %s\n", dev_name);
    if (ioctl(fd, MEMGETINFO, &mtd) < 0) {
        SFS_ERR("Could not get MTD info\n");
        close(fd);
        return -1;
    }

    if (erase_bytes(fd, &mtd, base_offset, len) < 0) {
        SFS_ERR("Could not erase.  Exiting\n");
        close(fd);
        return -1;
    }

	if (lseek(fd, base_offset, SEEK_SET) < 0) {
		SFS_ERR("Error seeking to %d of %s: %m\n", base_offset, dev_name);
        close(fd);
        return -1;
	}

    bytes = mtd.erasesize;
    written = 0;
    remaining = len;

    SFS_VERB("gsm flash write:  Writing %d bytes to flash\n", len);
    if (1) {
        const unsigned char *p = data;
        SFS_VERB("%02x%02x%02x%02x %02x%02x%02x%02x "
                 "%02x%02x%02x%02x %02x%02x%02x%02x\n", 
                 p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], 
                 p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);
        p += 16;
        SFS_VERB("%02x%02x%02x%02x %02x%02x%02x%02x "
                 "%02x%02x%02x%02x %02x%02x%02x%02x\n", 
                 p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], 
                 p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);
    }
    do {
        if (remaining < bytes) {
            bytes = remaining;
        }

        SFS_VERB("\rWriting %d bytes to offset %d", bytes, written);
#if defined(NO_FLASH_WRITE)
        result = bytes;
#else
        result = write(fd, &data[written], bytes);
#endif
        if (result < bytes) {
            SFS_ERR("Error writing at offset %d\n", written);
            close(fd);
            return -1;
        }
        written += bytes;
        remaining -= bytes;

        SFS_VERB("\rWrote %d bytes to %s", written, dev_name);
    } while (remaining > 0);
    SFS_VERB("\rWrote %d bytes to %s\n", written, dev_name);

    close(fd);
    return written;
}

static ssize_t
_flash_read_gsm(int fd, uint32_t base_offset, unsigned char *buf, uint32_t len)
{
    int rv;

    SFS_VERB("Seek 0x%x for read\n", base_offset);
	if (lseek(fd, base_offset, SEEK_SET) < 0) {
		SFS_ERR("Error seeking to %d of %s: %m\n", base_offset, dev_name);
        return -1;
    }

    /* Read in the data */
    SFS_VERB("Reading %d bytes from offset %d\n", len, base_offset);
    rv = read(fd, buf, len);
    SFS_VERB("Read result %d\n", rv);

    return rv;
}

/* Read in the file system, check magic and checksum */
ssize_t
gsm_flash_read(uint32_t base_offset, unsigned char *buf, uint32_t len)
{
    int fd;
    ssize_t bytes;

    if (len & 0x3) {
        SFS_ERR("ERROR:  Length must be 4-byte aligned on this platform\n");
        return -1;
    }

    fd = open(dev_name, O_RDONLY);
    if (fd < 0) {
        SFS_ERR("gsm flash read: Failed to open %s\n", dev_name);
        return -1;
    }

    bytes = _flash_read_gsm(fd, base_offset, buf, len);
    close(fd);

    if (1) {
        unsigned char *p = buf;
        SFS_VERB("Read in:\n");
        SFS_VERB("%02x%02x%02x%02x %02x%02x%02x%02x "
                 "%02x%02x%02x%02x %02x%02x%02x%02x\n", 
                 p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], 
                 p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);
        p += 16;
        SFS_VERB("%02x%02x%02x%02x %02x%02x%02x%02x "
                 "%02x%02x%02x%02x %02x%02x%02x%02x\n", 
                 p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], 
                 p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);
    }

    return bytes;
}
