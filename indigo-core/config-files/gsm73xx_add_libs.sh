#!/bin/bash
#
# Stanford OpenFlow build add libraries script
#
# Files are transfered from the Indigo build environment to the 
# Netgear build environment by this script.  The files are listed
# in variables whos names are related to the destination of the files.
# These variables are exported by the Indigo build before this script
# is invoked in the Netgear build environment.
#

echo "Entering add libs script"

if [ -z $ROOTFS ]; then
    echo "Error:  ROOTFS not specified." && exit 1
fi

if [ -z $INDIGO_ROOTFS ]; then
    echo "Error:  INDIGO_ROOTFS not specified." && exit 1
fi

echo "Copying $INDIGO_ROOTFS to $ROOTFS"
cp -a $INDIGO_ROOTFS/* $ROOTFS

echo "Exiting add libs script"
