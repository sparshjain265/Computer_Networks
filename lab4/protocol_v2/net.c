#include "net.h"

void netPrintFunc(void *data) {
	printf("%c %c\n", ((char*)data)[0], ((char*)data)[1]);
}	

void *netFunc(void *arg) {
	char str[500];
	while(1) {
		fgets(str, 500, stdin);
		int len = strlen(str);
		pthread_mutex_lock(&netListLock);
		for(int i = 0; i < len; i += 2)  {
			unsigned char *data = (unsigned char*) malloc(sizeof(unsigned char)*2);
			data[0] = str[i];
			if(i+1 < len)
				data[1] = str[i+1];
			else 
				data[1] = 255;
			appendToList(&netList, data);
		}
		pthread_mutex_unlock(&netListLock);
		enqueueEvent(NETWORK_LAYER_READY, NULL, NULL); 
		union sigval sval;
		sigqueue(getpid(), SIGUSR1, sval);
	}
}

int netListSize() {
	return netList.length;
}


int fromNetworkLayer(Frame *f) {
	pthread_mutex_lock(&netListLock);
	unsigned char *data = (unsigned char*) dequeueFromList(&netList);
	pthread_mutex_unlock(&netListLock);
	if(!data)
		return -1;
	memcpy(f->packet.data, data, 2);
	int len = 2;
	if(data[1] == 255)
		len = 1;
	f->length[0] = len & 0xF;
	f->length[1] = (len >> 8) & 0xF;
	if(netListSize() > 0) 
		enqueueEvent(NETWORK_LAYER_READY, NULL, NULL); 
	return 0; 
}

int toNetworkLayer(Frame *f) {
	unsigned char *data = f->packet.data;
	if(!data)
		return -1;
		
	int len = 0;
	len = len | f->length[0] | (f->length[1] << 8);
	
	memcpy(f->packet.data, data, len);
	data[len] = '\0';
	printf("%s", data);
	fflush(stdout);
	return 0; 
}

int initNet() {
	pthread_mutex_init(&netListLock, NULL);
	createList(&netList);
	pthread_create(&netThread, NULL, &netFunc, NULL);
	return 0;
}

