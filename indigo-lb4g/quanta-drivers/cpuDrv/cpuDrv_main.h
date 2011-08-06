#ifndef CPUDRV_MAIN
#define CPUDRV_MAIN

#define CPU_MAJOR_DEVICE_ID      243
#define CPU_DRIVER_NAME          "cpuDrv"

#define PFX                     "cpuDrv: "

#define cpuDrv_IOC_MAGIC 0x66
#define cpuDrv_IOCRESET _IO(cpuDrv_IOC_MAGIC, 0)
#define cpuDrv_IOCGCCSR _IOR(cpuDrv_IOC_MAGIC, 1, int)
#define cpuDrv_IOCSCCSR _IOR(cpuDrv_IOC_MAGIC, 2, int)
#define cpuDrv_IOC_MAXNR 3

#define CCSR_READ 1
#define CCSR_WRITE 2

#endif
