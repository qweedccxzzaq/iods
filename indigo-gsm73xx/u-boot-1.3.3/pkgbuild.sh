#!/bin/bash

PKG_NAME="u-boot-1.3.3"

case "$PROJECT" in
OS-6800) CONFIG_NAME="OS6800" ;;
KINNICK|XORP|NETGEAR) CONFIG_NAME="KINNICK" ;;
NGTT3) CONFIG_NAME="NGTT3" ;;
BES-1010) CONFIG_NAME="BES1010" ;;
GSM73XX) CONFIG_NAME="GSM73XX" ;;
esac

case "$1" in
	config)
		make ${CONFIG_NAME}_config 
		;;
	compile)
		make 
		make env
		;;
	install)
		[ -f u-boot.bin ] && cp u-boot.bin $BIN/$PROJECT-loader.bin
		[ -f tools/env/fw_printenv ] && {
			cp tools/env/fw_printenv $ROOTFS/sbin
		}
		;;
	clean)
		make clean
		;;
	distclean)
		make distclean 
		;;
esac
