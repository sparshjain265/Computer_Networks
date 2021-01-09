#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef EVENTS_H
#define EVENTS_H
#include "events.h"
#endif

#ifndef LINK_H
#define LINK_H
#include "linklayer.h"
#endif

struct sockaddr_in servAddr;
int clientSock, servSock;

pthread_t phyThread;
pthread_mutex_t phyLock;

int initPhy(int srcPort, char *destIp, int destPort);

int sendFrame(Frame *frame);
