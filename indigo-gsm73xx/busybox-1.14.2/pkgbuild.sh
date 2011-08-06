#!/bin/bash

PKG_NAME="busybox-1.14.2"


function copy_defconfig () {
[ ! -f $BUILDCFG/busyboxcfg ] && exit 0

cp $BUILDCFG/busyboxcfg .config
make oldconfig || exit 0
}

function build_image () {
make CONFIG_PREFIX=$ROOTFS || exit 0
}

function install_image () {
make CONFIG_PREFIX=$ROOTFS install || exit 0
}

INIT_DIR=$ROOTFS/etc/init.d

function install_init_script () {
[ -d $INIT_DIR ] || mkdir -p $INIT_DIR

## General server 
[ -e $INIT_DIR/S101busybox ] && rm -rf $INIT_DIR/S101busybox
cat > $INIT_DIR/S101busybox << EOF
#!/bin/sh

PATH=/bin:/sbin

## telnet daemon
if [ -x /sbin/telnetd ]; then
[ ! -d /dev/pts ] && {
	mkdir -p /dev/pts
	mount -t devpts none /dev/pts
}
#telnetd
fi

EOF
chmod 755 $INIT_DIR/S101busybox

}

case "$1" in
	prepare)
		copy_defconfig
		build_image
		exit 1
		;;
	config)
		copy_defconfig
		exit 1
		;;
	compile)
		build_image
		exit 1
		;;
	install)
		install_image
		install_init_script
		exit 1
		;;
	clean)
		make clean 
		exit 1
		;;
	distclean)
		make mrproper
		exit 1
		;;
esac

exit 1
