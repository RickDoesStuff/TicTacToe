#ifndef _QUEUE_H
#define _QUEUE_H

#include "network.h"

typedef struct {
    unsigned size;    // Capacity of the queue
    unsigned length;  // Current number of items in the queue
    unsigned first;   // Index of the oldest queue item
    int closed;       // 1 when no further enqueues are permitted
    ConnectionData **data; // Array storing the items
    pthread_mutex_t lock;
    pthread_cond_t read_ready;
    pthread_cond_t write_ready;
} queue_t;

int q_init(queue_t *, unsigned);
int q_destroy(queue_t *);

int q_enqueue(queue_t *, ConnectionData *);
int q_dequeue(queue_t *, ConnectionData **);

int q_close(queue_t *);

#endif
