#ifndef QUEUE_H
#define QUEUE_H


typedef struct List_s
{
    void *val;
    struct List_s *next;
} List_s;

typedef struct Queue_s
{
    unsigned int Len;
    List_s *list;
    List_s *head_node;
    List_s *end_node;

} Queue_s;


Queue_s *create_queue();
Queue_s *_create_queue(int is_kernel);

void insert_q(Queue_s* q, void *val);
void *pop_q(Queue_s* q);
void *head_q(Queue_s* q);
void free_q(Queue_s* q);
int remove_by_val_q(Queue_s *q, void *val);

#endif