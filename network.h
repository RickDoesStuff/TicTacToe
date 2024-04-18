#ifndef _NETWORK_H
#define _NETWORK_H
int connect_inet(char *host, char *service);
int open_listener(char *service, int queue_size);
#endif