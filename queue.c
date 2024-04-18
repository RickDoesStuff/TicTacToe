#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "queue.h"

int q_init(queue_t *Q, unsigned capacity)
{
    Q->size = capacity;
    Q->length = 0;
    Q->first = 0;
    Q->closed = 0;
    Q->data = (ConnectionData **) malloc(capacity * sizeof(ConnectionData *));
    if (Q->data == NULL) {
        perror("data malloc error:");
        return -1;
    }
    pthread_mutex_init(&Q->lock, NULL);
    pthread_cond_init(&Q->read_ready, NULL);
    pthread_cond_init(&Q->write_ready, NULL);

    return 0;
}

int q_destroy(queue_t *Q)
{
    free(Q->data);
    pthread_mutex_destroy(&Q->lock);
    pthread_cond_destroy(&Q->read_ready);
    pthread_cond_destroy(&Q->write_ready);

    return 0;
}

int q_enqueue(queue_t *Q, ConnectionData *item)
{
    pthread_mutex_lock(&Q->lock);

    if (Q->closed) {
        pthread_mutex_unlock(&Q->lock);
        return -1;
    }

    while (Q->length == Q->size) {
        pthread_cond_wait(&Q->write_ready, &Q->lock);
    }

    int last = (Q->first + Q->length) % Q->size;
    Q->data[last] = item;
    Q->length++;

    pthread_cond_signal(&Q->read_ready);

    pthread_mutex_unlock(&Q->lock);

    return 0;
}

int q_dequeue(queue_t *Q, ConnectionData **dst)
{

    while (Q->length == 0) {
        if (Q->closed) {
            pthread_mutex_unlock(&Q->lock);
            return -1;
        }
        pthread_cond_wait(&Q->read_ready, &Q->lock);
    }

    *dst = Q->data[Q->first];
    Q->length--;
    Q->first = (Q->first + 1) % Q->size;

    return 0;
}

int q_close(queue_t *Q)
{
    pthread_mutex_lock(&Q->lock);

    Q->closed = 1;
    pthread_cond_broadcast(&Q->read_ready); // Wake up all threads waiting to dequeue.

    pthread_mutex_unlock(&Q->lock);

    return 0;
}
