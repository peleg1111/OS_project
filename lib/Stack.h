#ifndef STACK_H
#define STACK_H

typedef struct List_s
{
    void *val;
    struct List_s *next;
} List_s;

typedef struct Stack_s{
    List_s* head;
    unsigned int Len;
}Stack;

Stack *create_stack();
Stack *_create_stack(int is_kernel);

void *head_s(Stack *s);
void insert_s(Stack *s, void *val);
void *pop_s(Stack *s);
void free_s(Stack *s);

#endif