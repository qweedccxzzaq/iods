/* vim:ts=4:sw=4:expandtab
 * (No tabs, indent level is 4 spaces)
 *
 * i2cDrv.c
 * I2C_Device Device Driver for QUANTA LB6 Platform
 * Written for Linux Kernel versions in the 2.6 series.
 *
 */

#define __NO_VERSION__
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/poll.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "i2cDrv.h"

#define RETRY_COUNT 3

typedef enum
{
  FIRST_I2C_DEVICE = 0,
  SODIMM,
/* Fressiy add, 2009/2/27 */
  ADT7460_SENSOR_1,
  ADT7460_SENSOR_2,
  #if 1 /*Yvonne. 9/15/2010 for adt7460 TACH4 issue*/
  ADT7460_SENSOR_1_EXT,
  #endif
  DS1338_RTC,
  AT24C02_EEPROM_MB,
  AT24C02_EEPROM_RPSU1,
  AT24C02_EEPROM_RPSU2,
#if 1  /* One PCA9555 to one RPSU, Modified by Quentin, 20100326 */
  PCA9555_EXPANDER_1,
  PCA9555_EXPANDER_2,
#else
  PCA9555_EXPANDER,
#endif
  M24512_EEPROM,  /* Added 02/11/2009 */  
  /* Fressiy modify, 2009/3/2 */
  DISTRIBUTER_1,
  DISTRIBUTER_2,
  DISTRIBUTER_3,
  AT24C02_EEPROM_SFP,
  LAST_I2C_DEVICE
} I2C_DEVICES;

#define I2C_INVALID_ADDRESS          0xFF


/* Fressiy modify, 2009/4/7 */
#define I2C_SODIMM		(0x53)
#define I2C_ADT7460_SENSOR_1	(0x2c)
#define I2C_ADT7460_SENSOR_2	(0x2e)
#if 1 /*Yvonne. 9/15/2010 for adt7460 TACH4 issue*/
#define I2C_ADT7460_SENSOR_1_EXT	(0x2d)
#endif
#define I2C_DS1338_RTC		(0x68)
#define I2C_AT24C02_EEPROM_MB		(0x54)
#if 1 /* Modified By Arius ---- 07/14/2009, For LB8 Hardware Ver 2.0 */
#define I2C_AT24C02_EEPROM_RPSU1	(0x58)
#define I2C_AT24C02_EEPROM_RPSU2	(0x59)
#else /* For LB8 Hardware Ver 1.0 */
#define I2C_AT24C02_EEPROM_RPSU1	(0x50)
#define I2C_AT24C02_EEPROM_RPSU2	(0x51)
#endif
#if 1  /* One PCA9555 to one RPSU, Modified by Quentin, 20100407 */
#define I2C_PCA9555_EXPANDER_1	(0x24)
#define I2C_PCA9555_EXPANDER_2	(0x25)
#else
#define I2C_PCA9555_EXPANDER		(0x24)
#endif
#define I2C_DISTRIBUTER_1	(0x25)
#define I2C_DISTRIBUTER_2	(0x26)
#define I2C_DISTRIBUTER_3	(0x27)
#define I2C_AT24C02_EEPROM_SFP	(0x50)
#define I2C_M24512_EEPROM	(0x52) 



struct i2cAddressMapping
{
  unsigned char address;
  unsigned char name[40];
};

/* This must match the order of I2C_DEVICES above. */
struct i2cAddressMapping i2c_device_adrs[] =
{
  {I2C_INVALID_ADDRESS,         "\0"},
  {I2C_SODIMM,              			"SODIMM"},
/* Fressiy add, 2009/2/27 */
  {I2C_ADT7460_SENSOR_1,			"ADT7460_SENSOR_1"},
  {I2C_ADT7460_SENSOR_2,			"ADT7460_SENSOR_2"},
  {I2C_ADT7460_SENSOR_1_EXT,			"ADT7460_SENSOR_1"},
  {I2C_DS1338_RTC, 				"DS1338_RTC"},
  {I2C_AT24C02_EEPROM_MB,		"AT24c02_EPPROM_MB"},
  {I2C_AT24C02_EEPROM_RPSU1,	"AT24c02_EPPROM_RPSU1"},
  {I2C_AT24C02_EEPROM_RPSU2, 	"AT24c02_EPPROM_RPSU2"},
#if 1  /* for RPSUx2, Modified by Quentin, 20100326 */
  {I2C_PCA9555_EXPANDER_1,		"PCA9555_EXPANDER_1"},
  {I2C_PCA9555_EXPANDER_2,		"PCA9555_EXPANDER_2"},
#else
  {I2C_PCA9555_EXPANDER,		"PCA9555_EXPANDER"},
#endif
  {I2C_M24512_EEPROM,		        "M24512_EPPROM"}, /* Added 02/11/2009 */   
  {I2C_DISTRIBUTER_1,		"I2C_DISTRIBUTER_1"},
  {I2C_DISTRIBUTER_2,		"I2C_DISTRIBUTER_2"},
  {I2C_DISTRIBUTER_3,		"I2C_DISTRIBUTER_3"},
  {I2C_AT24C02_EEPROM_SFP,		"AT24c02_EPPROM_SFP"},
};

/* FIXME: why is this not defined in linux/i2c.h? -ns */
#ifndef I2C_M_WR
#define I2C_M_WR 0x00
#endif

#define I2C_DEVICE_MAX_SIZE      256
#define I2C_DEVICE_DEVICE        0x54

static struct i2c_driver i2c_device_driver;
static struct i2c_client * i2c_device_client = NULL;

/* 2009/5/14 Raymond add. */
struct i2c_client i2cdev_client_temp = {
	.name		= "I2C /dev entry",
	.addr		= -1,
	.driver		= &i2c_device_driver,
};

extern int debugMode;

#define DBG_I2C_Device 0x0

#define PORTA_OFFSET           0x00090D10

static unsigned char i2c_dev_id = 0;

int i2c_device_init(void)
{
    int ret;

    ret = i2c_add_driver(&i2c_device_driver);
    if (ret)
    {
        printk(KERN_ERR PFX "i2c_device_init, i2c_add_driver failed, error %d\n", ret);
        return ret;
    }

    return 0;
}

void i2c_device_cleanup(void)
{
    i2c_del_driver(&i2c_device_driver);
}

int i2c_device_open(epfcb * fcb, struct file *file)
{
    file->f_pos = 0;
    fcb->lastpos = file->f_pos;
    fcb->maxpos = I2C_DEVICE_MAX_SIZE;
    return 0;
}


loff_t i2c_device_seek(struct file *file, loff_t offset, int whence)
{
    epfcb * fcb = (epfcb *) file->private_data;
    loff_t newpos;

    switch (whence)
    {
        case 0:         /* SEEK_SET */
            newpos = offset;
            break;
        case 1:         /* SEEK_CUR */
            newpos = fcb->lastpos + offset;
            break;
        case 2:         /* SEEK_END */
            newpos = fcb->maxpos + offset;
            break;
        default:
            return -EINVAL;
    }

    if (newpos < 0)
        return -EINVAL;

    if (newpos > fcb->maxpos)
        return -EINVAL;

    file->f_pos = newpos;

    if (debugMode & DBG_I2C_Device)
        printk(KERN_ERR PFX "seek complete, newpos=%lld\n", newpos);

    return newpos;
}


ssize_t i2c_device_read_internal(struct i2c_adapter * adap, unsigned char * kbuf, int offset, int bytesToRead)
{
    int ret, i;
    #if 1 /* Modified: 2/11/2009. Note: Added 2nd EEPROM */
    char address[2];
    #else
    char address;
    #endif
    struct i2c_msg msgs[2];
    unsigned char slaveAdrs;

    slaveAdrs = i2c_device_adrs[i2c_dev_id].address;

    /* build 1st message (starting address within i2c_device) */
    msgs[0].addr = slaveAdrs;
    msgs[0].flags = I2C_M_WR;
    /* Modified: 2/11/2009. Note: Added 2nd EEPROM */
    if (i2c_dev_id == M24512_EEPROM)
    {
      msgs[0].len = 2;
      msgs[0].buf = (char *) &address;
      address[0] = (char)(offset & 0xff00);
      address[1] = (char)(offset & 0xff);
      
    }    
    else
    {
      msgs[0].len = 1;
      msgs[0].buf = (char *) &address;
      address[0] = (char)(offset & 0xff);
    }
    /* End of here */  

    /* build 2nd message (data to be read) */
    msgs[1].addr = slaveAdrs;
    msgs[1].flags = I2C_M_RD;
    msgs[1].len = bytesToRead;
    msgs[1].buf = kbuf;

    /* issue command to device */
    if (debugMode & DBG_I2C_Device)
        printk(KERN_ERR PFX "reading i2c_device, offset=%d, count=%d\n",
            offset, bytesToRead);

    for (i=0; i<RETRY_COUNT; i++)
    {
        ret = i2c_transfer(adap, msgs, 2);
        if (ret == 2)
            break;
    }

    if (debugMode & DBG_I2C_Device)
        printk(KERN_ERR PFX "read complete, offset=%d, count=%d, ret=%d, "
            "data=%02X %02X %02X %02X %02X %02X %02X %02X\n",
            offset, bytesToRead, ret, kbuf[0], kbuf[1], kbuf[2], kbuf[3],
            kbuf[4], kbuf[5], kbuf[6], kbuf[7]);

    if (ret != 2)
    {
        if (debugMode & DBG_I2C_Device)  /* Modified by Quentin, 20100413 */
          printk(KERN_ERR PFX "i2c_device_read_internal, i2c_transfer failed, "
              "error %d\n", ret);
        return -EREMOTEIO;
    }

    return bytesToRead;
}


ssize_t i2c_device_read(struct file *file, char * buf, size_t count,
    loff_t *offsetptr)
{
    int ret;
    epfcb * fcb = (epfcb *) file->private_data;
    loff_t offset64 = *offsetptr;
    int offset;
    int bytesToRead;
    unsigned char * kbuf;
    struct i2c_client *client = (struct i2c_client *)fcb->client;

    offset = (int) offset64;

    if (offset64 > fcb->maxpos)
        return -EINVAL;

    offset = (int) offset64;

    if ((offset + count) > fcb->maxpos)
        bytesToRead = fcb->maxpos - offset;
    else
        bytesToRead = count;

    if (bytesToRead <= 0)
        return 0;

    /* allocate kernel-space buffer */
    kbuf = kmalloc(count * 2, GFP_KERNEL);
    if (kbuf == NULL)
        return -ENOMEM;

#if 1  /* For LB8 - EEPROM(M24512), Modified by Quentin, 20100413 */
    if (i2c_dev_id == M24512_EEPROM)
    	msleep(10);
#endif

    ret = i2c_device_read_internal(client->adapter, kbuf, offset, count);
    if (ret < 0)
    {
        kfree(kbuf);
        return ret;
    }

    /* copy data to user-space buffer */
    if (copy_to_user((void *) buf, kbuf, count))
    {
        kfree(kbuf);
        return -EFAULT;
    }

    kfree(kbuf);

    return count;
}

ssize_t i2c_device_write_internal(struct i2c_adapter * adap, unsigned char * kbuf, int bytesToWrite)
{
    int ret, i;
    struct i2c_msg msgs;
    unsigned char slaveAdrs;

    slaveAdrs = i2c_device_adrs[i2c_dev_id].address;

    msgs.addr = slaveAdrs;
    msgs.flags = I2C_M_WR;
    msgs.len = bytesToWrite;
    msgs.buf = kbuf;

    /* issue command to device */
    if (debugMode & DBG_I2C_Device)
        printk(KERN_ERR PFX "writeing i2c_device, offset=%d, count=%d\n", kbuf[0], (bytesToWrite-1));

    for (i=0; i<RETRY_COUNT; i++)
    {
        ret = i2c_transfer(adap, &msgs, 1);
        if (ret == 1)
            break;
    }

    if (debugMode & DBG_I2C_Device)
        printk(KERN_ERR PFX "write complete, offset=%d, count=%d, ret=%d, "
            "data=%02X %02X %02X %02X %02X %02X %02X %02X\n",
            kbuf[0], (bytesToWrite-1), ret, kbuf[1], kbuf[2], kbuf[3], kbuf[4],
            kbuf[5], kbuf[6], kbuf[7], kbuf[8]);

    if (ret != 1)
    {
        if (debugMode & DBG_I2C_Device)  /* Modified by Quentin, 20100413 */
            printk(KERN_ERR PFX "i2c_device_write_internal, i2c_transfer failed, "
                "error %d\n", ret);
        return -EREMOTEIO;
    }
    return bytesToWrite;
}

ssize_t i2c_device_write(struct file *file, const char * buf,
    size_t count, loff_t * offsetptr)
{
    int ret;
    epfcb * fcb = (epfcb *) file->private_data;
    loff_t offset64 = *offsetptr;
    int file_offset;
    unsigned char * kbuf;
    unsigned char * tmpbuf;
    struct i2c_client *client = (struct i2c_client *)fcb->client;

    if (offset64 > fcb->maxpos)
        return -EINVAL;

    file_offset = (int) offset64;

    if ((file_offset + count) > fcb->maxpos)
        return -EINVAL;

    if (count <= 0)
        return 0;

    /* allocate kernel-space buffer */
    kbuf = kmalloc(count, GFP_KERNEL);
    if (kbuf == NULL)
        return -ENOMEM;

    /* copy data from user-space buffer */
    if (copy_from_user(kbuf, (void *) buf, count))
    {
        kfree(kbuf);
        return -EFAULT;
    }

    /* Modified: 2/11/2009. Note: Added 2nd EEPROM */
    if (i2c_dev_id == M24512_EEPROM)
    {
       /* allocate kernel-space buffer */	
      tmpbuf = kmalloc(I2C_DEVICE_MAX_SIZE+3, GFP_KERNEL);
      if (tmpbuf == NULL)
      {
          kfree(kbuf);
          return -ENOMEM;
      }
      
      tmpbuf[1] = (file_offset & 0xff);
      tmpbuf[0] = (file_offset & 0xff00);
      memcpy(tmpbuf+2, kbuf, count);

#if 1  /* For LB8 - EEPROM(M24512), Modified by Quentin, 20100406 */
    	msleep(10);
#endif

      ret =  i2c_device_write_internal(client->adapter, tmpbuf, count+2); 
      if (ret != (count+2))
      {
          if (debugMode & DBG_I2C_Device)  /* Yvonne. 9/15/2010 */
              printk(KERN_ERR PFX "i2c_device_write error, "
                  "offset=%d, len=%d\n", file_offset, count);
          
          kfree(kbuf);
          kfree(tmpbuf);
          return -EFAULT;
      }                     
    }      
    else  
    { 
       /* allocate kernel-space buffer */	
      tmpbuf = kmalloc(I2C_DEVICE_MAX_SIZE+2, GFP_KERNEL);
      if (tmpbuf == NULL)
      {
          kfree(kbuf);
          return -ENOMEM;
      }
      
      tmpbuf[0] = file_offset;
      memcpy(tmpbuf+1, kbuf, count);
    ret =  i2c_device_write_internal(client->adapter, tmpbuf, count+1);
      if (ret != (count+1))
      {
          if (debugMode & DBG_I2C_Device)  /* Yvonne. 9/15/2010 */
              printk(KERN_ERR PFX "i2c_device_write error, "
                  "offset=%d, len=%d\n", file_offset, count);
          
          kfree(kbuf);
          kfree(tmpbuf);
          return -EFAULT;
      }           
    }
    /*  End of here */      

    kfree(kbuf);
    kfree(tmpbuf);

    return count;
}

int i2c_device_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
    int retcode = -EINVAL;

    switch (cmd)
    {
        /* I2c Device Select */
        case 0:
        i2c_dev_id = (unsigned char)arg;
        retcode = 0;
        break;
    }

    return retcode;
}


int i2c_device_release(struct inode * inode, struct file * file)
{
    return 0;
}


int i2c_device_attach_adapter(struct i2c_adapter *adapter)
{
    int ret;

    if (i2c_device_client != NULL)
        return -EBUSY;

    i2c_device_client = kmalloc(sizeof(struct i2c_client), GFP_KERNEL);
    if (i2c_device_client == NULL)
        return -ENOMEM;

    memset(i2c_device_client, 0x00, sizeof(*i2c_device_client));

    strcpy(i2c_device_client->name, I2C_DRIVER_NAME);
    i2c_device_client->flags = 0;
    i2c_device_client->addr = I2C_DEVICE_DEVICE;
    i2c_device_client->adapter = adapter;
    i2c_device_client->driver = &i2c_device_driver;

    /* attach the client to the device */
    ret = i2c_attach_client(i2c_device_client);
    return ret;
}


int i2c_device_detach_client(struct i2c_client *client)
{
    /* detach client from device */
    i2c_detach_client(client);

    /* release client memory */
    if (i2c_device_client != NULL)
    {
        kfree(i2c_device_client);
        i2c_device_client = NULL;
    }
    return 0;
}

static struct i2c_driver i2c_device_driver =
{
	.driver = {
		.name	= I2C_DEVICE_DRIVER_NAME,
	},
    id:                 51 /*I2C_DRIVERID_I2C_Device */,
    //flags:              I2C_DF_NOTIFY,
    attach_adapter:     i2c_device_attach_adapter,
    detach_client:      i2c_device_detach_client
};

