#! /bin/bash


export PATH=/run/media/mmcblk0p1/data/nodejs/node-v19.9.0-linux-arm64/bin:$PATH && cd /run/media/mmcblk0p1/data/nodejs/app && node bin/www &
echo $PATH
echo "started node localhost:3000"
 

cp -r ./spinnaker/lib/*  /usr/lib
echo "copy spinnaker libs to /usr/lib"

ifconfig eth0 down

ifconfig eth0 mtu 9000 up

ip link add link eth0 name eth0.10 type vlan id 10

ip addr add 192.168.10.254/24 dev eth0.10

ip link set up eth0.10

ifconfig



 