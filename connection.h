#ifndef _CONNECTION_H
#define _CONNECTION_H

#include <sys/socket.h>

// data to be sent to worker threads
typedef struct ConnectionData {
	struct sockaddr_storage addr;
	socklen_t addr_len;
	int fd;
} ConnectionData;


#endif