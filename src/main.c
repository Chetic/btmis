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

	if (argc != 2) {
		printf("Incorrect number of arguments. Example usage: %s AB_CD_EF_01_02_03\n", argv[0]);
		exit(1);
	}
	if (signal(SIGINT, sig_handler) == SIG_ERR) {
		printf("Can't catch SIGINT\n");
	}
	canusb_filter_id(0x1d6);
	if (canusb_init(0)) {
		printf("ERROR: Failed to initialize canusb\n");
	}
	if (bluezcomm_init(argv[1])) {
		printf("ERROR: Unable to initialize Bluetooth\n");
		bluezcomm_close();
	}

	CANMsg lastFrame;

	while (s_running)
	{
		int i, n;
		CANMsg* frame;

		for (i = 0; i < n; i++)
		{
			if (canusb_get_frame(frame)) { 
				printf("Received CAN frame: %03x", (unsigned int)frame->id);
				if (frame->id == 0x1D6)
				{
					printf("(Steering wheel: %02x%02x)", frame->data[0], frame->data[1]);

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
					memcpy(&lastFrame, frame, sizeof(CANMsg));
				}
				printf("\n");
			}
		}
	}

	bluezcomm_close();
	canusb_close();

	return 0;
}
