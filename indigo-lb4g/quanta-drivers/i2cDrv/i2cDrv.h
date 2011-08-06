
#ifndef QUANTA_I2C_DRV_H
#define QUANTA_I2C_DRV_H


#define I2C_MAJOR_DEVICE_ID      242
#define I2C_MINOR_ID       0

#define I2C_DRIVER_NAME          "Quanta_LB4G"
#define I2C_DEVICE_DRIVER_NAME          "Quanta_LB4G_I2C"
#define PFX                     "Quanta_LB4G: "

extern spinlock_t eeprom_access_lock;

typedef struct
{
    int devnum;
    struct inode * inode;
    loff_t maxpos;
    loff_t lastpos;
    int writeableProtectedArea;
} epfcb;

extern int i2c_device_init(void);
extern void i2c_device_cleanup(void);
extern int i2c_device_open(epfcb * fcb, struct file *file);
extern loff_t i2c_device_seek(struct file *file, loff_t offset, int whence);
extern ssize_t i2c_device_read(struct file *file, char * buf, size_t count, loff_t *offsetptr);
extern ssize_t i2c_device_write(struct file *file, const char * buf, size_t count, loff_t * offsetptr);
extern int i2c_device_ioctl(struct inode *inode, struct file *file,  unsigned int cmd, unsigned long arg);
extern int i2c_device_release(struct inode * inode, struct file * file);

#endif

