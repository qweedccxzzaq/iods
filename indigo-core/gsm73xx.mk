#
# Hardware Platform Make configuration file for GSM73XX
#
# Requires:
#    BASE_DIR:  Where ELDK, etc is found
#    CORE_DIR:  The Indigo core directory
#    TOP_LEVEL_DIR:  Where indigo platforms are found; usually $CORE_DIR/..

#
# Built products:
#   uImage from Linux kernel
#   busybox executables and links
#   OpenFlow:
#       libopenflow.a
#       libudatapath.a
#       executables in secchan and utilities
#

export PLAT=gsm73xx

all: flash_image md5 tftp-rfs copy-to-latest
	echo "Built ${FLASH_IMAGE}"

include common.mk

################################################################
#
# Set build string/extension, version name
#
################################################################

BUILD_STRING +=GSM73XX.

################################################################
#
# Where things come from 
#
################################################################

MTD_DIR=${PLATFORM_ROOT}/mtd-utils-1.1.0
export MTD_INCLUDE=${MTD_DIR}/mtd-utils-1.1.0/include

export LINUX_ROOT=${PLATFORM_ROOT}/linux-2.6.26.7
export LINUX_INCLUDE=${LINUX_ROOT}/include

export BCM_SDK=${PLATFORM_ROOT}/sdk-5.7.1
export SDK=${BCM_SDK}

SFS_DIR=${PLATFORM_ROOT}/sfs
SFS_SRC=${SFS_DIR}/target_build/sfsctl

################################################################
#
# Where things go (and how they get there)
#
################################################################

# The following tells Netgear bld env where our FS comes from
TARGET_SBIN=${TARGET_ROOTFS}/sbin
TARGET_MODULES_DIR=${TARGET_ROOTFS}/lib/modules

PADDED_KERNEL=config-files/kernel-7328-pad

################################################################
#
# Compile and configuration flags
#
################################################################

export GSM73XX=1

PLAT_NAME=Netgear GSM73XX
PLAT_TAG=ntgr-gsm73xx

CFG_CFLAGS += -DGSM73XX -DHAVE_STRLCPY
# Name of cross compiler target passed to OF configuration
CROSS_NAME=ppc-linux
HW_LIB_NAME = gsm73xx

################################################################
#
# Netgear build environment
#
################################################################

# Consider adding check to differentiate 7328 and 7352 for loader-.0.0.2
NETGEAR_PROJECT=${NETGEAR_TOP}

NETGEAR_TOOLS=${NETGEAR_TOP}/toolkit/gsm73xx/tools

TARGET_ROOTFS_IMAGE=${TMP_FILES_DIR}/gsm73xx-rootfs.image
FLASH_IMAGE=${PRODUCT_DIR}/mtd0image

export ELDK_DIR=${NETGEAR_TOP}/toolkit/gsm73xx
export ELDK=${ELDK_DIR}
export LANG=C

ELDK_SYS_ROOT=${ELDK_DIR}/powerpc-85xx-linux-uclibc/sys-root
export ELDK_EXTRA_LIB_DIR=${ELDK_SYS_ROOT}/usr/lib

ELDK_LINUX_LINK=${ELDK_SYS_ROOT}/usr/src/linux

# Don't use prebuilt JSON lib yet
#export BUILD_JSON_LIB=1
export JSON_PREBUILT_DIR=${PLATFORM_ROOT}/prebuilt/json-ppc

DROPBEAR_ARCH=ppc

################################################################
#
# Local build environment
#
################################################################


MTD_DIR=${PLATFORM_ROOT}/mtd-utils-1.1.0

################################################################
#
# Compile setup
#
################################################################
export ARCH=powerpc
export ARCH_PREFIX=ppc_85xx

export PATH:=${PATH}:${ELDK_DIR}/bin:${ELDK_DIR}/usr/bin:${NETGEAR_TOOLS}
export CROSS_COMPILE=ppc_85xx-
export CROSS_COMPILE_PATH=${ELDK_DIR}/usr

################################################################
#
# OpenFlow compilation defines
#
################################################################

# Default cross compiler prefix for target passed to config
ifndef CROSS_NAME
CROSS_NAME=powerpc-linux
endif

OF_CFG_FLAGS = --host=${CROSS_NAME}
OF_CFG_FLAGS += --enable-hw-lib=gsm73xx

OF_CFG_FLAGS += CFLAGS="${CFG_CFLAGS}"
export OF_CFG_FLAGS
export CFG_CFLAGS

OF_HW_LIB = lib${HW_LIB_NAME}.a

################################################################
#
# Netgear rootfs
#
################################################################

# Top level root FS target
RAMDISK_BLOCKS=32768

ROOTFS_SIZE_K = 32768 # 32M
ROOTFS_EXT2 = ${TMP_FILES_DIR}/rootfs.ramdisk
PLATFORM_DEFINED_ROOTFS = 1

target-rootfs: ${TARGET_ROOTFS}
${TARGET_ROOTFS}: gsm73xx-rootfs

# The final product for flash
flash_image: gsm73xx-rootfs-image padded-kernel
	cat ${PADDED_KERNEL} ${TARGET_ROOTFS_IMAGE} > ${FLASH_IMAGE}

padded-kernel:
	@echo "TODO: Support generating padded kernel"

gsm73xx-rootfs-image: gsm73xx-rootfs
	genext2fs -d ${TARGET_ROOTFS} -b ${ROOTFS_SIZE_K} -i ${ROOTFS_SIZE_K} \
		-q ${ROOTFS_EXT2}
	rm -f ${ROOTFS_EXT2}.gz
	gzip ${ROOTFS_EXT2}
	mkimage -A ppc -O linux -T ramdisk -C gzip -d ${ROOTFS_EXT2}.gz \
		${TARGET_ROOTFS_IMAGE}
	echo "Built rootfs image ${TARGET_ROOTFS_IMAGE}."


gsm73xx-rootfs: rootfs-common mtd sfs
#	cp -f ${SBIN_FILES_SRC} ${TARGET_SBIN}  # Check this
	cp ${SFS_SRC} ${TARGET_SBIN}
	strip-bin ${TARGET_ROOTFS}
	sudo chmod 755 -R ${TARGET_ROOTFS}/lib/*
	rm -rf ${TARGET_ROOTFS}/share
	rm -rf ${TARGET_ROOTFS}/man
	echo "Populated target rootfs: ${TARGET_ROOTFS}"

target-rootfs-init: tool-env-init ${PRODUCT_DIR}
	rm -rf ${TARGET_ROOTFS}
	mkdir -p ${TARGET_ROOTFS}
	mkdir -p ${TARGET_ROOTFS}/proc # Needed for reboot, etc 
	mkdir -p ${TARGET_ROOTFS}/tmp  # Needed by vlog
	sudo cp -a ${COMMON_ROOTFS_TEMPLATE}/* ${TARGET_ROOTFS}/
	sudo cp -a ${PLATFORM_ROOTFS_TEMPLATE}/* ${TARGET_ROOTFS}/
	mkdir -p ${TARGET_MODULES_DIR}/2.6.26.7
	touch ${TARGET_MODULES_DIR}/2.6.26.7/.keep
	@echo "${VERSION_NAME}: ${BUILD_STRING}" > ${VERSION_FILE}

# Flash subsystem target
mtd-clean:
	make -C ${MTD_DIR} clean

mtd: target-rootfs-init
	make -C ${MTD_DIR} WITHOUT_XATTR=1 RAWTARGETS="flashcp flash_eraseall"
	cp ${MTD_DIR}/flashcp ${TARGET_SBIN}
	cp ${MTD_DIR}/flash_eraseall ${TARGET_SBIN}

################################################################
# sfs
################################################################

sfs:
	make -C ${SFS_DIR}

sfs-clean:
	make -C ${SFS_DIR} clean

DRIVERS_CLEAN=sfs-clean mtd-clean
drivers-clean: ${DRIVERS_CLEAN}

.PHONY: sfs sfs-clean mtd mtd-clean gsm73xx-rootfs-image padded-kernel \
	tool-env-init drivers-clean target-products flash_image

