/*
 * bluezcomm.c
 *
 *  Created on: Jan 9, 2015
 *      Author: freveny
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dbus/dbus.h>
#include <unistd.h>
#include <stdbool.h>

static DBusConnection* bus = NULL;
static DBusMessage* msg = NULL;
static DBusMessage* reply = NULL;
static DBusError error;

static const int BLUEZ_MAC_IDX = 20; //Where to place device address
static char BLUEZ_DEVICE_OPATH[] = "/org/bluez/hci0/dev_XX_XX_XX_XX_XX_XX";
static char BLUEZ_MEDIA_OPATH[] =
		"/org/bluez/hci0/dev_XX_XX_XX_XX_XX_XX/player0";

int handleDbusError(const char* msg, const DBusError* error) {
	if (dbus_error_is_set(error)) {
		perror(msg);
		fprintf(stderr, "DBusError.name: %s\n", error->name);
		fprintf(stderr, "DBusError.message: %s\n", error->message);
		return 1;
	}
	return 0;
}

//dbus-send --system --dest=org.bluez --print-reply /org/bluez/hci0/dev_XX_XX_XX_XX_XX_XX org.freedesktop.DBus.Properties.Get string:'org.bluez.Device1' string:'Connected'
static int bluezcomm_get_property(const char* iface, const char* opath, const char* property, dbus_bool_t *result) {
	DBusMessageIter iter, subiter;
	int current_type = -1;

	msg = dbus_message_new_method_call("org.bluez", opath,
									   "org.freedesktop.DBus.Properties", "Get");
	if (msg == NULL) {
		fprintf(stderr, "Error: Ran out of memory when creating a message\n");
		return 1;
	}

	if (!dbus_message_append_args(msg,
								  DBUS_TYPE_STRING, &iface,
								  DBUS_TYPE_STRING, &property,
								  DBUS_TYPE_INVALID)) {
		fprintf(stderr, "Ran out of memory while constructing args\n");
		return 1;
	}

	reply = dbus_connection_send_with_reply_and_block(bus, msg, -1, &error);

	if (reply == NULL) {
		handleDbusError("Error sending get property message: ", &error);
		return 1;
	}

	dbus_message_iter_init(reply, &iter);

	if ((current_type = dbus_message_iter_get_arg_type(&iter))
		!= DBUS_TYPE_VARIANT) {
		handleDbusError("Error getting arguments from get property response: ",
						&error);
		return 1;
	}

	dbus_message_iter_recurse(&iter, &subiter);
	dbus_message_iter_get_basic(&subiter, result);

	dbus_connection_flush(bus);
	return 0;
}

static int bluezcomm_set_property(const char* iface, const char* opath, const char* property, dbus_bool_t* value) {
	DBusMessageIter iter, subiter;
	int current_type = -1;

	msg = dbus_message_new_method_call("org.bluez", opath,
									   "org.freedesktop.DBus.Properties", "Get");
	if (msg == NULL) {
		fprintf(stderr, "Error: Ran out of memory when creating a message\n");
		return 1;
	}

	if (!dbus_message_append_args(msg,
								  DBUS_TYPE_STRING, &iface,
								  DBUS_TYPE_STRING, &property,
								  DBUS_TYPE_INVALID)) {
		fprintf(stderr, "Ran out of memory while constructing args\n");
		return 1;
	}

	reply = dbus_connection_send_with_reply_and_block(bus, msg, -1, &error);

	if (reply == NULL) {
		handleDbusError("Error sending get property message: ", &error);
		return 1;
	}

/*	dbus_message_iter_init(reply, &iter);

	if ((current_type = dbus_message_iter_get_arg_type(&iter))
		!= DBUS_TYPE_VARIANT) {
		handleDbusError("Error getting arguments from get property response: ",
						&error);
		return 1;
	}*/

//	dbus_message_iter_recurse(&iter, &subiter);
//	dbus_message_iter_get_basic(&subiter, result);

	dbus_connection_flush(bus);
	return 0;
}

bool bluezcomm_device_get_connected(void) {
	dbus_bool_t connected;
	bluezcomm_get_property("org.bluez.Device1", BLUEZ_DEVICE_OPATH, "Connected", &connected);
	return connected;
}

void bluezcomm_adapter_set_powered(bool value) {
	bluezcomm_set_property("org.bluez.Adapter1", "/org/bluez/hci0", "Powered", (dbus_bool_t*)&value);
}

bool bluezcomm_adapter_get_powered(void) {
	dbus_bool_t powered;
	bluezcomm_get_property("org.bluez.Adapter1", "/org/bluez/hci0", "Powered", &powered);
	return (bool)powered;
}

static int bluezcomm_device_call(const char* method) {
	msg = dbus_message_new_method_call("org.bluez", BLUEZ_DEVICE_OPATH, "org.bluez.Device1", method);
	if (msg == NULL) {
		fprintf(stderr, "Error: Ran out of memory when creating a message\n");
		return 1;
	}

	dbus_message_set_no_reply(msg, TRUE);

	if (!dbus_connection_send(bus, msg, NULL)) {
		fprintf(stderr, "Error: Ran out of memory while queueing message\n");
		return 1;
	}

	dbus_connection_flush(bus);
	return 0;
}

static int bluezcomm_media_call(const char* method) {
	msg = dbus_message_new_method_call("org.bluez", BLUEZ_MEDIA_OPATH, "org.bluez.MediaPlayer1", method);
	if (msg == NULL) {
		fprintf(stderr, "Error: Ran out of memory when creating a message\n");
		return 1;
	}

	dbus_message_set_no_reply(msg, TRUE);

	if (!dbus_connection_send(bus, msg, NULL)) {
		fprintf(stderr, "Error: Ran out of memory while queueing message\n");
		return 1;
	}

	dbus_connection_flush(bus);
	return 0;
}

int bluezcomm_init(char* btmac) {
	int i, retries = 20;
	dbus_bool_t is_connected;

	printf("Connecting to Bluetooth device %s...\n", btmac);
	if (strlen(btmac) != 17) {
		perror("Error: Invalid MAC address length\n");
		return 1;
	}
	for (i = 0; i < 5; i++)
		btmac[2 + 3 * i] = '_';
	memcpy(BLUEZ_DEVICE_OPATH + BLUEZ_MAC_IDX, btmac, 17);
	memcpy(BLUEZ_MEDIA_OPATH + BLUEZ_MAC_IDX, btmac, 17);

	dbus_error_init(&error);

	bus = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
	if (handleDbusError("Error: Failed to open Session bus\n", &error)) {
		return 1;
	}

	if (!dbus_bus_name_has_owner(bus, "org.bluez", &error)) {
		fprintf(stderr, "Error: Bluez has no owner on the bus!\n");
		return 1;
	}

	while (retries--) {
		if (bluezcomm_device_call("Connect")) {
			handleDbusError("Error connecting: \n", &error);
			return 1;
		}

		if (bluezcomm_device_get_connected()) {
			break;
		} else {
			printf("Waiting for device... Is connected: %d\n", is_connected);
		}
		usleep(500000);
	}

	return 0;
}

int bluezcomm_close(void) {
	printf("Disconnecting from Bluetooth device...\n");
	if (bluezcomm_device_call("Disconnect")) {
		handleDbusError("Error disconnecting: \n", &error);
		return 1;
	}
	return 0;
}

int bluezcomm_media_playpause(void) {
	return bluezcomm_media_call("Pause");
}

int bluezcomm_media_next(void) {
	return bluezcomm_media_call("Next");
}

int bluezcomm_media_prev(void) {
	return bluezcomm_media_call("Previous");
}
