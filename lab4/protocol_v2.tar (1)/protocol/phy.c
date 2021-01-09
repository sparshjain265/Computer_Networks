#include "phy.h"
#include <errno.h>

void cleanupFrame(void *a) {
	free(a);
}

void *phyFunc(void *arg) {
	int size;
	unsigned int cAddrLen;
	struct sockaddr_in cAddr;
	char buffer[sizeof(Frame)];
	
	memset(&cAddr, 0, sizeof(cAddr));
	
	printf("Done\n");
	while(1) {
		size = recvfrom(servSock , buffer, sizeof(Frame), 0, (struct sockaddr*) &cAddr, &cAddrLen); 
		if(size < sizeof(Frame))
			continue;
		Frame *frame = (Frame*) malloc(sizeof(Frame));
		memcpy(frame, buffer, sizeof(Frame));
		enqueueEvent(FRAME_ARRIVAL, frame, cleanupFrame);
		union sigval sval;
		sigqueue(getpid(), SIGUSR1, sval);
	}
}

int initPhy(int srcPort, char *destIp, int dstPort) {
	printf("Initiating phy layer...");
	pthread_mutex_init(&phyLock, NULL);
	
	clientSock = socket(AF_INET, SOCK_DGRAM, 0);
	servSock = socket(AF_INET, SOCK_DGRAM, 0);
	if(clientSock < 0 || servSock < 0)
		return -1;
		
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = inet_addr(destIp);
	servAddr.sin_port = htons(dstPort);
	
	struct sockaddr_in sAddr;
	
	memset(&sAddr, 0, sizeof(sAddr));
	
	sAddr.sin_family = AF_INET;
	sAddr.sin_addr.s_addr = INADDR_ANY;
	sAddr.sin_port = htons(srcPort);
	
	int opt = 1;
	if(setsockopt(servSock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
		return -1;
                                                  
	if(bind(servSock, (struct sockaddr*) &sAddr, sizeof(sAddr)) < 0)
		return -1;
		
	pthread_create(&phyThread, NULL, &phyFunc, NULL);
	return 0;
}


int sendFrame(Frame *frame) {
	Frame *frameError = (Frame*) malloc(sizeof(Frame));
	memcpy(frameError, frame, sizeof(Frame));
	double r = rand()/(RAND_MAX+1.0); 
	
	if(r < 0.4) {
		int i = rand()%2;
		frameError->length[i] = 0;
		frameError->packet.data[i] = 255;
	}
	int ret = sendto(clientSock, (char*) frameError, sizeof(Frame), 0, (struct sockaddr*) &servAddr, sizeof(servAddr));
	free(frameError);
	usleep(100);
	return ret;
}
