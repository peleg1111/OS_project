#include "Stack.h"
#include "stdio.h"
#include "memory.h"



Stack* create_stack(){
    return _create_stack(0);
}

Stack *_create_stack(int is_kernel){

    void *(*alloc)(unsigned int);
    if (is_kernel == 1)
    {
        alloc = kmalloc;
    }
    else
    {
        alloc = malloc;
    }

    Stack *s = alloc(sizeof(Stack));

    if (s == null)
        return null;

    s->head = null;
    s->Len = 0;

    return s;
}

void* head_s(Stack* s){
    if(s != null && s->head != null){
        return s->head->val;
    }
    return null;
}


void insert_s(Stack* s, void* val){
    if(s == null) return;

    List_s* lst = malloc(sizeof(List_s));
    if(lst == null) return;
    lst->val = val;

    lst->next = s->head;
    s->head = lst;
    s->Len += 1;
    
}

void* pop_s(Stack* s){
    if(s == null) return null;
    if(s->head == null) return null;
    List_s* lst = s->head;
    s->head = s->head->next;
    void* val = lst->val;
    free(lst);

    return val;
}

void free_s(Stack* s){
    List_s* lst;
    while (s->head!= null)
    {
        lst = s->head;
        s->head = s->head->next;
        free(lst);
    }
    free(s);
    
}