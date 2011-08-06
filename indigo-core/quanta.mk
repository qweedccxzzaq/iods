# Copyright (c) Big Switch Networks 2011 and Stanford University, 2010
#
# Makefile fragment common to Quanta platforms

all: target-products copy-to-latest md5 tftp-rfs
	echo "Built ${TARGET_ROOTFS_IMAGE}"

include common.mk

export ELDK_DIR=${TOOLS_DIR}/ELDK_4.2
export PATH:=${PATH}:/sbin:${ELDK_DIR}/bin:${ELDK_DIR}/usr/bin

# Where to link linux source directory for tools
export ELDK_LINUX_LINK=${ELDK_DIR}/usr/src/linux

LINUX_CONFIG_COMMAND=echo "Assuming ${LINUX_ROOT} is already configured"
LINUX_UIMAGE_SOURCE=${LINUX_ROOT}/arch/${ARCH}/boot/uImage
OF_HW_PLAT = 1

ifndef BCM_REF
PLATFORM_DRIVERS=quanta-drivers
PLATFORM_DRIVERS+=device-tree
DRIVERS_CLEAN=quanta-drivers-clean
endif

PLAT_DEVS_TAR = ${CONFIG_DIR}/${PLAT}-dev.tar

TARGET_ROOTFS_IMAGE=${PRODUCT_DIR}/uInitrd2m

# For Prontos, this is what we use as flash image
FLASH_IMAGE=${TARGET_ROOTFS_IMAGE}

target-products: ${TARGET_ROOTFS_IMAGE}

target-rootfs-init: ${PRODUCT_DIR}
	rm -rf ${TARGET_ROOTFS}
	mkdir -p ${TARGET_ROOTFS}
	sudo cp -a ${COMMON_ROOTFS_TEMPLATE}/* ${TARGET_ROOTFS}/
	sudo cp -a ${PRONTO_ROOTFS_TEMPLATE}/* ${TARGET_ROOTFS}/
	sudo cp -a ${PLATFORM_ROOTFS_TEMPLATE}/* ${TARGET_ROOTFS}/

RAMDISK_TMP=${CORE_DIR}/tmp-files/ram_disk-${BLD_EXT}
# Note requires sudo for mount, etc
# Requires MKIMAGE_FLAGS defined
${TARGET_ROOTFS_IMAGE}: ${TARGET_ROOTFS} dev ${PRODUCT_DIR}
	echo "Creating ramdisk image in ${RAMDISK_TMP}"
	rm -frd ${RAMDISK_TMP}
	mkdir ${RAMDISK_TMP}
	dd if=/dev/zero of=initrd2m bs=1k count=${RAMDISK_BLOCKS}
	mke2fs -F -m0 initrd2m
	sudo mount -o loop initrd2m ${RAMDISK_TMP}
	sudo cp -a ${TARGET_ROOTFS}/* ${RAMDISK_TMP}/
	sudo rm -rf ${RAMDISK_TMP}.saved
	sudo cp -a ${RAMDISK_TMP} ${RAMDISK_TMP}.saved
	sudo umount ${RAMDISK_TMP}
	cp initrd2m ${PRODUCT_DIR}/initrd2m-${BLD_EXT}
	gzip -f -9 initrd2m
	mkimage ${MKIMAGE_FLAGS} -d initrd2m.gz uInitrd2m
	mv -f uInitrd2m $@
	mv initrd2m.gz tmp-files/initrd2m-${BLD_EXT}.gz

# The directory that looks like the root FS on the target used
# to generate the final image

target-rootfs: ${TARGET_ROOTFS}
${TARGET_ROOTFS}: rootfs-common ${PLATFORM_DRIVERS} html-files dev
	mkdir -p ${ROOTFS_MODULES_DIR}
	echo "${VERSION_NAME}: ${BUILD_STRING}" > ${VERSION_FILE}
	mkdir -p ${TARGET_ROOTFS}/proc # Needed for reboot, etc 
	mkdir -p ${TARGET_ROOTFS}/tmp  # Needed by vlog

################################################################
#
# Other Quanta targets
#
################################################################

MKIMAGE_FLAGS	= -A ppc -O linux -T ramdisk -C gzip -a 00000000 -e 00000000
PLATFORM_TARGETS = ${PRODUCT_DIR}/uInitrd2m ${PRODUCT_DIR}/uImage

export MODULE_INSTALL_DIR=${TARGET_ROOTFS}/lib/modules
export ROOTFS_MODULES_DIR=${TARGET_ROOTFS}/lib/modules/${LINUX_VERSION}

QUANTA_DRV_DIR = ${PLATFORM_ROOT}/quanta-drivers
QUANTA_DRV_PRODUCTS = ${QUANTA_DRV_DIR}/flashDrv/flashDrv.ko
QUANTA_DRV_PRODUCTS	+= ${QUANTA_DRV_DIR}/i2cDrv/i2cDrv.ko
QUANTA_DRV_PRODUCTS	+= ${QUANTA_DRV_DIR}/pciDrv/pciDrv.ko
QUANTA_DRV_PRODUCTS	+= ${QUANTA_DRV_DIR}/cpuDrv/cpuDrv.ko

quanta-drivers: target-rootfs-init ${FLASH_KERNEL}
	make -C ${PLATFORM_ROOT}/quanta-drivers all
	cp ${QUANTA_DRV_PRODUCTS} ${TARGET_ROOTFS}/lib/modules

quanta-drivers-clean:
	make -C ${PLATFORM_ROOT}/quanta-drivers clean

dev: target-rootfs-init
	cd ${TARGET_ROOTFS} && sudo tar xf ${PLAT_DEVS_TAR}

################################################################
#
# Device tree
#
################################################################

DEV_TREE_SOURCE=${LINUX_ROOT}/arch/powerpc/boot/dts/${PLAT}.dts
device-tree: ${PRODUCT_DIR}
	${LINUX_ROOT}/arch/powerpc/boot/dtc -O dtb \
		-o ${PRODUCT_DIR}/device-tree.dtb -b 0 -p 1024 ${DEV_TREE_SOURCE}
	cp ${DEV_TREE_SOURCE} ${PRODUCT_DIR}/device-tree-source.dts

.PHONY: all device-tree release target-rootfs quanta-drivers \
	quanta-drivers-clean dev device-tree release

