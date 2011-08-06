
/* 
 * sfsctl.c
 *
 * SFS control program.  
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <string.h>

#include "sfs.h"
#if defined(HOST_TESTING)
#include "sfs_host.h"
#define SFS_FLASH_READ fake_flash_read
#define SFS_FLASH_WRITE fake_flash_write
#else
#include "sfs_gsm.h"
#define SFS_FLASH_READ gsm_flash_read
#define SFS_FLASH_WRITE gsm_flash_write
#endif

#define SFS_DEFAULT_TARGET_DIR "sfs"

/* In the spirit of SFS, stupid argument parsing */
static int
find_arg(int argc, char *argv[], char *string, int len)
{
    int i;
    
    for (i = 2; i < argc; i++) {
        if (strncmp(argv[i], string, len) == 0) {
            return i;
        }
    }
    return -1;
}

/* set base if --base is present; else return -1 */
static int
find_base_arg(int argc, char *argv[], uint32_t *base)
{
    int i;

    if ((i = find_arg(argc, argv, "--base=", 7)) > 0) {
        *base = strtoul(&argv[i][7], NULL, 0);
        return 0;
    }

    return -1;
}

/* Set target if --target is present; else return -1 */
static int
find_target_arg(int argc, char *argv[], char **target)
{
    int i;

    if ((i = find_arg(argc, argv, "--target=", 9)) > 0) {
        *target = &argv[i][9];
        return 0;
    }

    return -1;
}

static unsigned char fs_buf[SFS_ALLOC_BYTES];

#define TRY(op, rv) do {                                 \
        if ((rv = (op)) != 0) {                          \
            printf("ERROR: %s returned %d\n", #op, rv);  \
            return rv;                                   \
        }                                                \
    } while (0)


static int
files_add(sfs_ram_fs_t *ram_fs, int argc, char *argv[], int *count)
{
    int i;
    int rv;

    /* Iterate through arguments and add files */
    *count = 0;
    for (i = 2; i < argc; i++) {
        if (strncmp(argv[i], "--", 2)) { /* Ignore options */
            TRY(sfs_file_append(argv[i], ram_fs), rv);
            printf("Added file %s\n", argv[i]);
            ++*count;
        }
    }

    return 0;
}

static void
usage(char *prog_name)
{
    printf("Usage: %s <command> [--verb]\n"
           "    check [--base=<base-offset>]\n"
           "        Verify and report on an SFS\n"
           "    extract [--target=<target>] [--base=<base-offset>]\n"
           "        Extract an SFS from <base-offset> in flash to <target>\n"
           "    create [--base=<base-offset>] <file-list>\n"
           "        WARNING:  DESTRUCTIVE, OVERWRITES existing data in flash\n"
           "        Create an SFS at <base-offset> with files in file-list\n"
           "    search\n"
           "        Search for a valid SFS header in flash\n"
           "    test [--target=<target>] <file-list>\n"
           "        Create a RAM SFS from file-list and write the result\n"
           "        to the output-target directory\n"
           "    ftest [--base=<base-offset>] [--target=<target>] <file-list>\n"
           "        WARNING:  DESTRUCTIVE, OVERWRITES existing data in flash\n"
           "        Create a RAM SFS from file-list, commit to flash and\n"
           "        extract the result back to the output-target directory\n"
           "  Notes:\n"
           "    Default base address is 0x%x.  Default target is %s\n"
           "    SFS size is %d bytes.\n"
           "    --verb for verbose output\n"
           "    No spaces allowed around = for --base= and --target options\n"
           "    Output directories must already exist\n",
           prog_name, SFS_DEFAULT_OFFSET, SFS_DEFAULT_TARGET_DIR,
           SFS_ALLOC_BYTES);
}

int
main(int argc, char *argv[])
{
    sfs_ram_fs_t ram_fs;
    int rv;
    int count;
    char* target_dir = SFS_DEFAULT_TARGET_DIR;
    uint32_t base = SFS_DEFAULT_OFFSET;
    int verbose;
    int override;

    TRY(sfs_config(SFS_FLASH_READ, SFS_FLASH_WRITE), rv);
    TRY(sfs_ram_fs_init(&ram_fs, fs_buf, SFS_ALLOC_BYTES), rv);

    if (argc == 1) {
        usage(argv[0]);
        return -1;
    }

    find_base_arg(argc, argv, &base);
    find_target_arg(argc, argv, &target_dir);
    verbose = (find_arg(argc, argv, "--verb", 6) > 0);
    if (verbose) {
        sfs_debug_set(SFS_DEBUG_VERB);
    }
    override = (find_arg(argc, argv, "--override", 10) > 0);

    if (strcmp(argv[1], "check") == 0) {
        printf("Checking at flash offset 0x%x\n", base);
        TRY(sfs_flash_read(base, &ram_fs), rv);
        sfs_dump(&ram_fs);
        printf("SFS at offset 0x%x appears okay\n", base);
        return 0;
    } else if (strcmp(argv[1], "extract") == 0) {
        printf("Extracting from flash offset 0x%x to %s\n", base, target_dir);
        TRY(sfs_flash_read(base, &ram_fs), rv);
        /*        sfs_dump(&ram_fs); */
        /* FIXME: Check target_dir exists */
        TRY(sfs_file_extract_all(target_dir, &ram_fs), rv);
        printf("Extraction done\n");
        return 0;
    } else if (strcmp(argv[1], "create") == 0) {
        if (!SFS_SAFE_OFFSET(base) && !override) {
            printf("FAIL: Not a safe offset: 0x%x\n", base);
            return -1;
        }
        printf("Creating at flash offset 0x%x\n", base);
        TRY(files_add(&ram_fs, argc, argv, &count), rv);
        /* sfs_dump(&ram_fs); */
        printf("Committing %d files\n", count);
        TRY(sfs_flash_write(base, &ram_fs), rv);
        printf("Done\n");
        return 0;
    } else if (strcmp(argv[1], "search") == 0) {
        int found = 0;
        int dbg_save;
        
        if (!verbose) { /* If not verbose, turn off all dbg messages */
            sfs_debug_set(SFS_DEBUG_NONE);
        }
        for (base = 0; base < SFS_FLASH_BYTES; base += SFS_ALLOC_BYTES) {
            if (sfs_flash_read(base, &ram_fs) == 0) {
                printf("Found valid SFS at offset 0x%x\n", base);
                found = 1;
            }
        }
        if (!found) {
            printf("No SFS found from 0 to 0x%x (%d), SFS size 0x%x (%d)\n", 
                   SFS_FLASH_BYTES, SFS_FLASH_BYTES, 
                   SFS_ALLOC_BYTES, SFS_ALLOC_BYTES);
        }
    } else if (strcmp(argv[1], "test") == 0) {
        printf("Testing in RAM only\n");
        TRY(files_add(&ram_fs, argc, argv, &count), rv);
        printf("Read in %d files.  Dump of RAM FS\n", count);
        sfs_dump(&ram_fs);
        printf("Writing out %d files to %s\n", count, target_dir);
        /* FIXME: Check target_dir exists */
        TRY(sfs_file_extract_all(target_dir, &ram_fs), rv);
        printf("Done\n");
        return 0;
    } else if (strcmp(argv[1], "ftest") == 0) {
        if (!SFS_SAFE_OFFSET(base) && !override) {
            printf("FAIL: Not a safe offset: 0x%x\n", base);
            return -1;
        }
        printf("Testing flash commit\n");
        TRY(files_add(&ram_fs, argc, argv, &count), rv);
        printf("Read in %d files.  Dump of RAM FS\n", count);
        sfs_dump(&ram_fs);
        printf("Committing %d files to flash\n", count);
        TRY(sfs_flash_write(base, &ram_fs), rv);
        printf("Re-initializing RAM fs\n");
        TRY(sfs_ram_fs_init(&ram_fs, fs_buf, SFS_ALLOC_BYTES), rv);
        printf("Reading back files from flash\n", count, target_dir);
        TRY(sfs_flash_read(base, &ram_fs), rv);
        sfs_dump(&ram_fs);
        printf("Extracting from flash offset 0x%x to %s\n", base, target_dir);
        /* FIXME: Check target_dir exists */
        TRY(sfs_file_extract_all(target_dir, &ram_fs), rv);
        printf("Extraction done\n");
        return 0;
    } else {
        usage(argv[0]);
    }

    return -1;
}
