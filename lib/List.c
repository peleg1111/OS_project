#include "List.h"
#include "stdio.h"
#include "memory.h"

List *create_list()
{
    return _create_list(0);
}

List *_create_list(int is_kernel)
{
    void *(*alloc)(unsigned int);
    if (is_kernel == 1)
    {
        alloc = kmalloc;
    }
    else
    {
        alloc = malloc;
    }

    List *lst = alloc(sizeof(List));
    if (!lst) return null;

    lst->len = 0;
    lst->head = null;
    lst->tail = null;

    return lst;
}

void append_list(List *lst, void *data)
{
    if (!lst) return;

    Node *node = malloc(sizeof(Node));
    if (!node) return;

    node->data = data;
    node->next = null;

    if (lst->len == 0)
    {
        lst->head = node;
        lst->tail = node;
    }
    else
    {
        lst->tail->next = node;
        lst->tail = node;
    }
    lst->len++;
}

void k_append_list(List *lst, void *data)
{
    if (!lst) return;

    Node *node = kmalloc(sizeof(Node));
    if (!node) return;

    node->data = data;
    node->next = null;

    if (lst->len == 0)
    {
        lst->head = node;
        lst->tail = node;
    }
    else
    {
        lst->tail->next = node;
        lst->tail = node;
    }
    lst->len++;
}

void *pop_list(List *lst, int index)
{
    if (!lst || index < 0 || index >= lst->len || !lst->head)
    {
        return null;
    }

    Node *current = lst->head;
    Node *prev = null;
    int i = 0;

    while (current != null && i < index)
    {
        prev = current;
        current = current->next;
        i++;
    }

    if (!current) return null;

    void *data = current->data;

    if (prev == null)
    {
        lst->head = current->next;
    }
    else
    {
        prev->next = current->next;
    }

    if (current == lst->tail)
    {
        lst->tail = prev;
    }

    lst->len--;
    if (lst->len == 0)
    {
        lst->head = null;
        lst->tail = null;
    }

    free(current);
    return data;
}

void free_list(List *lst)
{
    if (!lst) return;

    Node *current = lst->head;
    Node *temp;
    while (current != null)
    {
        temp = current;
        current = current->next;
        free(temp);
    }
    free(lst);
}