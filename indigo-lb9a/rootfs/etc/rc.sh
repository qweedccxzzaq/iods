#!/bin/sh

/sbin/ifconfig lo 127.0.0.1

/sbin/ifconfig eth0 192.168.2.1
/sbin/ifconfig eth1 192.168.3.1

#>/etc/mtab

# mount /proc so "reboot" works
/bin/mount -t proc proc /proc

mount -t devpts devpts /dev/pts
mount /dev/hda1 /cf_card/
mount -o remount,rw /

insmod /lib/modules/cpuDrv.ko
insmod /lib/modules/flashDrv.ko
insmod /lib/modules/i2cDrv.ko
insmod /lib/modules/pciDrv.ko
insmod /lib/modules/wdtDrv.ko

insmod /lib/modules/linux-kernel-bde.ko
insmod /lib/modules/linux-user-bde.ko

./usr/sbin/telnetd
#insmod /lib/modules/linux-bcm-diag-full.ko
#insmod /lib/modules/linux-bcm-core.ko debug=1 init=all
#insmod /lib/modules/linux-bcm-net.ko

# BCM shell
#insmod /lib/modules/linux-uk-proxy.ko
#mknod /dev/linux-uk-proxy c 125 0
#insmod /lib/modules/linux-bcm-diag.ko debug=1
cd /bin
./bcm.user
#./bcm.user.proxy
