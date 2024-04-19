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

typedef struct Games{
    Game *games;
    int active;
    int size;

    pthread_mutex_t lock;
    pthread_cond_t read_ready;
    pthread_cond_t write_ready;
} Games;

/**
 * Initialize a player
 * return 1 on success
 * return -1 on failure
*/
int init_player(Player *player);

/**
 * Destroy a player
*/
void destroy_player(Player *player);

/**
 * initialize a game
*/
int init_game(Game **games);

/**
 * Destroy a game
*/
void destroy_game(Game *game);
/**
 * Initialize a game
 * Game**
 * amt
 * return 1 on success
 * return -1 on failure
*/
int init_games(Games **games, int amt);

/**
 * Destroy a game
*/
void destroy_games(Games **games);

#endif