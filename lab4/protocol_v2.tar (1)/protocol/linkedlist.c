#include "linkedlist.h"

void createList(List *list) {
	list->headNode = NULL;
	list->tailNode = NULL;
	list->length = 0;
}

void deleteList(List *list, void (*cleanup_func)(void*)) {
	Node *tempNode;
	while(list->headNode) {
		tempNode = list->headNode;
		list->headNode = tempNode->nextNode;
		cleanup_func(tempNode->data);
		free(tempNode);
	}
	list->tailNode = NULL;
	list->length = 0;
}

void* dequeueFromList(List *list) {
	if(!list->headNode)
		return NULL;
		
	Node *tempNode = list->headNode;
	void *ret = tempNode->data;
	
	if(tempNode->nextNode)
			list->headNode = tempNode->nextNode;
	else {
		list->headNode = NULL;
		list->tailNode = NULL;
	}
	list->length--;
	free(tempNode);
	return ret;
}

void appendToList(List *list, void *data) {
	Node *newNode = (Node*) malloc(sizeof(Node));
	newNode->data = data;
	newNode->nextNode = NULL;
	if(list->tailNode) {
		newNode->prevNode = list->tailNode;
		list->tailNode->nextNode = newNode;
		list->tailNode = newNode;
	}
	else {
		newNode->prevNode = NULL;
		list->headNode = newNode;
		list->tailNode = newNode;
	}
	list->length++;
}

int deleteFromList(List *list, Node *node, void (*cleanup_func)(void*)) {
	if(!node)
		return -1;
		
	if(node->prevNode) {
		node->prevNode->nextNode = node->nextNode;
		if(node->nextNode)
			node->nextNode->prevNode = node->prevNode;
		else
			list->tailNode = node->prevNode;
	}
	else if(node->nextNode) {
		node->nextNode->prevNode = NULL;
		list->headNode = node->nextNode;
	}
	else {
		list->headNode = NULL;
		list->tailNode = NULL;
	}
	
	if(cleanup_func)
		cleanup_func(node->data);
		
	free(node);
	list->length--;
	return 0;
}

Node* findInList(List *list, void* data, int (*cmp_func)(void*,void*)) {
	Node *currNode = list->headNode;
	while(currNode) {
		if(cmp_func(data, currNode->data))
			return currNode;
		currNode = currNode->nextNode;
	}
	return NULL;
}  

int findAndDeleteFromList(List *list, void* data, int (*cmp_func)(void*,void*), void (*cleanup_func)(void*)) {
	return deleteFromList(list, findInList(list, data, cmp_func), cleanup_func);
}

void printList(List *list, void (*print_func)(void*)) {
	Node *currNode = list->headNode;
	while(currNode) {
		if(currNode->data)
			print_func(currNode->data);
		currNode = currNode->nextNode;
	}
}
