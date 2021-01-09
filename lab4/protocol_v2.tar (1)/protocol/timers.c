#include "timers.h"

int timerCmpFunc(void *a, void *b) {
	Timer *ta = (Timer*) a;
	Timer *tb = (Timer*) b;
	return (ta->seqNo == tb->seqNo);
}

int timerCmpSeqNoFunc(void *i, void *a) {
	Timer *ta = (Timer*) a;
	int *seqNo = (int*) i;
	return (ta->seqNo == *seqNo);
}

int timerCmpTimeoutFunc(void *a, void *b) {
	Timer *tb = (Timer*) b;
	struct itimerspec its;
	timer_gettime(tb->timerId, &its);
	return ((its.it_value.tv_sec == 0) && (its.it_value.tv_nsec == 0));
}

void timerCleanupFunc(void *a) {
	Timer *ta = (Timer*) a;
	timer_delete(ta->timerId);
	free(ta);
}

void cleanupTimers() {
	deleteList(&timerList, timerCleanupFunc);
	timerCleanupFunc(ackTimer);
}

void timeoutHandler(int signum, siginfo_t *info, void *ptr) {
	
	pthread_mutex_lock(&timerListLock);
	Node *node = findInList(&timerList, NULL, timerCmpTimeoutFunc);
	
	while(node) {
		enqueueEvent(TIMEOUT, node->data, timerCleanupFunc);
		deleteFromList(&timerList, node, NULL);
		node = findInList(&timerList, NULL, timerCmpTimeoutFunc);
	}
	
	if((ackTimer != NULL) && timerCmpTimeoutFunc(NULL, ackTimer)) {
		enqueueEvent(ACK_TIMEOUT, ackTimer, timerCleanupFunc);
		ackTimer = NULL;
	}
			
	pthread_mutex_unlock(&timerListLock);
	union sigval sval;
	sigqueue(getpid(), SIGUSR1, sval);
}

int initTimers() {
	createList(&timerList);
	pthread_mutex_init(&timerListLock, NULL);
	ackTimer = NULL;
	
	struct sigaction sig_act;
	memset(&sig_act, 0, sizeof(sig_act));
	sig_act.sa_flags = SA_SIGINFO;
	sig_act.sa_sigaction = (void* )timeoutHandler;
	sigemptyset(&sig_act.sa_mask);
	
	return sigaction(SIGUSR2, &sig_act, NULL);
}

int startTimer(unsigned int seqNo) {
	pthread_mutex_lock(&timerListLock);
	
	if(findInList(&timerList, &seqNo, timerCmpSeqNoFunc)) {
		pthread_mutex_unlock(&timerListLock);
		return -1;
	}
	
	Timer *timer = (Timer*) malloc(sizeof(Timer));
	memset(timer, 0, sizeof(Timer));
	timer->timerType = DATA_TIMER;
	timer->seqNo = seqNo;
	
	struct sigevent sig_eve;
	memset(&sig_eve, 0, sizeof(struct sigevent));

	sig_eve.sigev_notify = SIGEV_SIGNAL;
	sig_eve.sigev_signo = SIGUSR2;
	sig_eve.sigev_value.sival_ptr = (void*) timer;

	struct itimerspec its;
	memset(&its, 0, sizeof(its));
	its.it_value.tv_sec = TIMER_TIMEOUT_SEC;
	its.it_value.tv_nsec = TIMER_TIMEOUT_NSEC;
	
	timer_create(CLOCK_REALTIME, &sig_eve, &timer->timerId);
	timer_settime(timer->timerId, 0, &its, NULL);
	
	appendToList(&timerList, (void*) timer);
	pthread_mutex_unlock(&timerListLock);
	
	return 0;
}

int stopTimer(unsigned int seqNo) {
	pthread_mutex_lock(&timerListLock);
	findAndDeleteFromList(&timerList, &seqNo, timerCmpSeqNoFunc, timerCleanupFunc);	
	pthread_mutex_unlock(&timerListLock);
	return 0;
}

int startAckTimer() {
	if(ackTimer)
		return -1;
		
	ackTimer = (Timer*) malloc(sizeof(Timer));
	memset(ackTimer, 0, sizeof(Timer));
	
	ackTimer->timerType = ACK_TIMER;
	
	struct sigevent sig_eve;
	memset(&sig_eve, 0, sizeof(struct sigevent));

	sig_eve.sigev_notify = SIGEV_SIGNAL;
	sig_eve.sigev_signo = SIGUSR2;
	sig_eve.sigev_value.sival_ptr = (void*) ackTimer;

	struct itimerspec its;
	memset(&its, 0, sizeof(its));
	its.it_value.tv_sec = ACK_TIMER_TIMEOUT_SEC;
	its.it_value.tv_nsec = ACK_TIMER_TIMEOUT_NSEC;
	
	timer_create(CLOCK_REALTIME, &sig_eve, &ackTimer->timerId);
	timer_settime(ackTimer->timerId, 0, &its, NULL);
	
	return 0;
}

int stopAckTimer(){
	if(!ackTimer)
		return -1;
	
	timer_delete(ackTimer->timerId);
	free(ackTimer);
	ackTimer = NULL;
	
	return 0;
}
