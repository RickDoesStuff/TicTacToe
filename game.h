#ifndef _GAME_H
#define _GAME_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

typedef struct Game{
    void *connection1;
    void *connection2;
    int turn;
    int currentMove;
    int board[3][3];

    pthread_mutex_t lock;
    pthread_cond_t read_ready;
    pthread_cond_t write_ready;

} Game;

#endif