#!/bin/sh

port_num=1
[ -e /etc/profile ] && {
port_num=$(cat /etc/profile | grep "PORT_NUM" | awk -F "=" '{printf $2}')
}

echo "!"

echo "vlan 1 description default"
echo "!"

echo "service telnet"
echo "!"

for p in `seq 1 $port_num` ; do
echo "interface eth$p"
echo " switchport"
echo " no shutdown"
echo "!"
done

echo "interface lo"
echo "!"


