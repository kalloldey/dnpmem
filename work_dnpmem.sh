#!/bin/bash
#sudo su
#./a.sh &>> a.txt
cd /home/kallol
echo "DNP MEM AUTO SCRIPT START"
if [ -f "/home/kallol/.ONCE.txt" ]
then
	echo "[work.sh tells :] OK! Executing first time after a restart."
        rm .ONCE.txt
else
	echo "[work.sh tells :] Already executed once after the restart.!!!"
        exit 2
fi
cd dev/igbvf
make clean -s
if [ $? -ne 0 ]
then
        echo "[work.sh tells :] Error in make clean of igbvf"
        exit 2
fi
echo > /var/log/kern.log
wait
make -s
if [ $? -ne 0 ]
then
        echo "[work.sh tells :] Error in make of igbvf"
        exit 2
fi
wait
echo > /var/log/syslog
wait
rmmod igb
wait
modprobe igb max_vfs=2
wait
dmesg -C
wait
cd ..
wait
cd xen-netback
wait
make clean -s
if [ $? -ne 0 ]
then
        echo "[work.sh tells :] Error in make clean of xen-netback"
        exit 2
fi
cp ../igbvf/igbvf.h .
cp ../igbvf/Module.symvers .
wait
make -s
if [ $? -ne 0 ]
then
        echo "[work.sh tells :] Error in make of netback"
        exit 2
fi
wait
rmmod igbvf
wait
cd ..
wait
cd igbvf
wait
insmod igbvf.ko
wait
cd ..
wait 
cd xen-netback
insmod xen-netback.ko
wait
ifup eth1
wait
ifup eth5
wait
ifup eth6
wait
ifup eth2
wait
brctl addbr xenbr0
wait
ifup xenbr0
wait
cd vm
wait
xl create sd_vm
wait
ethtool -K eth5 rx off
wait
ethtool -K eth5 tx off
wait
ethtool -K eth5 tso off
wait
ethtool -K eth5 gro off
wait
ethtool -K eth5 gso off
echo "DNP MEM AUTO SCRIPT END"
vncviewer 0 
