# Bluetooth Media Integration System
By hooking up a CANUSB adapter (http://www.can232.com/canusb/) to your car's CAN bus and getting a bluez-compatible Bluetooth dongle, you can stream music from your bluetooth device and change songs using the stock steering wheel buttons (or whatever buttons you can access from the CAN bus).
The CAN messages used in this program are for BMW E90, but can easily be adapted.

Dependencies
============
- bluez 5
- libdbus-1
- https://github.com/Chetic/libcanusb

Hardware
============
You can probably adapt this to any hardware and OS fairly easily, but here's my suggestion: A Raspberry Pi running Arch Linux with an optimized boot time (since you want it to be ready as soon as possible after starting your car). The trimmed down boot schedule I managed with systemd on my setup will play music from my phone after about 11 seconds. Hopefully I can get this down to something around 5 in the future using the new RPi models, and disabling more unneeded services.
- Raspberry Pi (incl. SD card)
- A car supply/switch that will allow the RPi to shut down safely, like this one: http://mausberrycircuits.com/collections/car-power-supply-switches/products/2a-car-supply-switch
- A CANUSB device: http://www.can232.com/canusb/

