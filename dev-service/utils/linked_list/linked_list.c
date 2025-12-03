#include <stdlib.h>
#include "linked_list.h"

ll_list_t *ll_create(void) 
{
    ll_list_t *list = (ll_list_t *)malloc(sizeof(ll_list_t));
    if (list == NULL) 
    {
        return NULL;
    }

    list->head = NULL;
    list->size = 0;
    return list;
}

void ll_free(ll_list_t *list) 
{
    if (list == NULL) 
    {
        return;
    }

    ll_node_t *current = list->head;
    ll_node_t *next;

    while (current != NULL) 
    {
        next = current->next;
        free(current);
        current = next;
    }

    free(list);
}

int ll_add(ll_list_t *list, void *data) 
{
    if (list == NULL) 
    {
        return -1; // Return error if list is NULL
    }

    ll_node_t *new_ll_node_t = (ll_node_t *)malloc(sizeof(ll_node_t));
    if (new_ll_node_t == NULL) 
    {
        return -1; // Return error if memory allocation fails
    }

    new_ll_node_t->data = data;
    new_ll_node_t->next = NULL;

    if (list->head == NULL) 
    {
        list->head = new_ll_node_t;
    } 
    else 
    {
        ll_node_t *current = list->head;
        while (current->next != NULL) 
        {
            current = current->next;
        }
        current->next = new_ll_node_t;
    }

    list->size++;
    return 0; // Return 0 on success
}

void *ll_get(ll_list_t *list, int index) 
{
    if (list == NULL || index < 0 || index >= list->size) 
    {
        return NULL; // Return NULL if list is NULL or index is out of bounds
    }

    ll_node_t *current = list->head;
    for (int i = 0; i < index; i++) 
    {
        current = current->next;
    }

    return current->data;
}