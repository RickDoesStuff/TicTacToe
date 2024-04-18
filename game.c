#include "game.h"

int game_init(Game *game)
{
    for (int x = 0; x < 3; x++){
        for (int y = 0; y < 3 ; y++) {
            game->board[x][y] = 0;
        }
    }
    game->connection1 = NULL;
    game->connection2 = NULL;
    game->currentMove = 0;
    game->turn = 0;
    
    pthread_mutex_init(&game->lock, NULL);
    pthread_cond_init(&game->read_ready, NULL);
    pthread_cond_init(&game->write_ready, NULL);

    return 0;
}

int game_destroy(Game *game)
{
    pthread_mutex_destroy(&game->lock);
    pthread_cond_destroy(&game->read_ready);
    pthread_cond_destroy(&game->write_ready);

    return 0;
}