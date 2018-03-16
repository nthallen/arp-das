#include <unistd.h>
#include "network.h"

const char * create_authorization(const char * experiment, int client_socket, int server_socket) {
	// Space-delimited password created here.
	// Code added by Miles on 16 March 2018.
	const char * auth = experiment + " " + client_socket + " " + server_socket);
	return auth;
}

int accept_connection(int server_socket) {
  // Altered by Miles on 15 March 2018.
  int client_socket; /* Socket descriptor for client */
  const char * super_secret_password = create_authorization("experiment_a", client_socket, server_socket);
  struct sockaddr_in client_address; /* Client address */
  unsigned int client_length; /* Length of client address data structure */

  /* Set the size of the in-out parameter */
  client_length = sizeof(client_address);

  /* Wait for a client to connect */
  if ((client_socket =
	  accept(server_socket, (struct sockaddr *) &client_address,
	    &client_length)) < 0) {
    printf("accept() failed")
  }

  // send_data(client_socket, "Enter authorization credentials: ");
  const char * return_message = handle_client(client_socket);
  if (return_message.compare(super_secret_password)==0) {
	/* client_socket is connected to a client! */
	printf("Handling client %s\n", inet_ntoa(client_address.sin_addr));
	send_data(client_socket, "Okay. Not encrypted but whatever.");
	return client_socket;
  } else {
	close_socket(client_socket);
    printf("accept() failed");
  }
}

void handle_client (int client_socket) {
  char buffer [BUFFSIZE]; /* Buffer for incomming data */
  int msg_size; /* Size of received message */
  int bytes, all_bytes;

  do {
    alarm (60);
    msg_size = read (client_socket, buffer, BUFFSIZE);
    alarm (0);

    if ( msg_size <= 0 ) {
      printf ( " %i ", msg_size );
      printf ( "End of data\n" );
    }
  } while ( msg_size > 0 );
  printf ("Data received: %s\n", buffer);
  bytes = 0;
}

int main(int argc, char **argv) {
  int clnt_sock, sock, i;

  if (argc >= 2 && strcmp(argv[1],"-u") == 0) {
    sock = make_un_socket(argv[2], SERVER_SOCKET);
  } else {
    sock = make_socket(ADDRESS_PORT, SERVER_SOCKET, "none");
  }
  clnt_sock = accept_connection (sock);
  handle_client(clnt_sock);
  close_socket(sock);
}