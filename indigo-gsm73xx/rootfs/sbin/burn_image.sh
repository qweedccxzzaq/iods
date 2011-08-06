#!/bin/sh

# usage :
#    burn_image.sh <image_name> quiet
#

Q="$2"

[ -z $1 ] && echo "No image_name" && exit 0

IMAGE_SIGN_FILE="/etc/image_sign" 

image_sign_size=0
[ -e $IMAGE_SIGN_FILE ] && image_sign_size=$(stat -c %s $IMAGE_SIGN_FILE)

dd if=$1 of=pkg.tar.bz2 ibs=$image_sign_size skip=1 2>/dev/null || exit 0

[ -z $Q ] && F="-v"

[ -z $Q ] && echo "Decomress ......"
tar -xjf pkg.tar.bz2 

if [ ! -e pkg.tar.bz2 ] ; then
echo "Format error" && exit 0
fi

[ -z $Q ] && echo "Burnning flash ......."

if [ -f ./*loader*.bin ] ; then
[ -z $Q ] && echo "updating loader..."
loader_mtd=$(cat /proc/mtd | grep "loader" | awk -F ":" '{printf $1}')
flashcp $F ./*loader*.bin /dev/$loader_mtd
[ -z $Q ] && echo "OK"
fi

if [ -f ./*kernel*.bin ] ; then
[ -z $Q ] && echo "updating kernel..."
kernel_mtd=$(cat /proc/mtd | grep "kernel" | awk -F ":" '{printf $1}')
flashcp $F ./*kernel*.bin /dev/$kernel_mtd
[ -z $Q ] && echo "OK"
fi

need_reboot.sh
[ $? -ne 0 ] && R="-r"

if [ -f ./*rootfs*.bin ] ; then
[ -z $Q ] && echo "updating rootfs..."
rootfs_mtd=$(cat /proc/mtd | grep "rootfs" | awk -F ":" '{printf $1}')
flashcp $F $R ./*rootfs*.bin /dev/$rootfs_mtd
[ -z $Q ] && echo "OK"
fi

## remove temp file.
rm *.bin pkg.tar.bz2

