#!/bin/sh
#
# Sample Indigo pre-init script (indigo-preinit.sh)
#

source /etc/find-env

if test -d $client_root/overlay; then
    echo "Found overlay directory in client_root $client_root"
    cp -a $client_root/overlay/* /
elif test -d $sfs_dir/overlay; then
    echo "Found overlay directory in sfs_dir $sfs_dir"
    cp -a $sfs_dir/overlay/* /
fi
