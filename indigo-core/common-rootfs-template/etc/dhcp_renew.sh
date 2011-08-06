#!/bin/sh

RESOLV_CONF="/etc/resolv.conf"


echo `date`
echo "Running udhcpc script renew.sh"
echo $ip >> /var/dhcpfile
echo $subnet >> /var/dhcpfile
echo "Iterface: $interface"
echo "IP: $ip"
echo "Subnet: $subnet"
echo "DNS: $dns"
echo "router: $router"

[ -n "$broadcast" ] && BROADCAST="broadcast $broadcast"
[ -n "$subnet" ] && NETMASK="netmask $subnet"

/sbin/ifconfig $interface $ip $BROADCAST $NETMASK

if [ -n "$router" ];  then 
    while /sbin/route del default gw 0.0.0.0 dev $interface; do
        echo "deleted default gw"
    done

    for i in $router; do
        /sbin/route add default gw $i dev $interface
        echo Added router $i
    done
fi

if [ -n "$dns" ]; then
    echo "Setting DNS"
    if [ -e $RESOLV_CONF ]; then
        cp -f $RESOLV_CONF $RESOLV_CONF.old
    fi
    echo "nameserver $dns" > $RESOLV_CONF
    echo "domain $domain" >> $RESOLV_CONF
fi
