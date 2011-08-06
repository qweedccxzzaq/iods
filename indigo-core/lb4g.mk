# Copyright (c) Big Switch Networks 2011 and Stanford University, 2010
#
# Makefile fragment for Quanta lb4g platform

export PLAT=lb4g
include quanta.mk

#export ELDK_DIR=${TOOLS_DIR}/eldk_4.1
PLAT_NAME=Pronto 3240
PLAT_TAG=pronto-3240

#export ARCH=ppc
export ARCH=powerpc
export STANFORD_LB4G=1
export QUANTA_LB4G=1

CFG_CFLAGS += -DSTANFORD_LB4G -DQUANTA_LB4G
BUILD_STRING += Stanford-LB4G.
HW_LIB_NAME = stanford-lb4g

PLATFORM_ROOT=${TOP_LEVEL_DIR}/indigo-lb4g
#export LINUX_ROOT=${PLATFORM_ROOT}/linux-2.6.15
#LINUX_VERSION=2.6.15
export LINUX_ROOT=${PLATFORM_ROOT}/linux-2.6.27
LINUX_VERSION=2.6.27


#UBOOT_DIR=${PLATFORM_ROOT}/uboot
UBOOT_DIR=${PLATFORM_ROOT}/u-boot-1.3.0

# FIXME
#export BUSYBOX_DIR=${CORE_DIR}/busybox-1.4.2

RAMDISK_BLOCKS=40960
#RAMDISK_BLOCKS=32768

# From 2.6.27 port
export CONFIG_DEBUG_INFO = 1

# Lua socket library requires this for some builds.
export USE_LIBGCC_S 1

RELEASE_FILES+=${PLATFORM_ROOT}/quanta-drivers/sfs/SFS_usage.txt
