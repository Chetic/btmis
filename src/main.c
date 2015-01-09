#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include "canusb.h"
#include "bluezcomm.h"

static int s_running = 1;

void sig_handler(int signo)
{
	if (signo == SIGINT)
		s_running = 0;
}

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		printf(
				"Too few arguments. Example usage: btmis /dev/ttyUSB0 AB_CD_EF_01_02_03\n");
		exit(1);
	}

	if (signal(SIGINT, sig_handler) == SIG_ERR)
		printf("\ncan't catch SIGINT\n");

	canusb_init(argv[1]);
	bluezcomm_init(argv[2]);

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
				{
					bluezcomm_media_playpause();
				}
				if ((frame->data[0] & 0x20))
				{
					bluezcomm_media_next();
				}
				if ((frame->data[0] & 0x10))
				{
					bluezcomm_media_prev();
				}
			}
			else
			{
				printf("Received CAN frame: %03x\n", frame->id);
			}
		}
		canusb_reset();
	}

	printf("\rClosing CAN channel... \t\t");
	canusb_send_cmd("C\r");
	bluezcomm_close();

	return 0;
}
