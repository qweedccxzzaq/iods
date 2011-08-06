#!/bin/sh

# USAGE
#       mkdefcfg.sh FILENAME
#

[ -z $1 ] && exit 0

echo "!---- default configuration ----" > $1

if [ -x /etc/create_def.sh ] ; then
## use the project specify default config creater.
/etc/create_def.sh			>> $1
else

## make default config file.
port_num=1
[ -e /etc/profile ] && {
port_num=$(cat /etc/profile | grep "PORT_NUM" | awk -F "=" '{printf $2}')
}

echo "!"				>> $1

for p in `seq 1 $port_num` ; do
echo "interface eth$p"			>> $1
echo "!"				>> $1
done

echo "interface lo"			>> $1
echo "!"				>> $1

fi

