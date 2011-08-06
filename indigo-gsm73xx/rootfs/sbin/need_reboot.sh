#!/bin/sh

# NOTE : If the rootfs is compress file system. we need to reboot the
#        system after image upgrade immediately.
if grep "root . squashfs" /proc/mounts 1>/dev/null; then
	exit 1
fi

exit 0

