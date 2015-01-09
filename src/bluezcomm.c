/*
 * bluezcomm.c
 *
 *  Created on: Jan 9, 2015
 *      Author: freveny
 */
#include <stdlib.h>
#include <stdio.h>
#include <dbus/dbus.h>

static DBusConnection* bus = NULL;
static DBusMessage* msg = NULL;
static DBusMessage* reply = NULL;
static DBusError error;

const int BLUEZ_MAC_IDX = 20; //Where to place device address
const char* DEVICE_ADDRESS = "BC_F5_AC_52_F5_69";

const char* BLUEZ_NAME = "org.bluez";
const char* BLUEZ_DEVICE_OPATH = "/org/bluez/hci0/dev_XX_XX_XX_XX_XX_XX";
const char* BLUEZ_DEVICE_IFACE = "org.bluez.Device1";
const char* BLUEZ_MEDIA_OPATH = "/org/bluez/hci0/dev_XX_XX_XX_XX_XX_XX/player0";
const char* BLUEZ_MEDIA_IFACE = "org.bluez.MediaPlayer1";

int terminateOnError(const char* msg, const DBusError* error) {
	if (dbus_error_is_set(error)) {
		perror(msg);
		fprintf(stderr, "DBusError.name: %s\n", error->name);
		fprintf(stderr, "DBusError.message: %s\n", error->message);
		/* If the program wouldn't exit because of the error, freeing the
		 DBusError needs to be done (with dbus_error_free(error)).
		 NOTE:
		 dbus_error_free(error) would only free the error if it was
		 set, so it is safe to use even when you're unsure. */
		return 1;
	}
	return 0;
}

static int bluezcomm_device_call(const char* btmac, const char* method) {
	msg = dbus_message_new_method_call(BLUEZ_NAME, BLUEZ_DEVICE_OPATH,
			BLUEZ_DEVICE_IFACE, method);
	if (msg == NULL) {
		fprintf(stderr, "Ran out of memory when creating a message\n");
		return 1;
	}

	dbus_message_set_no_reply(msg, TRUE);

	if (!dbus_connection_send(bus, msg, NULL)) {
		fprintf(stderr, "Ran out of memory while queueing message\n");
		return 1;
	}

	dbus_connection_flush(bus);
}

static int bluezcomm_media_call(const char* btmac, const char* method) {
	msg = dbus_message_new_method_call(BLUEZ_NAME, BLUEZ_MEDIA_OPATH,
			BLUEZ_MEDIA_IFACE, method);
	if (msg == NULL) {
		fprintf(stderr, "Ran out of memory when creating a message\n");
		return 1;
	}

	dbus_message_set_no_reply(msg, TRUE);

	if (!dbus_connection_send(bus, msg, NULL)) {
		fprintf(stderr, "Ran out of memory while queueing message\n");
		return 1;
	}

	dbus_connection_flush(bus);
}

int bluezcomm_init(void) {
	dbus_error_init(&error);

	bus = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
	if (terminateOnError("Failed to open Session bus\n", &error)) {
		return 1;
	}

	if (!dbus_bus_name_has_owner(bus, BLUEZ_NAME, &error)) {
		fprintf(stderr, "Bluez has no owner on the bus!\n");
		return 1;
	}

	if (bluezcomm_device_call(DEVICE_ADDRESS, "Connect")) {
		return 1;
	}

	return 0;
}

int bluezcomm_close(void) {
	return 0;
}

int bluezcomm_media_playpause(void) {
	return bluezcomm_media_call(DEVICE_ADDRESS, "Pause");
}

int bluezcomm_media_next(void) {
	return bluezcomm_media_call(DEVICE_ADDRESS, "Next");
}

int bluezcomm_media_prev(void) {
	return bluezcomm_media_call(DEVICE_ADDRESS, "Previous");
}
