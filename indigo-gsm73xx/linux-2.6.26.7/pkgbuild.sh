#!/bin/bash

PKG_NAME="linux-2.6.26.7"

#########################################################################
## menu config
function menu_config () {
case "$PROJECT" in
	GSM73XX)
		make ARCH=powerpc menuconfig
		;;
	*)
		make menuconfig
		;;
esac
}

#########################################################################
## config routine
function copy_defconfig () {

cp $BUILDCFG/kernelcfg .config
case "$PROJECT" in
	GSM73XX)
		make ARCH=powerpc oldconfig
		;;
	*)
		make oldconfig
		;;
esac
}

#########################################################################
## build routine
function build_image_and_module () {
case "$PROJECT" in
	GSM73XX)
		make ARCH=powerpc cuImage.mpc8533_gsm73xx
		make ARCH=powerpc KBUILD_MODPOST_WARN=y modules
		;;

	*) make Image || exit 0 ;;
esac
}

function redirect_eldk_kernel_link () {

[ -h $NFS/usr/src/linux ] && rm $NFS/usr/src/linux
ln -s $(pwd) $NFS/usr/src/linux

}

#########################################################################
## install routine
function copy_target_image () {

case "$PROJECT" in
	GSM73XX)              SRC_FILE="arch/powerpc/boot/cuImage.mpc8533_gsm73xx" ;;
        *)            
		echo "[Error] : Unknow project" 
		exit 0
		;;
esac

[ -f $SRC_FILE ] && cp $SRC_FILE  $BIN/$PROJECT-kernel.bin

}

#########################################################################
## clean routine.
function package_clean () {
case "$PROJECT" in
	GSM73XX) make ARCH=powerpc clean ;;
	*) make clean ;;
esac
}

#########################################################################
#########################################################################

function redirect_eldk_kernel_link () {
[ -h $NFS/usr/src/linux ] && rm $NFS/usr/src/linux
ln -s $(pwd) $NFS/usr/src/linux
}


INIT_DIR=$ROOTFS/etc/init.d
MODULE_INIT_FILE=$INIT_DIR/S014npe

function install_dev_module () {

## modules load script
[ -d $INIT_DIR ] || mkdir -p $INIT_DIR
[ -e $MODULE_INIT_FILE ] && rm $MODULE_INIT_FILE
cat > $MODULE_INIT_FILE << EOF
#!/bin/sh

PATH=/bin:/sbin

if [ -e /etc/profile ] ; then
port_num=\$(cat /etc/profile | grep "PORT_NUM" | awk -F "=" '{printf \$2}')
mac_addr=\$(cat /etc/profile | grep "MAC_ADDR" | awk -F "=" '{printf \$2}')
fi
[ -z \$port_num ] && port_num=1
[ -z \$mac_addr ] && mac_addr="00:11:88:2d:ab:cc"

## insert modules
if [ -e /drivers/dni_cpld_proc.ko ] ; then
insmod /drivers/dni_cpld_proc.ko
fi

if [ -e /drivers/deth.ko   ] ; then
insmod /drivers/deth.ko eth_num=\$port_num mac_addr=\$mac_addr
fi

if [ -e /drivers/bonding.ko ] ; then 
insmod /drivers/bonding.ko miimon=1000
fi

if [ -e /drivers/llc.ko ] ; then
insmod /drivers/llc.ko
fi

if [ -e /drivers/bridge.ko ] ; then
insmod /drivers/bridge.ko
fi

EOF

chmod 755 $MODULE_INIT_FILE

## copy file
[ -d $ROOTFS/drivers ] || mkdir $ROOTFS/drivers
module_list=$(find . -iname "*.ko")
[ ! -z "$module_list" ] && cp $module_list $ROOTFS/drivers

}

case "$1" in
	export_kernel_path)
		redirect_eldk_kernel_link
		;;
	prepare)
		redirect_eldk_kernel_link
		copy_defconfig
		build_image_and_module
		;;
	config)
		copy_defconfig
		;;
	menuconfig)
		menu_config
		;;
	compile)
		build_image_and_module
		;;
	install)
		copy_target_image
		install_dev_module
		;;
	clean|distclean)
		package_clean
		;;
esac

exit 1
