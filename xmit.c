#define _POSIX_C_SOURCE 200809L

#include "xmit.h"

#define BUFLEN 256

int main(int argc, char **argv)
{
    int sock, bytes, sent;
    char buf[BUFLEN];

    signal(SIGPIPE, SIG_IGN);

    if (argc != 3)
    {
        printf("Specify host and service\n");
        exit(EXIT_FAILURE);
    }

    sock = connect_inet(argv[1], argv[2]);
    if (sock < 0) {
        exit(EXIT_FAILURE);
    }

    printf("Connected to %s:%s\n", argv[1], argv[2]);

// PLAY name
// WAIT
// BEGN role name
// MOVE role position
// MOVD role position board
// INVL reason
// RSGN
// DRAW message
// OVER outcome reason
    char *protocol = malloc(sizeof(char*));
    protocol = "WAIT\0";

    // keep playing until protocol is over
    while (!(strcmp(protocol,"OVER") == 0)) {
        printf("current protocol: %s\n", protocol);

        // if protocol is wait, read what server is sending
        if (strcmp(protocol,"WAIT") == 0) {
            printf("wait protocol read, waiting for server\n");
            bytes = read(sock, buf, BUFLEN);
            printf("%d bytes read\n", bytes);
            printf("|%s|\n", buf);

            protocol = "not wait\0";

        } else {
            // if protocol isnt wait, read what I am typing
            printf("protocol isnt wait, waiting for my input\n");
            bytes = read(STDIN_FILENO, buf, BUFLEN);
            printf("%d bytes read\n", bytes);
            sent = write(sock, buf, bytes);        
            printf("%d bytes sent\n", sent);
            if (sent == -1)
            {
                perror("write");
                break;
            }
            else if (sent < bytes)
            {
                printf("Only %d bytes sent\n", sent);
            }
            protocol = "WAIT\0";
        }
    }

    close(sock);

    return EXIT_SUCCESS;
}
