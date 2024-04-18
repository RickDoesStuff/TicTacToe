#include "game.h"

int game_init(Game *game)
{
    for (int x = 0; x < 3; x++){
        for (int y = 0; y < 3 ; y++) {
            game->board[x][y] = ".";
        }
    }
    init_player(&game->player1);
    init_player(&game->player2);

    game->currentMove = 0;
    game->turn = 0;
    
    pthread_mutex_init(&game->lock, NULL);
    pthread_cond_init(&game->read_ready, NULL);
    pthread_cond_init(&game->write_ready, NULL);

    return 0;
}

int game_destroy(Game *game)
{
    destroy_player(&game->player1);
    destroy_player(&game->player2);

    pthread_mutex_destroy(&game->lock);
    pthread_cond_destroy(&game->read_ready);
    pthread_cond_destroy(&game->write_ready);

    return 0;
}

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
        free(player->role);
        perror("player role malloc error:");
        return -1;
    }
    player->role = NULL;
}