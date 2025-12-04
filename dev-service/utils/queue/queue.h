#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct queue_node {
    void *data;
    struct queue_node *next;
} queue_node_t;

typedef struct queue {
    queue_node_t *head;
    queue_node_t *tail;
    size_t size;

    pthread_mutex_t mutex;
    pthread_cond_t cond;
} queue_t;

queue_t* queue_create(void);
void queue_destroy(queue_t *q);
void queue_push(queue_t *q, void *data);
void* queue_pop(queue_t *q);
void* queue_try_pop(queue_t *q);
size_t queue_size(queue_t *q);

#endif // QUEUE_H