#!/bin/sh
#
# Example Indigo pre-init script.
#

# Get system environment variables
source /etc/find-env

# Copy passwd and shadow if present in config directory

if test -e $client_root/passwd; then
    cp  $client_root/passwd /etc/
elif test -e $sfs_dir/passwd; then
    cp  $sfs_dir/passwd /etc/
fi

if test -e $client_root/shadow; then
    cp  $client_root/shadow /etc/
elif test -e $sfs_dir/shadow; then
    cp  $sfs_dir/shadow /etc/
fi

