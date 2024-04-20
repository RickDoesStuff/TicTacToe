#include "game.h"



/**
 * send a Player a message
 * return 1 on success
*/
int sendClientMsg(Player *player, char *msg) {
    if (player == NULL) {
        printf("send client message : player null\n");
        return -1;
    }
    if (msg == NULL) {
        printf("send client message : msg null\n");
        return -1;
    }
    printf("called sending msg\n");
    //pthread_mutex_lock(&player->lock);
    //printf("in lock\n");

    if (player == NULL) {printf("player null");}
    if (player->connection == NULL) {printf("player connection null");}
    if (player->connection->fd == 0) {printf("player fd is 0");}

    printf("writing |%s| to %i\n", msg, player->connection->fd);
    write(player->connection->fd, msg, strlen(msg));
    printf("wrote\n");

    //pthread_mutex_unlock(&player->lock);

    return 1;
}

/**
 * Start a game
*/
void *startGame(void *arg)
{
    Game *game = (Game*)(arg);
    if (game == NULL) {printf("Game null"); return NULL; }
    int isRunning = 1;
    pthread_mutex_lock(&game->lock);
    while(isRunning) {
        printf("is running\n");
        // simulate player2 sending play
        sendClientMsg(game->player1,"PLAY");
        // sendClientMsg(&game->player2,"WAIT");
        

        // // simulate player1 sending play
        // sendClientMsg(&game->player2,"PLAY");
        // sendClientMsg(&game->player1,"WAIT");
    
        pthread_mutex_unlock(&game->lock);
    }
    return NULL;
}


/**
 * Initialize a game structure
 * return 1 on sucess 
 * return -1 on failure
*/
int init_game(Game *game) {
    if (game == NULL) {
        fprintf(stderr, "Game structure is NULL\n");
        return -1;
    }

    // Initialize players
    game->player1 = malloc(sizeof(Player));
    game->player2 = malloc(sizeof(Player));
    if (game->player1 == NULL || game->player2 == NULL) {
        perror("Failed to allocate memory for players");
        free(game->player1);  // Safely free any allocated memory
        free(game->player2);
        return -1;
    }

    if (init_player(game->player1) != 1 || init_player(game->player2) != 1) {
        destroy_player(game->player1);  // Cleanup initialized player
        destroy_player(game->player2);
        return -1;
    }

    // Setup the game board
    game->board = malloc(9 * sizeof(char));
    if (game->board == NULL) {
        perror("Failed to allocate memory for game board");
        destroy_player(game->player1);
        destroy_player(game->player2);
        return -1;
    }
    memset(game->board, '.', 9);  // Initialize the board with spaces

    // Initialize synchronization primitives
    if (pthread_mutex_init(&game->lock, NULL) != 0 ||
        pthread_cond_init(&game->read_ready, NULL) != 0 ||
        pthread_cond_init(&game->write_ready, NULL) != 0) {
        perror("Failed to initialize synchronization primitives");

        free(game->board);
        destroy_player(game->player1);
        destroy_player(game->player2);

        pthread_mutex_destroy(&game->lock);
        pthread_cond_destroy(&game->read_ready);
        pthread_cond_destroy(&game->write_ready);

        return -1;
    }

    return 1;
}



/**
 * Destroy a game structure
*/
void destroy_game(Game *game) {
    if (game != NULL) {
        destroy_player(game->player1);
        destroy_player(game->player2);
        free(game->board);

        pthread_mutex_destroy(&game->lock);
        pthread_cond_destroy(&game->read_ready);
        pthread_cond_destroy(&game->write_ready);

        free(game); // Finally free the game structure itself
    }
}

void destroy_games(Games *games) {
    for (int i = 0; games->size; i++) {
        destroy_game(&games->games[i]);
    }
}

int init_player(Player *player) {
    if (player == NULL) {
        fprintf(stderr, "Player is NULL\n");
        return -1;
    }

    player->connection = malloc(sizeof(ConnectionData));
    if (player->connection == NULL) {
        perror("Player connection malloc error:");
        return -1;
    }

    player->name = malloc(50 * sizeof(char));  // Assuming 50 chars for name
    if (player->name == NULL) {
        free(player->connection);
        perror("Player name malloc error:");
        return -1;
    }
    player->name[0] = '\0';  // Initialize the string to be empty

    player->role = malloc(50 * sizeof(char));  // Assuming 50 chars for role
    if (player->role == NULL) {
        free(player->connection);
        free(player->name);
        perror("Player role malloc error:");
        return -1;
    }
    player->role[0] = '\0';  // Initialize the string to be empty

    // Initialize mutex and condition variables
    if (pthread_mutex_init(&player->lock, NULL) != 0) {
        perror("Mutex init failed");
        free(player->connection);
        free(player->name);
        free(player->role);
        return -1;
    }
    if (pthread_cond_init(&player->read_ready, NULL) != 0) {
        perror("Read condition variable init failed");
        pthread_mutex_destroy(&player->lock);  // Clean up previously initialized mutex
        free(player->connection);
        free(player->name);
        free(player->role);
        return -1;
    }
    if (pthread_cond_init(&player->write_ready, NULL) != 0) {
        perror("Write condition variable init failed");
        pthread_mutex_destroy(&player->lock);
        pthread_cond_destroy(&player->read_ready);
        free(player->connection);
        free(player->name);
        free(player->role);
        return -1;
    }

    return 1;
}

/**
 * destroy a player structure
*/
void destroy_player(Player *player) {
    if (player != NULL) {
        free(player->connection);
        free(player->name);
        free(player->role);

        pthread_mutex_destroy(&player->lock);
        pthread_cond_destroy(&player->read_ready);
        pthread_cond_destroy(&player->write_ready);

        free(player);  // Finally, free the player itself
    }
}

int init_games(Games *games, int amt) {
    games->games = malloc (amt * sizeof(Game));
    if(games->games == NULL) {
        printf("Failed to malloc Games\n");
        return -1;
    }
    for (int i = 0 ; i < amt ; i++){
        if(init_game(&games->games[i]) == -1){
            printf("malloc error\n");
            return -1;
        }
    }
    printf("All games initialized\n");
    return 1;
}