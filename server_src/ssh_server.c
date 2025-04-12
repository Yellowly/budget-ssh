#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <connection.h>

/*!
 * Starts a server using the given address.
 * Creates and binds a socket to the port of the given address.
 *
 * @returns the file descriptor of the server's main socket, or -1 if opening
 * the socket failed
 */
int start_server(struct sockaddr_in *addr) {
  // create socket
  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd < 0)
    return -1;

  // set socket options
  int opt = 1;
  if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                 sizeof(opt)))
    return -1;

  if (bind(socket_fd, (struct sockaddr *)addr, sizeof(*addr)) < 0)
    return -1;

  return socket_fd;
}

void *conn_handler(void *args) {
  int socket_fd = *(int *)args;
  handle_connection(socket_fd);
  return NULL;
}

/*!
 * Runs a server on the given address
 *
 * This should be called once in main.c
 */
int run_server(struct sockaddr_in addr) {
  // start the server by creating a binding a socket to the given port
  int server_sock;
  if ((server_sock = start_server(&addr)) < 0) {
    printf("Failed to start server\n");
    return -1;
  }
  socklen_t addr_size = sizeof(addr);

  printf("Server listening on %s:%d\n", inet_ntoa(addr.sin_addr),
         addr.sin_port);

  // check for connections
  while (1) {
    // blocks until a connection is made
    if (listen(server_sock, 5) < 0) {
      printf("An error occured while making a connection\n");
    }

    // create a socket for the newly connected client
    int client_sock;
    if ((client_sock =
             accept(server_sock, (struct sockaddr *)&addr, &addr_size)) < 0) {
      printf("An error occured while accepting a connection\n");
    }

    // start a thread for handling the client connection
    pthread_t client_thread;
    if (pthread_create(&client_thread, NULL, conn_handler, &client_sock) ==
        -1) {
      pthread_detach(client_thread);
    } else {
      printf("An error occured while creating a thread for the client\n");
    }
  }

  return 0;
}
