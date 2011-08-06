#!/bin/sh
#
# Copyright 2011 Big Switch Networks
#
# Script to automatically restart ofprotocol.sh
#

source /etc/find-env

log_file=$log_dir/ofproto.log

if test -e $log_file; then
    mv $log_file $log_file.old
fi

if test -z "$controller_ip"; then
    echo "No controller IP address set; exitting"
    echo "No controller IP address set; exitting" > $log_file
    exit 1
fi

echo "Starting ofprotocol" > $log_file

while [ 1 ]
do
    # Resource find-env in case changed since original start
    source /etc/find-env
    date >> $log_file
    if test -z "$controller_port"; then export controller_port=6633; fi
    echo "ofp_options = $ofp_options" >> $log_file
    echo "controller_ip = $controller_ip" >> $log_file
    echo "controller_port = $controller_port" >> $log_file
    fail_arg=""
    [ "$fail_mode" != "" ] && fail_arg="--fail=$fail_mode"
    echo "fail_mode = $fail_mode" >> $log_file
    /sbin/ofprotocol $ofp_options $fail_arg --out-of-band \
        tcp:127.0.0.1 tcp:$controller_ip:$controller_port >> $log_file 2>&1
    sleep 1
    echo "ofprotocol terminated" >> $log_file
    echo "Restarting ofprotocol in 5 seconds" >> $log_file
    sleep 5
done
