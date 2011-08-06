/*
 * sfs_host.c
 * 
 * Debug SFS functions for host platform
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
#include <string.h>

#include "sfs.h"
#include "sfs_host.h"

static unsigned char fake_flash[SFS_FLASH_BYTES];

static void save_file(void)
{
    int fd;

    fd = open("_fake_flash", O_CREAT + O_WRONLY + O_TRUNC, S_IRUSR + S_IWUSR);
    if (fd < 0) {
        SFS_ERR("FAILED to open _fake_flash for write\n");
        return;
    }

    SFS_WARN("Saved %d bytes to _fake_flash\n", 
           (int)write(fd, fake_flash, SFS_FLASH_BYTES));
    close(fd);
}

static void read_file(void)
{
    int fd;
    int bytes_in;

    fd = open("_fake_flash", O_RDONLY);
    if (fd < 0) {
        SFS_ERR("FAILED to open _fake_flash for read\n");
        return;
    }

    bytes_in = (int)read(fd, fake_flash, SFS_FLASH_BYTES);

    SFS_WARN("Read in %d bytes from _fake_flash\n", bytes_in);
           
    close(fd);
}

/* Driver functions for HW specific read/write ops; return bytes written/read */
ssize_t
fake_flash_write(uint32_t base_offset, const unsigned char *buf, uint32_t len)
{
    read_file();

    if (base_offset > SFS_FLASH_BYTES) {
        return -1;
    }

    if (base_offset + len > SFS_FLASH_BYTES) {
        len = SFS_FLASH_BYTES - base_offset;
    }

    SFS_VERB("Copying %d bytes to FS\n", len);
    memcpy(&fake_flash[base_offset], buf, len);

    save_file();

    return (ssize_t)len;
}

ssize_t
fake_flash_read(uint32_t base_offset, unsigned char *buf, uint32_t len)
{
    read_file();

    if (base_offset > SFS_FLASH_BYTES) {
        return -1;
    }

    if (base_offset + len > SFS_FLASH_BYTES) {
        len = SFS_FLASH_BYTES - base_offset;
    }

    SFS_VERB("Copying %d bytes from FS\n", len);
    memcpy(buf, &fake_flash[base_offset], len);

    return (ssize_t)len;
}
