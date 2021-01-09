#include <stdlib.h>

typedef struct Node_t {
	struct Node_t *nextNode;
	struct Node_t *prevNode;
	void *data;
} Node;


typedef struct List_t {
	Node *headNode;
	Node *tailNode;
	int length;	
} List;

void createList(List *list);

void deleteList(List *list, void (*cleanup_func)(void*));

void appendToList(List *list, void *data);

void* dequeueFromList(List *list);

int deleteFromList(List *list, Node *node, void (*cleanup_func)(void*));

Node* findInList(List *list, void* data, int (*cmp_func)(void*,void*));

int findAndDeleteFromList(List *list, void* data, int (*cmp_func)(void*,void*), void (*cleanup_func)(void*));

void printList(List *list, void (*print_func)(void*));

