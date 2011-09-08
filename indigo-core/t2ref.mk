#
# t2ref.mk
#     Platform specific make configuration file
#
# Processed early in make inclusion, so no targets here please.
#

PLAT=t2ref
BCM_REF=1  # Allows quanta.mk to exclude some defines
include quanta.mk

# Not platform drivers for this platform now
PLATFORM_DRIVERS=""
DRIVERS_CLEAN=""
PLATFORM_ROOTFS_TEMPLATE=${PLATFORM_ROOT}/t2ref-rootfs-template

PLAT_NAME=Broadcom 56634 Reference
PLAT_TAG=t2ref

export BCM_REF_DESIGN=1
export ARCH=powerpc
export BCM_TRIUMPH2_REF=1
CFG_CFLAGS += -DBCM_TRIUMPH2_REF
BUILD_STRING += T2-ref.
HW_LIB_NAME = bcm56634-ref

#
# Common include file for BCM reference designs
#

export PLATFORM_ROOT=${TOP_LEVEL_DIR}/indigo-bcm-ref
export LINUX_ROOT=${PLATFORM_ROOT}/linux-2.6.25


MODULE_VERSION_DIR=${TARGET_ROOTFS}/lib/modules/2.6.25-bcm-ntsw
LINUX_CONFIG_COMMAND=make -C ${LINUX_ROOT} ARCH=powerpc bcm98548xmc_defconfig
LINUX_UIMAGE_SOURCE=${LINUX_ROOT}/arch/powerpc/boot/uImage
LINUX_KERNEL_IMAGE=${LINUX_ROOT}/arch/${ARCH}/boot/uImage
export MODULE_INSTALL_DIR=${PLATFORM_ROOTFS}/lib/modules
export ROOTFS_MODULES_DIR=${TARGET_ROOTFS}/lib/modules/${LINUX_VERSION}

RAMDISK_BLOCKS=50000

CROSS_COMPILE_PATH=${ELDK_DIR}/bin:${ELDK_DIR}/usr/bin

# Lua socket library requires this for some builds.
export USE_LIBGCC_S=1
