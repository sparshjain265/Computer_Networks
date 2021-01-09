#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "timers.h"
#include "phy.h"
#include "net.h"
#include <errno.h>

bool computeCRC(Frame *frame) {
	//CRC-32 to be used which has ones in the following powers of x
	//32, 26, 23, 22, 16, 12, 11, 10, 8, 7, 5, 4, 2, 1, 0
	
	return true;
}

bool checkCRC(Frame *frame) {
	return true;
}

int main(int argc, char** argv) {
	Event event;
	initEvents();
	initTimers();
	initPhy(strtol(argv[2], NULL, 10), argv[1], strtol(argv[3], NULL, 10));
	initNet();
		
	while(1) {
		waitForEvent(&event);
		
		switch(event.eventType) {
			case TIMEOUT:
				debugPrint("Timeout event occured");
				Timer *timer = (Timer*) event.eventData;
				printf("Timeout for SEQ no %d\n", timer->seqNo);

				break;
			case ACK_TIMEOUT:
				debugPrint("ACK timeout event occured");

				break;	
			case FRAME_ARRIVAL:
				debugPrint("Frame arrival event occured");
				toNetworkLayer((Frame*) event.eventData);
				break;
			case NETWORK_LAYER_READY:
				debugPrint("Network layer is ready");
				Frame f;
				fromNetworkLayer(&f);
				computeCRC(&f);
				sendFrame(&f);
				break;	
			default:
				debugPrint("Unknown event occured");
				break;
		}
		eventCleanup(&event);
	}

	return 0;
}

