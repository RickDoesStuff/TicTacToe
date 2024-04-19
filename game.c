#include "game.h"

/**
 * Initialize a game structure
*/
int game_init(Game *game)
{
    game->board = malloc(sizeof(char)*10);
    for (int i = 0; i < 9; i++){
        game->board[i] = '.';
    }
    game->board[9] = '\0';

    init_player(&game->player1);
    init_player(&game->player2);

    game->currentMove = 0;
    game->turn = 0;
    
    pthread_mutex_init(&game->lock, NULL);
    pthread_cond_init(&game->read_ready, NULL);
    pthread_cond_init(&game->write_ready, NULL);

    return 0;
}

/**
 * Destroy a game structure
*/
void game_destroy(Game *game)
{
    free(game->board);

    destroy_player(&game->player1);
    destroy_player(&game->player2);

    pthread_mutex_destroy(&game->lock);
    pthread_cond_destroy(&game->read_ready);
    pthread_cond_destroy(&game->write_ready);
}

/**
 * initialize a player
 * return -1 on error
 * return 1 on success
*/
int init_player(Player *player) {
    player->connection = malloc(sizeof(ConnectionData));
    if (player->connection == NULL) {
        perror("player connection malloc error:");
        return -1;
    }
    player->connection = NULL;

    player->name = malloc(sizeof(char*));
    if (player->name == NULL) {
        free(player->connection);
        perror("player name malloc error:");
        return -1;
    }
    player->name = NULL;

    player->role = malloc(sizeof(char*));
    if (player->role == NULL) {
        free(player->connection);
        free(player->name);
        perror("player role malloc error:");
        return -1;
    }
    player->role = NULL;
    return 1;
}

/**
 * destroy a player structure
*/
void destroy_player(Player *player) {
    free(player->connection);
    free(player->name);
    free(player->role);

    
    pthread_mutex_destroy(&player->lock);
    pthread_cond_destroy(&player->read_ready);
    pthread_cond_destroy(&player->write_ready);
}

int init_games(Games **games, int amt) {
    for (int i = 0 ; i < amt ; i++){
        if(init_game(games[i])){
            return -1;
        }
    }
    return 1;
}