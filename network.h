#ifndef _NETWORK_H
#define _NETWORK_H


#include <sys/socket.h>

// data to be sent to worker threads
typedef struct ConnectionData {
	struct sockaddr_storage addr;
	socklen_t addr_len;
	int fd;
} ConnectionData;

int connect_inet(char *host, char *service);
int open_listener(char *service, int queue_size);
#endif