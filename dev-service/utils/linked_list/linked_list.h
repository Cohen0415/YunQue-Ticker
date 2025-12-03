#ifndef LINKED_LIST_H
#define LINKED_LIST_H

typedef struct ll_node {
    void *data;
    struct ll_node *next;
} ll_node_t;

typedef struct ll_list_t {
    ll_node_t *head;
    int size;
} ll_list_t;

ll_list_t *ll_create(void);
void ll_free(ll_list_t *list);
int ll_add(ll_list_t *list, void *data);
void *ll_get(ll_list_t *list, int index);

#endif // LINKED_LIST_H