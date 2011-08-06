#!/bin/sh
#
# Initialization script for OpenFlow release of Quanta switch
#
# See README-config or the OpenFlow wiki for more information
#
# The key environment variable that should be defined is DEV_ADDR
# that identifies the client.

# Order of priority for location of init files
# $client_cfg/ (/config/$DEV_ADDR)
# /config/default/
# $sfs_dir/
# /etc/

# Client specific directory (for logs, etc) is $client_root
# which is normally /client/$DEV_ADDR

echo "Running rc.sh for platform:  __PLATFORM_NAME__"
echo "Release: __INDIGO_REL_NAME__"

echo "Bringing up local devices"
/sbin/ifconfig lo 127.0.0.1

# mount /proc so "reboot" works
/bin/mount -t proc proc /proc

# These are old; not sure they're all right
mount -t devpts devpts /dev/pts

# Get the current environment
source /etc/find-env

if test -z "$DEV_ADDR"; then
    echo "WARNING: DEV_ADDR is not set; 'unknown' for client root"
fi
mkdir -p $client_root
echo "Client directories: config $client_cfg, log $client_root" 

# Try to extract SFS and execute script there.
export sfs_dir=/no_sfs
if test -z "$no_sfs"; then
    if test -e /sbin/sfs_extract; then 
        echo "Extracting SFS"
        /sbin/sfs_extract $client_root
        export sfs_dir=$client_root/sfs
    fi
else
    echo "Not extracting SFS"
fi

# Get the current environment again in case changed due to sfs
source /etc/find-env

if test -e $sys_init_file; then
    echo "Running $sys_init_file"
    source $sys_init_file
else
    echo "No system init file found; manual config required"
fi

echo "Welcome to Indigo.  Release: __INDIGO_REL_NAME__"
echo
