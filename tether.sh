#!/bin/bash 
mac_addr=$1
mac_addr_dbus_fmt=${mac_addr//:/_}
while true
do
  echo -e "power on\nconnect $mac_addr \nquit" | bluetoothctl
  dbus-send --system --print-reply --type=method_call --dest=org.bluez /org/bluez/hci0/dev_$mac_addr_dbus_fmt org.bluez.Network1.Connect string:'nap'
  # journalctl will show:
  #Jan 01 00:15:27 alarmpi bluetoothd[224]: bnep0 disconnected
  # journalctl will show if tethering not enabled on phone:
  #Jan 01 00:15:27 alarmpi bluetoothd[224]: bnep%d connected
  if [[ "$(journalctl -u bluetooth --lines=1 -o cat)" == *" connected" ]]; then
    ifconfig bnep0 192.168.44.45 netmask 255.255.255.0 up
  fi
  sleep 10
done

