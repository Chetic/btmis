#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <dbus/dbus.h>
#include "canusb.h"

static int s_running = 1;

void sig_handler(int signo)
{
	if (signo == SIGINT)
		s_running = 0;
}

//dbus-send --system --dest=org.bluez --type=method_call --print-reply /org/bluez/hci0/dev_BC_F5_AC_52_F5_69 org.bluez.Device1.Connect
//dbus-send --system --dest=org.bluez --type=method_call --print-reply /org/bluez/hci0/dev_BC_F5_AC_52_F5_69 org.bluez.Device1.Disconnect

#define BLUEZ_NAME  "org.bluez"
#define BLUEZ_OPATH "/org/bluez/hci0/dev_BC_F5_AC_52_F5_69"
#define BLUEZ_DEVICE_IFACE "org.bluez.Device1"
#define BLUEZ_MEDIA_OPATH "/org/bluez/hci0/dev_BC_F5_AC_52_F5_69/player0"
#define BLUEZ_MEDIA_IFACE "org.bluez.MediaPlayer1"

void terminateOnError(const char* msg, const DBusError* error) {
	if (dbus_error_is_set(error)) {
		fprintf(stderr, msg);
		fprintf(stderr, "DBusError.name: %s\n", error->name);
		fprintf(stderr, "DBusError.message: %s\n", error->message);
		/* If the program wouldn't exit because of the error, freeing the
		 DBusError needs to be done (with dbus_error_free(error)).
		 NOTE:
		 dbus_error_free(error) would only free the error if it was
		 set, so it is safe to use even when you're unsure. */
		exit(EXIT_FAILURE);
	}
}

void init_dbus(void) {
	/* Structure representing the connection to a bus. */
	DBusConnection* bus = NULL;
	/* The method call message. */
	DBusMessage* msg = NULL;
	DBusMessage* reply = NULL;
	DBusError error;
	dbus_error_init(&error);
	bus = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
	terminateOnError("Failed to open Session bus\n", &error);
	if (!dbus_bus_name_has_owner(bus, BLUEZ_NAME, &error)) {
		fprintf(stderr, "Name has no owner on the bus!\n");
		exit(EXIT_FAILURE);
	}
	msg = dbus_message_new_method_call(BLUEZ_NAME,
										BLUEZ_MEDIA_OPATH,
										BLUEZ_MEDIA_IFACE,
										"Play"); /* Method str */
	if (msg == NULL) {
		fprintf(stderr, "Ran out of memory when creating a message\n");
		exit(EXIT_FAILURE);
	}
	dbus_message_set_no_reply(msg, TRUE);
	if (!dbus_connection_send(bus, msg, NULL)) {
		fprintf(stderr, "Ran out of memory while queueing message\n");
		exit(EXIT_FAILURE);
	}
	dbus_connection_flush(bus);
}

int main(int argc, char* argv[])
{
	init_dbus();

	if (argc < 2)
	{
		printf(
				"Too few arguments. Please provide the path to your serial port (e.g. /dev/ttyUSB0)\n");
		exit(1);
	}

	if (signal(SIGINT, sig_handler) == SIG_ERR)
		printf("\ncan't catch SIGINT\n");

	canusb_init(argv[1]);

	printf("Version:\t");
	canusb_print_version();
	printf("Serial number:\t");
	canusb_print_serial_number();
	printf("Status flags:\t");
	canusb_print_status();

	printf("Disabling timestamps... \t");
	canusb_disable_timestamps();
	printf("Setting acceptance code... \t");
	canusb_send_cmd("M00001D60\r");
	printf("Setting acceptance mask... \t");
	canusb_send_cmd("m00001FF0\r");
	printf("Setting CANUSB to 100kbps... \t");
	canusb_send_cmd("S3\r");
	printf("Opening CAN channel... \t\t");
	canusb_send_cmd("O\r");

	while (s_running)
	{
		int i;
		CANFrame* frame;
		int n;

		n = canusb_poll();

		for (i = 0; i < n; i++)
		{
			frame = canusb_get_frame(i);

			if (frame->id == 0x1D6)
			{
				printf("Steering wheel: %02x%02x\n", frame->data[0], frame->data[1]);

				if ((frame->data[0] & 0x01))
					system("./playpause.sh");
				if ((frame->data[0] & 0x20))
					system("./next.sh");
				if ((frame->data[0] & 0x10))
					system("./prev.sh");
			}
		}
		canusb_reset();
	}

	printf("\rClosing CAN channel... \t\t");
	canusb_send_cmd("C\r");

	return 0;
}
