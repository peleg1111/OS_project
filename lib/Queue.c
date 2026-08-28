#include "Queue.h"
#include "stdio.h"
#include "memory.h"

void *malloc(unsigned int size);

Queue_s *create_queue()
{
    return _create_queue(0);
}

Queue_s* _create_queue(int is_kernel){
    void* ( *alloc)(unsigned int);
    if(is_kernel == 1){
        alloc = kmalloc;
    }
    else{
        alloc = malloc;
    }

    Queue_s *q = alloc(sizeof(Queue_s));
    if (!q)
        return null;

    q->Len = 0;
    q->list = null;
    q->head_node = null;
    q->end_node = null;

    return q;
}

void insert_q(Queue_s *q, void *val)
{
    if (!q) return;

    List_s *lst = malloc(sizeof(List_s));
    if (!lst) return;

    lst->val = val;
    lst->next = null;

    if (q->Len == 0)
    {
        q->list = lst;
        q->head_node = lst;
        q->end_node = lst;
    }
    else
    {
        lst->next = q->end_node;
        q->end_node = lst;
    }
    q->Len++;
}

void *pop_q(Queue_s *q)
{
    if (!q || q->Len == 0 || !q->head_node)
    {
        return null;
    }

    void *val = q->head_node->val;
    List_s *to_free = q->head_node;

    if (q->Len == 1)
    {
        q->head_node = null;
        q->end_node = null;
        q->list = null;
    }
    else
    {
        List_s *current = q->end_node;
        while (current != null && current->next != q->head_node)
        {
            current = current->next;
        }

        if (current)
        {
            current->next = null;
            q->head_node = current;
        }
    }

    free(to_free);
    q->Len--;

    return val;
}

void *head_q(Queue_s *q)
{
    if (!q || !q->head_node) return null;
    return q->head_node->val;
}

void free_q(Queue_s * q)
{
    List_s* current = q->end_node;
    List_s* temp;
    while (current != null)
    {
        temp = current;
        current = current->next;
        free(temp);
    }
    free(q);
}

int remove_by_val_q(Queue_s *q, void *val)
{
    if (!q || q->Len == 0 || !q->end_node)
        return 0;

    List_s *current = q->end_node;
    List_s *prev = null;

    while (current != null)
    {
        if (current->val == val)
        {
            if (prev == null)
            {
                q->end_node = current->next;
            }
            else
            {
                prev->next = current->next;
            }

            if (current == q->head_node)
            {
                q->head_node = prev;
            }

            free(current);
            q->Len--;

            if (q->Len == 0)
            {
                q->head_node = null;
                q->end_node = null;
                q->list = null;
            }
            else
            {
                q->list = q->end_node;
            }

            return 1;
        }

        prev = current;
        current = current->next;
    }

    return 0;
}