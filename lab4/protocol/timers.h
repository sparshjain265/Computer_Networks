#ifndef EVENTS_H
#define EVENTS_H
#include "events.h"
#endif

#ifndef LINK_H
#define LINK_H
#include "linklayer.h"
#endif

#ifndef LIST_H
#define LIST_H
#include "linkedlist.h"
#endif

#include <string.h>
#include <stdlib.h>

#define TIMER_TIMEOUT_SEC 0
#define ACK_TIMER_TIMEOUT_SEC 100000
#define TIMER_TIMEOUT_NSEC 0
#define ACK_TIMER_TIMEOUT_NSEC 100000

typedef enum {DATA_TIMER, ACK_TIMER} TimerType;


typedef struct Timer_t {
	unsigned int seqNo;
	TimerType timerType;
	timer_t timerId;
} Timer;

List timerList;
Timer *ackTimer;
pthread_mutex_t timerListLock;

int initTimers();

void cleanupTimers();

int startTimer(unsigned int seqNo);

int stopTimer(unsigned int seqNo);

int startAckTimer();

int stopAckTimer();
