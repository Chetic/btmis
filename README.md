# Bluetooth Media Integration System
By hooking up a CANUSB adapter (http://www.can232.com/canusb/) to your car's CAN bus and getting a bluez-compatible Bluetooth dongle, you can stream music from your bluetooth device and change songs using the stock steering wheel buttons (or whatever buttons you can access from the CAN bus).
The CAN messages used in this program are for BMW E90, but can easily be adapted.

Dependencies
============
- bluez 5
- libdbus-1
- https://github.com/Chetic/libcanusb
