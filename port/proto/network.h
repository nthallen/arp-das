#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>

#include <stdlib.h>
#include <stdio.h>

#define ADDRESS_PORT 10203
#define ADDRESS_IP "127.0.0.1"
#define MAXPENDING 5
#define BUFFSIZE 21

#define SERVER_SOCKET 1
#define CLIENT_SOCKET 0

#define TRUE 1
#define FALSE 0
#define START 11
#define DIVIDER ":"

int make_socket(uint16_t port, int type, const char *server_IP);
int make_un_socket(const char *name, int type);
void close_socket(int socket);
void send_data (int socket, const char * data );

