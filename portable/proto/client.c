#include "network.h"

int main(int argc, char **argv) {
  int sock;
  if (argc >= 2 && strcmp(argv[1],"-u") == 0) {
    sock = make_un_socket(argv[2], CLIENT_SOCKET);
  } else {
    sock = make_socket(ADDRESS_PORT, CLIENT_SOCKET, "127.0.0.1");
  }
  
  send_data (sock, "Some data to be sent");
  close_socket(sock);
}