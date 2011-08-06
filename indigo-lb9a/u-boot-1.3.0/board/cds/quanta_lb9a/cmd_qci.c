
#include <common.h>
#include <command.h>
#include <i2c.h>
#include <asm/byteorder.h>
#include <net.h>

#define CFG_CMD_QCI

#ifdef CFG_CMD_QCI
#endif  /* CFG_CMD_QCI */

#define FLASH_START_RAMDISK	0xFF000000
#define FLASH_START_KERNEL	0xFFD00000
#define FLASH_START_DTB		0xFFEE0000
#define FLASH_START_UBOOT	0xFFF00000
#define FLASH_START_END		0x100000000

#define FLASH_SIZE_RAMDISK	FLASH_START_KERNEL - FLASH_START_RAMDISK
#define FLASH_SIZE_KERNEL	FLASH_START_DTB - FLASH_START_KERNEL
#define FLASH_SIZE_DTB		FLASH_START_UBOOT - FLASH_START_DTB
#define FLASH_SIZE_UBOOT	FLASH_START_END - FLASH_START_UBOOT


static int urlParser(char *in_str, char *ipaddr, char *path , char *filename, char *xferToken )
{
    char *pbuf ;
    char *ptmp ;
    unsigned int colonIndex;
    unsigned int fileIndex;
    unsigned int pathIndex;
    unsigned int tokenLen = strlen("xmodem");

    if (strlen(in_str) < 1) /*At least tftp:x should be there*/
        return 1;

    if (strncmp(in_str,"xmodem", tokenLen) == 0 )
    {
        strcpy(xferToken,in_str);
        *ipaddr = '\0';
        *path = '\0';
        *filename = '\0';
        return 0;
    }

    /* check for ':' */
    pbuf=strchr(in_str,':');
    if(pbuf == NULL)  /* token separator not found */
        return 1;

    /*Location of first ':' */
    colonIndex=strlen(in_str) - strlen(pbuf);

    if(colonIndex  == strlen("tftp") )
        tokenLen= colonIndex ;
    else
        return 1;

    if (strncmp(in_str,"tftp", tokenLen) != 0 )
        return 1;

    strncpy(xferToken,in_str,tokenLen);

    /*Get filename */
    /* check for last '/' */
    pbuf=strrchr(in_str,'/');
    if(pbuf == NULL)
    {
        return 1; /* No filename */
    }
    else
    {
        /*filename start index */
        fileIndex = strlen(in_str) - strlen (pbuf);
        tokenLen = strlen(pbuf) - 1;

        if(tokenLen > 0)
            strncpy(filename, in_str + fileIndex + 1   , tokenLen);
        else
            return 1; /* No filename */
    }


    /*Get path */
    /* check for first '/' after 'tftp://'*/
    ptmp = strstr(in_str, "//");
    if (ptmp == NULL)
        return 1;

    pbuf=strchr(ptmp + 2, '/');

    if(pbuf == NULL)
    {
        return 1; /* No filename */
    }
    else
    {
        /*path start index */
        pathIndex = strlen(in_str) - strlen (pbuf);
        if( pathIndex  == fileIndex )
        {
            strcpy(path, "./");  /*Default path current dir */
            tokenLen = 0;
        }
        else
        {
            tokenLen = fileIndex - pathIndex ;
        }

        if(tokenLen > 0)
            strncpy(path, in_str + pathIndex + 1, tokenLen);
    }


    /* tftp xfer type */
    /* ip addres is from colonIndex to pathIndex */
    tokenLen =  pathIndex - colonIndex  - 3;
    if(tokenLen > 0)
        strncpy(ipaddr, in_str + colonIndex + 3, tokenLen);
    else
        return 1;

    return 0;
}

#define MAX_STRING_LENGTH     128

int do_copy(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    int rc = 0;
    unsigned int upgrade_type = 0, transtype = 0;
    char strIp[MAX_STRING_LENGTH];
    char filePath[MAX_STRING_LENGTH];
    char fileName[MAX_STRING_LENGTH];
    char type[MAX_STRING_LENGTH];
    int ch;
    int size;
    ulong end_addr;
    ulong flash_sect_addr = 0;
    ulong mem_addr;
    ulong image_len;

    if (argc != 3)
    {
        printf("Usage:\n%s\n", cmdtp->usage);
        return 1;
    }

    if ((strcmp(argv[1], "-boot")==0)||(strcmp(argv[1], "-b")==0))
        upgrade_type = 1;
    else if ((strcmp(argv[1], "-kernel")==0)||(strcmp(argv[1], "-k")==0))
        upgrade_type = 2;
    else if ((strcmp(argv[1], "-ramdisk")==0)||(strcmp(argv[1], "-r")==0))
        upgrade_type = 3;
    else if ((strcmp(argv[1], "-dtb")==0)||(strcmp(argv[1], "-d")==0))
        upgrade_type = 4;
    else
    {
        printf("%s : Unknow upgrade type!\n", argv[0]);
        return 1;
    }

    memset(strIp, 0, MAX_STRING_LENGTH);
    memset(filePath, 0, MAX_STRING_LENGTH);
    memset(fileName, 0, MAX_STRING_LENGTH);
    memset(type, 0, MAX_STRING_LENGTH);

    if (urlParser(argv[2], strIp, filePath, fileName,type))
    {
        printf("%s : Wrong arguments!\n", argv[0]);
        return 1;
    }

    if (strcmp(type, "tftp")==0)
        transtype = 1;
    else if (strcmp(type, "xmodem")==0)
        transtype = 2;
    else
    {
        printf("%s : Unknow transfer type!\n", argv[0]);
        return 1;
    }

    printf("\nDownload Image File");
    printf("\nTransfer Type \t: %s",type);
    if (transtype==1)
    {
        printf("\nServer IP \t: %s", strIp);
        printf("\nFile Name \t: %s",fileName);
        printf("\n\nAre you sure you want to start? (y/n) ");
        while(1)
        {
            ch = serial_getc();
            switch (ch)
            {
                case 'y':
                case 'Y':
                    printf("\n");
                    goto TRANS_START;
                case 'n':
                case 'N':
                    printf("\nTransmission aborted!\n\n");
                    return 0;
            }
        }
        TRANS_START:;

        load_addr = CFG_LOAD_ADDR;
        setenv ("serverip", strIp);
        setenv ("bootfile", fileName);
        if ((size = NetLoop(TFTP)) < 0)
        {
            printf("%s: TFTP transfer error!\n", argv[0]) ;
            return 1;
        }

        /* flush cache */
        flush_cache(load_addr, size);

        mem_addr = load_addr;
        if (upgrade_type==1) /* UBOOT */
            flash_sect_addr = FLASH_START_UBOOT;//0xFFF00000;
        else if (upgrade_type==2) /* KERNEL */
            flash_sect_addr = FLASH_START_KERNEL;//0xFFD00000;
        else if (upgrade_type==3) /* RAMDISK */
            flash_sect_addr = FLASH_START_RAMDISK;//0xFF000000;
		else if (upgrade_type==4) /* DTB */
            flash_sect_addr = FLASH_START_DTB;//0xFFEE0000;
		
        if (size&(~0xfffe0000))
            image_len = (size&0xfffe0000)+0x20000;
        else
            image_len = (size&0xfffe0000);
        end_addr = flash_sect_addr + image_len - 1;

		/* Check image/partition size */
		{
		int errFlag = 0;
		char errMsg[MAX_STRING_LENGTH];
		memset(errMsg, 0, MAX_STRING_LENGTH);
        if ((upgrade_type==1) && (size > FLASH_SIZE_UBOOT)) /* UBOOT */
		{
            errFlag = 1;
			sprintf(errMsg,"UBOOT partition size: 0x%x",FLASH_SIZE_UBOOT);
		}
        else if ((upgrade_type==2) && (size > FLASH_SIZE_KERNEL)) /* KERNEL */
		{
            errFlag = 1;
			sprintf(errMsg,"KERNEL partition size: 0x%x",FLASH_SIZE_KERNEL);
		}
        else if ((upgrade_type==3) && (size > FLASH_SIZE_RAMDISK)) /* RAMDISK */
		{
            errFlag = 1;
			sprintf(errMsg,"RAMDISK partition size: 0x%x",FLASH_SIZE_RAMDISK);
		}
		else if ((upgrade_type==4) && (size > FLASH_SIZE_DTB)) /* DTB */
		{
            errFlag = 1;
			sprintf(errMsg,"DTB partition size: 0x%x",FLASH_SIZE_DTB);
		}
		if(errFlag)
		{
			printf ("Error!! Ivalid size, Image file size 0x%x %s\n",size ,errMsg);
			return 1;
		}
		}
		
		
        if (flash_sect_protect(0, flash_sect_addr, end_addr))
            return 1;

        printf ("Erasing Flash... ");
        if (flash_sect_erase(flash_sect_addr, end_addr))
            return 1;
	printf ("done\n");

        printf ("Writing Flash... ");
        rc = flash_write((char *) mem_addr, flash_sect_addr, image_len);
        if (rc)
            flash_perror(rc);
        else
            printf ("done\n");

        if (flash_sect_protect (1, flash_sect_addr, end_addr))
            return 1;
    }

    return 0;
}

U_BOOT_CMD(
    copy, 4, 1, do_copy,
    "copy    - download image file from tftp server\n",
    "<-b/-k/-r/-d> <tftp://serverip/filename | xmodem>\n\t<-b/-boot> boot image file\n\t<-k/-kernel> kernel image file\n\t<-r/-ramdidk> ramdisk image file\n\t<-d/-dtb> Device Tree Binary file\n"
);

