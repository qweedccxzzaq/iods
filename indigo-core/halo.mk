#
# halo.mk
#     Platform specific make configuration file
#
# Halo is a platform where the host and target coincide, but datapath
# forwarding is still done by hardware.  The initial case is 
# identical to the t2ref platform, but with a PCIe extender cable to
# an x86, Linux PC.
#
# For this case, we don't need to worry about the root FS, etc.  Just
# build ofswd and ofproto on the host
#

#PLAT=halo
PLAT=t2ref
BCM_REF=1  # Allows quanta.mk to exclude some defines
X86_PLAT=1

################################################################
# This replaces the include of quanta.mk

all: openflow bcm-sdk target-install
	echo "Installed files to ${PRODUCT_DIR}/halo-exec"

include common.mk
PLATFORM_ROOT=${TOP_LEVEL_DIR}/indigo-halo

export JSON_DIR=${CMDSRV_DIR}/json-c-0.9
TARG_EXEC_DIR = ${TARGET_ROOTFS}/halo-exec
INSTALL_DIR = ${PRODUCT_DIR}/halo-exec

target-rootfs-init:
	rm -rf ${CORE_DIR}/build-images/${BLD_EXT}
	mkdir -p ${PRODUCT_DIR}
	rm -rf ${TMP_FILES_DIR}/rootfs-${BLD_EXT}
	mkdir -p ${TARGET_ROOTFS}/sbin
	mkdir -p ${TARGET_ROOTFS}/lib/modules
	mkdir -p ${TARGET_ROOTFS}/etc
	cp -a ${PLATFORM_ROOTFS_TEMPLATE}/* ${TARGET_ROOTFS}/

target-install:
	mkdir -p ${INSTALL_DIR}
	cp -a ${TARGET_ROOTFS}/halo-exec ${PRODUCT_DIR}/
	cp  ${TARGET_ROOTFS}/sbin/ofswd ${INSTALL_DIR}/
	cp  ${TARGET_ROOTFS}/sbin/ofprotocol ${INSTALL_DIR}/
	cp  ${TARGET_ROOTFS}/sbin/dpctl ${INSTALL_DIR}/

################################################################

# Not platform drivers for this platform now
PLATFORM_DRIVERS=""
DRIVERS_CLEAN=""

PLAT_NAME=Halo BRCM 56634 Reference
PLAT_TAG=halo-x86

export BCM_REF_DESIGN=1
export ARCH=x86
export BCM_TRIUMPH2_REF=1
export CFG_CFLAGS += -DBCM_TRIUMPH2_REF
BUILD_STRING += Halo-T2-ref.
HW_LIB_NAME = bcm56634-ref

#
# Common include file for BCM reference designs
#

export PLATFORM_ROOT=${TOP_LEVEL_DIR}/indigo-halo
export LINUX_ROOT=/lib/modules/`uname -r`/build


#MODULE_VERSION_DIR=${TARGET_ROOTFS}/lib/modules/2.6.25-bcm-ntsw
#LINUX_CONFIG_COMMAND=make -C ${LINUX_ROOT} ARCH=powerpc bcm98548xmc_defconfig
#LINUX_UIMAGE_SOURCE=${LINUX_ROOT}/arch/powerpc/boot/uImage
#LINUX_KERNEL_IMAGE=${LINUX_ROOT}/arch/${ARCH}/boot/uImage
#export MODULE_INSTALL_DIR=${PLATFORM_ROOTFS}/lib/modules
#export ROOTFS_MODULES_DIR=${TARGET_ROOTFS}/lib/modules/${LINUX_VERSION}

#RAMDISK_BLOCKS=50000

#CROSS_COMPILE_PATH=${ELDK_DIR}/bin:${ELDK_DIR}/usr/bin

# Lua socket library requires this for some builds.
#export USE_LIBGCC_S=1
