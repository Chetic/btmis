#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
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
	setbuf(stdout, NULL);

	if (argc < 3)
	{
		printf(
				"Too few arguments. Example usage: btmis /dev/ttyUSB0 AB_CD_EF_01_02_03\n");
		exit(1);
	}

	if (signal(SIGINT, sig_handler) == SIG_ERR)
		printf("\ncan't catch SIGINT\n");

	canusb_init(argv[1]);

	if (bluezcomm_init(argv[2])) {
		printf("Unable to initialize Bluetooth. Shutting down...\n");
		bluezcomm_close();
		exit(1);
	}

	printf("Disabling timestamps... \t");
	canusb_disable_timestamps();
	printf("Setting CANUSB to 100kbps... \t");
	canusb_send_cmd("S3\r");
	//printf("Setting acceptance code... \t");
	//canusb_send_cmd("M00001D60\r");
	//printf("Setting acceptance mask... \t");
	//canusb_send_cmd("m00001FF0\r");
	printf("Opening CAN channel... \t\t");
	canusb_send_cmd("O\r");

	CANFrame lastFrame;
	canusb_reset_frame(&lastFrame);

	while (s_running)
	{
		int i, n;
		CANFrame* frame;

		n = canusb_poll();

		for (i = 0; i < n; i++)
		{
			frame = canusb_get_frame(i);

			printf("Received CAN frame: %03x\n", frame->id);

			if (frame->id == 0x1D6)
			{
				printf("Steering wheel: %02x%02x\n", frame->data[0], frame->data[1]);

				if (!(lastFrame.data[0] & 0x01) && (frame->data[0] & 0x01))
				{
					bluezcomm_media_playpause();
				}
				if (!(lastFrame.data[0] & 0x20) && (frame->data[0] & 0x20))
				{
					bluezcomm_media_next();
				}
				if (!(lastFrame.data[0] & 0x10) && (frame->data[0] & 0x10))
				{
					bluezcomm_media_prev();
				}

				memcpy(&lastFrame, frame, sizeof(CANFrame));
			}
		}
		canusb_reset();
	}

	printf("\rClosing CAN channel... \t\t");
	canusb_send_cmd("C\r");
	bluezcomm_close();

	return 0;
}
