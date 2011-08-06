/*
 * sfs_lb4g.c
 * 
 * SFS functions for LB4G platform
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

#include "../flashDrv/flashDrv.h"
#include "sfs.h"
#include "sfs_lb4g.h"

ssize_t
lb4g_flash_write(uint32_t base_offset, const unsigned char *data, uint32_t len)
{
    int fd;
    ssize_t wb;
    int rv;

    fd = open("/dev/flash", O_WRONLY);
    if (fd < 0) {
        SFS_ERR("lb4g flash write: Failed to open flash device\n");
        return -1;
    }

    SFS_VERB("lb4g flash write: Opened flash device\n");
    rv = ioctl(fd, FLASHDRV_SET_ADDR_CMD, FLASH_MEM_ADDR_BEGIN + base_offset);
    if (rv != 0) {
        SFS_ERR("lb4g flash write: Write seek ioctl returns %d; failed\n", rv);
        close(fd);
        return -1;
    }
#if defined(NO_FLASH_WRITE)
    SFS_WARN("lb4g flash write:  NOT writing %d bytes to flash\n", len);
    wb = len;
#else
    wb = write(fd, data, len);
    if (wb >= 0) { /* Driver doesn't return number of bytes */
        wb = len;
        SFS_VERB("Wrote %d bytes.\n", (int)len);
    } else {
        SFS_ERR("Error writing data to flash\n");
    }
#endif

    close(fd);
    return wb;
}

static ssize_t
_flash_read_lb4g(int fd, uint32_t base_offset, unsigned char *buf, uint32_t len)
{
    int rv;

    SFS_VERB("Seek 0x%x\n", FLASH_MEM_ADDR_BEGIN + base_offset);
    rv = ioctl(fd, FLASHDRV_SET_ADDR_CMD, FLASH_MEM_ADDR_BEGIN + base_offset);
    if (rv != 0) {
        SFS_ERR("lb4g flash read: seek ioctl for 0x%x returns %d; failed\n", 
              base_offset, rv);
        return -1;
    }

    /* Read in the data */
    return read(fd, buf, len);
}

/* Read in the file system, check magic and checksum */
ssize_t
lb4g_flash_read(uint32_t base_offset, unsigned char *buf, uint32_t len)
{
    int fd;
    ssize_t rv;

    if (len & 0x3) {
        SFS_ERR("ERROR:  Length must be 4-byte aligned on this platform\n");
        return -1;
    }

    fd = open("/dev/flash", O_RDONLY);
    if (fd < 0) {
        SFS_ERR("lb4g flash read: Failed to open flash device\n");
        return -1;
    }

    rv = _flash_read_lb4g(fd, base_offset, buf, len);
    close(fd);

    if (rv >= 0) { /* Driver doesn't return number bytes read :< */
        return len;
    } else {
        return rv;
    }
}
