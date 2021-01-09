#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

#ifndef LIST_H
#define LIST_H
#include "linkedlist.h"
#endif

#define DEBUG 0
#define debugPrint(x) if(DEBUG) printf("DEBUG: %s\n", x);

typedef enum {TIMEOUT, ACK_TIMEOUT, FRAME_ARRIVAL, NETWORK_LAYER_READY} EventType;

typedef struct Event_t {
	unsigned long int id;
	EventType eventType;
	void *eventData;
	void (*cleanupFunc)(void*);
} Event;

unsigned long eventId;

List eventList;

pthread_mutex_t eventListLock;

Event *gEvent;

int gotEvent;

int initEvents();

void waitForEvent(Event *event);

int eventCleanup(Event *event);

void enqueueEvent(EventType eventType, void *eventData, void (*cleanupFunc)(void*));
