#ifndef LIST_H
#define LIST_H

typedef struct Node
{
    void *data;
    struct Node *next;
} Node;

typedef struct List
{
    int len;
    Node *head;
    Node *tail;
} List;


List *create_list();
List *_create_list(int is_kernel);

void append_list(List *lst, void *data);
void k_append_list(List *lst, void *data);
void *pop_list(List *lst, int index);
void free_list(List *lst);









#endif