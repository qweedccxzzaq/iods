# Copyright 2011, Big Switch Networks
# Copyright 2010, Stanford University
#
# Top level Makefile for Indigo OpenFlow development
#
# See README-make for more information.

# Make recursively based on target which gives the platform

include base.mk

# Default target is LB9A/3290
all: lb9a

gsm73xx:
	make -f gsm73xx.mk

lb9a:
	make -f lb9a.mk

lb4g:
	make -f lb4g.mk

lb8:
	make -f lb8.mk

t2ref:
	make -f t2ref.mk

scorref:
	make -f scorref.mk

%-clean:
	make -f $*.mk clean

%-show:
	make -f $*.mk show

%-allvars:
	make -f $*.mk allvars

%-iods-prep:
	make -f $*.mk iods-platform BCM_BINARY_RELEASE=1

clean: ${CLEAN_TARGETS}
	@echo "Made all clean"


# Install UI components on local machine
ui-install:
	sudo cp -r ${COMMON_ROOTFS_TEMPLATE}/usr/local/lib/lua/5.1/* \
		/usr/local/lib/lua/5.1

#TEST_ARG=vverb
ui-test: ui-install
	cd ${COMMON_ROOTFS_TEMPLATE}/usr/local/lib/lua/5.1/tests && \
		lua parse.lua -v
	cd ${COMMON_ROOTFS_TEMPLATE}/usr/local/switch_cli && \
		./test-all.sh ${TEST_ARG}

################################################################
#
# Some defines needed for targets below
# WARNING:  These are also defined in common.mk
#
################################################################

MOUNT_DIR=rootfs-mounts
LATEST_IMAGE=${MOUNT_DIR}/rootfs-image-latest

################################################################
#
# Help, show, etc
#
################################################################

help:
	@echo "Make system for Indigo OpenFlow hardware platforms"
	@echo "Targets include:"
	@echo "  lb4g:      Pronto 3240"
	@echo "  lb9a:      Pronto 3290"
	@echo "  lb8:       Pronto 3780"
	@echo "  gsm73xx:   Netgear GSM7328 and GSM7352"
	@echo "The default target is lb9a"
	@echo "Target iods-rel generates IODS release, requires RELEASE_NAME"
	@echo "These targets call the platform with the respective target"
	@echo "  <plat>-clean    Clean the target"
	@echo "  <plat>-show     Show basic variables"
	@echo "  <plat>-allvars  Show many variables and environment"
	@echo "Variables include:"
	@echo "  BCM_BINARY_RELEASE=1: Build binary release"
	@echo "  RELEASE_NAME='name':  Use 'name' for release"
	@echo "  NO_BB_BLD=1:          Do not build busybox"
	@echo "  QUICK_BUILD=1:        Do not build linux, dropbear, lua, haserl, busybox"


# A special target to mount the latest rootfs; requires latest build 
# to have linked its root FS image to tmp/rootfs-image-latest

mount-rfs:
	mkdir -p ${MOUNT_DIR}/rfs
	rm -f ${MOUNT_DIR}/rfs-source ${MOUNT_DIR}/rfs-source.gz
	dd if=${LATEST_IMAGE} of=${MOUNT_DIR}/rfs-source.gz bs=64 skip=1
	(cd ${MOUNT_DIR} && gunzip rfs-source.gz)
	(cd ${MOUNT_DIR} && sudo mount -o loop rfs-source rfs)
	@echo "Mounted latest rootfs image to ${MOUNT_DIR}/rfs"

unmount-rfs:
	(cd ${MOUNT_DIR} && sudo umount rfs)
	@echo "Unmounted rootfs"

.PHONY: all gsm73xx lb9a lb4g lb8 help mount-rfs unmount-rfs iods-init \
	iods-rel ui-install ui-test


