#!/bin/bash 
mac_addr=$1

while !(journalctl -b -u bluetooth | grep "Endpoint registered"); do sleep 1; done
echo -e "power on\nconnect $mac_addr \nquit" | bluetoothctl

/home/pi/btmis/tether.sh $mac_addr &
/home/pi/btmis/bin/btmis /dev/ttyUSB0 $mac_addr

