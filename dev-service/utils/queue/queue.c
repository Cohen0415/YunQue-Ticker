#include "queue.h"
#include <stdio.h>

queue_t* queue_create(void)
{
    queue_t *q = (queue_t*)malloc(sizeof(queue_t));
    if(!q) 
        return NULL;

    q->head = q->tail = NULL;
    q->size = 0;
    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->cond, NULL);
    return q;
}

void queue_destroy(queue_t *q)
{
    if(!q) 
        return;

    pthread_mutex_lock(&q->mutex);
    queue_node_t *node = q->head;
    while(node) 
    {
        queue_node_t *tmp = node;
        node = node->next;
        free(tmp);
    }
    q->head = q->tail = NULL;
    q->size = 0;
    pthread_mutex_unlock(&q->mutex);

    pthread_mutex_destroy(&q->mutex);
    pthread_cond_destroy(&q->cond);
    free(q);
}

void queue_push(queue_t *q, void *data)
{
    if(!q) 
        return;

    queue_node_t *node = (queue_node_t*)malloc(sizeof(queue_node_t));
    if(!node) 
        return;

    node->data = data;
    node->next = NULL;

    pthread_mutex_lock(&q->mutex);
    if(q->tail) 
    {
        q->tail->next = node;
        q->tail = node;
    } else 
    {
        q->head = q->tail = node;
    }
    q->size++;
    pthread_cond_signal(&q->cond); // 通知可能阻塞的 pop
    pthread_mutex_unlock(&q->mutex);
}

void* queue_pop(queue_t *q)
{
    if(!q) 
        return NULL;

    pthread_mutex_lock(&q->mutex);
    while(q->size == 0) 
    {
        pthread_cond_wait(&q->cond, &q->mutex);
    }
    queue_node_t *node = q->head;
    q->head = node->next;
    if(q->head == NULL) 
        q->tail = NULL;
    q->size--;
    pthread_mutex_unlock(&q->mutex);

    void *data = node->data;
    free(node);
    return data;
}

void* queue_try_pop(queue_t *q)
{
    if(!q) 
        return NULL;

    pthread_mutex_lock(&q->mutex);
    if(q->size == 0) 
    {
        pthread_mutex_unlock(&q->mutex);
        return NULL;
    }
    queue_node_t *node = q->head;
    q->head = node->next;
    if(q->head == NULL) 
        q->tail = NULL;
    q->size--;
    pthread_mutex_unlock(&q->mutex);

    void *data = node->data;
    free(node);
    return data;
}

size_t queue_size(queue_t *q)
{
    if(!q) 
        return 0;

    pthread_mutex_lock(&q->mutex);
    size_t size = q->size;
    pthread_mutex_unlock(&q->mutex);
    return size;
}