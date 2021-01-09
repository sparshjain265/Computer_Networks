#include "events.h"

void eventHandler(int signum, siginfo_t *info, void *ptr) {
	if(gotEvent)
		return;
		
	pthread_mutex_lock(&eventListLock);
	Event *lEvent = (Event*) dequeueFromList(&eventList);
	if(lEvent) {
		gEvent->id = lEvent->id;
		gEvent->eventType = lEvent->eventType;
		gEvent->eventData = lEvent->eventData;
		gEvent->cleanupFunc = lEvent->cleanupFunc;
		free(lEvent);
		pthread_mutex_unlock(&eventListLock);
		gotEvent = 1;
		return;
	}
	
	pthread_mutex_unlock(&eventListLock);
	return;
}

int initEvents() {
	createList(&eventList);
	pthread_mutex_init(&eventListLock, NULL);
	
	struct sigaction sig_act;
	memset(&sig_act, 0, sizeof(sig_act));
	sig_act.sa_flags = SA_SIGINFO;
	sig_act.sa_sigaction = (void* )eventHandler;
	sigemptyset(&sig_act.sa_mask);
	
	return sigaction(SIGUSR1, &sig_act, NULL);
	
	gEvent = NULL;
	eventId = 0;
	
	return 0;
}

int eventCleanup(Event *event) {
	if(!event)
		return -1;
		
	if(event->cleanupFunc)
		event->cleanupFunc(event->eventData);
	else
		if(event->eventData)
			free(event->eventData);
	gotEvent = 0;		
	return 0;
}

void waitForEvent(Event *event) {
	gEvent = event;
	
	pthread_mutex_lock(&eventListLock);
	if(eventList.length > 0) {
		Event *lEvent = (Event*) dequeueFromList(&eventList);
		event->id = lEvent->id;
		event->eventType = lEvent->eventType;
		event->eventData = lEvent->eventData;
		event->cleanupFunc = lEvent->cleanupFunc;
		free(lEvent);
		gotEvent = 1;
		pthread_mutex_unlock(&eventListLock);
		return;
	}
	
	pthread_mutex_unlock(&eventListLock);
	gotEvent = 0;
	
	sigset_t mask, oldmask;

	sigemptyset (&mask);
	sigaddset (&mask, SIGUSR1);
	sigprocmask (SIG_BLOCK, &mask, &oldmask);
	while(!gotEvent)
		sigsuspend (&oldmask);
	sigprocmask (SIG_UNBLOCK, &mask, NULL);	
}


void enqueueEvent(EventType eventType, void *eventData, void (*cleanupFunc)(void*)) {
	Event *event = (Event*) malloc(sizeof(Event));
	event->id = eventId++;
	
	event->eventType = eventType;
	event->eventData = eventData;
	event->cleanupFunc = cleanupFunc;
	
	pthread_mutex_lock(&eventListLock);
	appendToList(&eventList, event);
	pthread_mutex_unlock(&eventListLock);
}
