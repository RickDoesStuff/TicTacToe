// NOTE: must use option -pthread when compiling!
#define _POSIX_C_SOURCE 200809L
#include "ttts.h"
#include "network.h"
#include "queue.h"

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
    	NI_NUMERICSERV
    );
    if (error) {
        fprintf(stderr, "getnameinfo: %s\n", gai_strerror(error));
        strcpy(host, "??");
        strcpy(port, "??");
    }

    printf("Connection from %s:%s\n", host, port);

    while (active && (bytes = read(con->fd, buf, BUFSIZE)) > 0) {
        buf[bytes] = '\0';
        printf("[%s:%s] read %d bytes |%s|\n", host, port, bytes, buf);
    }

	if (bytes == 0) {
		printf("[%s:%s] got EOF\n", host, port);
	} else if (bytes == -1) {
		printf("[%s:%s] terminating: %s\n", host, port, strerror(errno));
	} else {
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
    pthread_t tid;
    char *service = argc == 2 ? argv[1] : "15000";
    install_handlers(&mask);

    // queue_t queue;
    // if (q_init(&queue, 10) == -1) {
    //     return -1;
    // }

    int listener = open_listener(service, QUEUE_SIZE);
    if (listener < 0) exit(EXIT_FAILURE);
    
    printf("Listening for incoming connections on %s\n", service);

    while (active) {
        con = (ConnectionData *)malloc(sizeof(ConnectionData));
        con->addr_len = sizeof(struct sockaddr_storage);

        con->fd = accept(listener, (struct sockaddr *)&con->addr, &con->addr_len);
        
        if (con->fd < 0) {
            perror("accept");
            free(con);
            continue;
        }
            // Signal handling should be done here before the threads start
            int error = pthread_sigmask(SIG_BLOCK, &mask, NULL);
            if (error != 0) {
                fprintf(stderr, "sigmask: %s\n", strerror(error));
                exit(EXIT_FAILURE);
            }

            printf("before creating\n");
            ConnectionData *con;
            error = pthread_create(&tid, NULL, read_data, con);        
            if (error != 0) {
                fprintf(stderr, "sigmask: %s\n", strerror(error));
                exit(EXIT_FAILURE);
            }
            pthread_detach(tid);

            printf("**thread made and detached\n");

        //printf("Waiting in the queue for a second connection\n");

        //q_enqueue(&queue, con);

        //printf("en queued\n");

        // Handle the connections when there are exactly two
        
        // printf("queue locked\n");
        // pthread_mutex_lock(&queue.lock);
        // if (queue.length >= MAX_CONNECTIONS) {
        //     printf("**length >= max connections\n");


        //     ConnectionData *con1, *con2;
        //     printf("dequeuing 1\n");
        //     q_dequeue(&queue, &con1);
        //     printf("dequeuing 2\n");
        //     q_dequeue(&queue, &con2);
        //     printf("all dequeued\n");

        //     // Signal handling should be done here before the threads start
        //     int error = pthread_sigmask(SIG_BLOCK, &mask, NULL);
        //     if (error != 0) {
        //         fprintf(stderr, "sigmask: %s\n", strerror(error));
        //         exit(EXIT_FAILURE);
        //     }

        //     printf("before creating\n");
        //     error = pthread_create(&tid, NULL, read_data, con1);        
        //     if (error != 0) {
        //         fprintf(stderr, "sigmask: %s\n", strerror(error));
        //         exit(EXIT_FAILURE);
        //     }
        //     pthread_detach(tid);
        //     pthread_create(&tid, NULL, read_data, con2);        
        //     if (error != 0) {
        // 	    fprintf(stderr, "sigmask: %s\n", strerror(error));
        // 	    exit(EXIT_FAILURE);
        //     }
        //     pthread_detach(tid);
        //     printf("**threads made and detached\n");
        // }
        // pthread_mutex_unlock(&queue.lock);
        // printf("queue unlocked\n");
    }

    puts("Shutting down");
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
