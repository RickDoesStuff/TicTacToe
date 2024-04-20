// NOTE: must use option -pthread when compiling!
#define _POSIX_C_SOURCE 200809L
#include "ttts.h"
#include "network.h"
#include "queue.h"
#include "game.h"

#define QUEUE_SIZE 8

#define QUEUE_SIZE 8
#define MAX_CONNECTIONS 2

volatile int active = 1;

void handler(int signum)
{
    active = 0;
}

// set up signal handlers for primary thread
// return a mask blocking those signals for worker threads
// FIXME should check whether any of these actually succeeded
void install_handlers(sigset_t *mask)
{
    struct sigaction act;
    act.sa_handler = handler;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);

    sigaction(SIGINT, &act, NULL);
    sigaction(SIGTERM, &act, NULL);

    sigemptyset(mask);
    sigaddset(mask, SIGINT);
    sigaddset(mask, SIGTERM);
}

Games games;
Game *game;
queue_t queue;

#define BUFSIZE 256
#define HOSTSIZE 100
#define PORTSIZE 10
void *read_data(void *arg)
{

    ConnectionData *con = arg;
    char buf[BUFSIZE + 1], host[HOSTSIZE], port[PORTSIZE];
    int bytes, error;

    error = getnameinfo(
        (struct sockaddr *)&con->addr, con->addr_len,
        host, HOSTSIZE,
        port, PORTSIZE,
        NI_NUMERICSERV);
    if (error)
    {
        fprintf(stderr, "getnameinfo: %s\n", gai_strerror(error));
        strcpy(host, "??");
        strcpy(port, "??");
    }

    printf("Connection from %s:%s:  fd:%i\n", host, port, con->fd);

    while (active && (bytes = read(con->fd, buf, BUFSIZE)) > 0)
    {
        buf[bytes] = '\0';
        printf("[%s:%s] read %d bytes |%s|\n", host, port, bytes, buf);
    }

    if (bytes == 0)
    {
        printf("[%s:%s] got EOF\n", host, port);
    }
    else if (bytes == -1)
    {
        printf("[%s:%s] terminating: %s\n", host, port, strerror(errno));
    }
    else
    {
        printf("[%s:%s] terminating\n", host, port);
    }

    close(con->fd);
    free(con);

    return NULL;
}

int main(int argc, char **argv)
{
    sigset_t mask;
    ConnectionData *con;
    int error;
    pthread_t tid;

    q_init(&queue, 8);
    // init_games(&games, 1);
    game = (Game *)malloc(sizeof(Game));
    init_game(game);

    char *service = argc == 2 ? argv[1] : "15000";

    install_handlers(&mask);

    int listener = open_listener(service, QUEUE_SIZE);
    if (listener < 0)
        exit(EXIT_FAILURE);

    printf("Listening for incoming connections on %s\n", service);

    while (active)
    {
        con = (ConnectionData *)malloc(sizeof(ConnectionData));
        con->addr_len = sizeof(struct sockaddr_storage);

        con->fd = accept(listener,
                         (struct sockaddr *)&con->addr,
                         &con->addr_len);

        if (con->fd < 0)
        {
            perror("accept");
            free(con);
            // TODO check for specific error conditions
            continue;
        }
        // connection sucessful

        // send wait to client
        int bytesWritten;
        if ((bytesWritten = write(con->fd, "WAIT", 4) != 4))
        {
            printf("Error Sending Wait : Bytes Written: %i\n", bytesWritten);
            return -1;
        }
        else
        {
            printf("Wait sent!\n");
        }

        // create a game if there isnt one

        // temporarily disable signals
        // (the worker thread will inherit this mask, ensuring that SIGINT is
        // only delivered to this thread)
        error = pthread_sigmask(SIG_BLOCK, &mask, NULL);
        if (error != 0)
        {
            fprintf(stderr, "sigmask: %s\n", strerror(error));
            exit(EXIT_FAILURE);
        }

        error = pthread_create(&tid, NULL, read_data, con);
        if (error != 0)
        {
            fprintf(stderr, "pthread_create: %s\n", strerror(error));
            close(con->fd);
            free(con);
            continue;
        }

        // automatically clean up child threads once they terminate
        pthread_detach(tid);

        // unblock handled signals
        error = pthread_sigmask(SIG_UNBLOCK, &mask, NULL);
        if (error != 0)
        {
            fprintf(stderr, "sigmask: %s\n", strerror(error));
            exit(EXIT_FAILURE);
        }

        if (q_enqueue(&queue, con) == -1)
        {
            return -1;
        }
        printf("join queue: total in queue: %i\n", queue.length);

        pthread_mutex_lock(&queue.lock);

        // check if we can start a game
        if (queue.length >= 2)
        {
            printf("**queue length >= 2\n");
            // ConnectionData *con1 = queue.data[queue.first];
            // q_dequeue(&queue, &con1);
            // ConnectionData *con2 = queue.data[queue.first];
            // q_dequeue(&queue, &con2);

            game->player1->connection = queue.data[queue.first];
            q_dequeue(&queue, &game->player1->connection);

            game->player2->connection = queue.data[queue.first];
            q_dequeue(&queue, &game->player2->connection);

            // Player player1;
            // init_player(&player1);
            // if (init_player(&player1) == -1) {
            //     return -1;
            // }
            // player1.connection = con1;

            // Player player2;
            // init_player(&player2);
            // if (init_player(&player2) == -1) {
            //     return -1;
            // }
            // player2.connection = con2;

            pthread_mutex_lock(&games.lock);

            // (*games.games[0].player1) = player1;

            // (*games.games[0].player2) = player2;
            // might need to make another game here if there is not enough in Games.size

            // create a thread to run the game on
            // error = pthread_create(&tid, NULL, startGame, &games.games[games.active]);
            printf("creating game...\n");
            error = pthread_create(&tid, NULL, startGame, &game);
            if (error != 0)
            {
                fprintf(stderr, "pthread_create: %s\n", strerror(error));
                continue;
            }
            pthread_detach(tid);
            games.active++;
            printf("game created!\n");
            pthread_mutex_unlock(&games.lock);
        }
        pthread_mutex_unlock(&queue.lock);

        // check amount of current connections
    }

    puts("Shutting down");
    destroy_game(game);
    q_destroy(&queue);
    close(listener);

    // returning from main() (or calling exit()) immediately terminates all
    // remaining threads

    // to allow threads to run to completion, we can terminate the primary thread
    // without calling exit() or returning from main:
    //   pthread_exit(NULL);
    // child threads will terminate once they check the value of active, but
    // there is a risk that read() will block indefinitely, preventing the
    // thread (and process) from terminating

    // to get a timely shut-down of all threads, including those blocked by
    // read(), we will could maintain a global list of all active thread IDs
    // and use pthread_cancel() or pthread_kill() to wake each one

    return EXIT_SUCCESS;
}
