//header for my circular linked list
#ifndef LIST_H_
#define LIST_H_

typedef struct _list List;

struct _list { //nodes
	char *value;
	List *next;
	List *previous;
};

List * createCircularLinkedList(const char * const strings[]);
void freeCircularLinkedList(List **list);

#endif

