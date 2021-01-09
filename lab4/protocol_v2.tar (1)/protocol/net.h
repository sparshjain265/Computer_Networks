#include <pthread.h>
#include <stdio.h>
#include <string.h>

#ifndef EVENTS_H
#define EVENTS_H
#include "events.h"
#endif

#ifndef LIST_H
#define LIST_H
#include "linkedlist.h"
#endif

#ifndef LINK_H
#define LINK_H
#include "linklayer.h"
#endif

List netList;
pthread_t netThread;
pthread_mutex_t netListLock;

int initNet();

int netListSize();

int fromNetworkLayer(Frame *f);

int toNetworkLayer(Frame *f);
