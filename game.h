#ifndef _GAME_H
#define _GAME_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "network.h"

/**
 * ConnectionData *connection   - Connection to server
 * char *name                   - Display Name
 * char *role                   - X or O
 * 
 * pthread_mutex_t lock
 * pthread_cond_t read_ready
 * pthread_cond_t write_ready
*/
typedef struct Player{
    ConnectionData *connection;
    char *name;
    char *role;

    pthread_mutex_t lock;
    pthread_cond_t read_ready;
    pthread_cond_t write_ready;
} Player;

/**
 * Player player1
 * Player player2
 * int turn
 * int currentMove
 * char *board
 * 
 * pthread_mutex_t lock
 * pthread_cond_t read_ready
 * pthread_cond_t write_ready
*/
typedef struct Game{
    Player player1;
    Player player2;
    int turn;
    int currentMove;
    char *board;

    pthread_mutex_t lock;
    pthread_cond_t read_ready;
    pthread_cond_t write_ready;

} Game;

/**
 * Initialize a player
*/
int init_player(Player *player);

/**
 * Destroy a player
*/
int destroy_player(Player *player);

/**
 * Initialize a game
*/
int init_game(Game *game);

/**
 * Destroy a game
*/
int destroy_game(Game *game);

#endif