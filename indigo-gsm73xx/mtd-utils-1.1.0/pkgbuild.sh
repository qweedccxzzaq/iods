#!/bin/bash

PKG_NAME="mtd-utils-1.1.0"

case "$1" in
	config)
		;;
	compile)
		make WITHOUT_XATTR=1 RAWTARGETS="flashcp flash_eraseall" || exit 0
		exit 1
		;;
	install)
		make WITHOUT_XATTR=1 RAWTARGETS="flashcp flash_eraseall" install || exit 0
		exit 1
		;;
	clean|distclean)
		make clean || exit 0
		;;
esac
