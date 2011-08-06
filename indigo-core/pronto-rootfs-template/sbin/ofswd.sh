#!/bin/sh
#
# Copyright 2011 Big Switch Networks
#
# Script to automatically restart ofswd
#

source /etc/find-env

log_file=$log_dir/ofswd.log

if test -e $log_file; then
    mv $log_file $log_file.old
fi

echo "Starting ofswd" > $log_file
while [ 1 ]
do
    source /etc/find-env
    date >> $log_file
    cd $client_root
    /sbin/ofswd < /dev/null >> $log_file 2>&1
    sleep 1
    echo "ofswd terminated" >> $log_file
    echo "Restarting ofswd in 5 seconds" >> $log_file
    sleep 5
done
