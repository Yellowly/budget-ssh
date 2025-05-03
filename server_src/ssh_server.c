#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <session.h>
#include <signal.h>

int server_socket;

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
  //
  // You may get "Failed to start server: Address already in use" while testing.
  // The error should go away on its own if you wait a minute before trying to
  // restart the server. But if it gets too annoying you can uncomment this code
  // to instantly reuse the last address for the server

  // int opt = 1; if
  // (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
  //               sizeof(opt)))
  //   return -1;

  if (bind(socket_fd, (struct sockaddr *)addr, sizeof(*addr)) != 0)
    return -1;

  return socket_fd;
}

void on_interrupt(int sig) {
  if (sig == SIGINT) {
    close(server_socket);
    server_socket = -1;
  }
}

void register_close_sig() { signal(SIGINT, on_interrupt); }

void *conn_handler(void *args) {
  int socket_fd = *(int *)args;
  handle_connection(socket_fd);
  close(socket_fd);
  printf("Client on socket %d was disconnected\n", socket_fd);
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
    perror("Failed to start server");
    return -1;
  }

  // register a handler to properly close server socket
  // when the server gets closed (TODO: close clients)
  server_socket = server_sock;
  register_close_sig();

  // set up listening socket
  if (listen(server_sock, 5) != 0) {
    printf("An error occured while setting up listening socket\n");
    return -1;
  }

  printf("Server listening on %s:%d\n", inet_ntoa(addr.sin_addr),
         ntohs(addr.sin_port));

  // setup sessions which clients can connect to
  init_sessions(8);

  // check for connections
  while (server_socket != -1) {
    // listens and creates socket for newly connected clients
    int client_sock;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    if ((client_sock = accept(server_sock, (struct sockaddr *)&client_addr,
                              &client_addr_len)) < 0) {
      if (server_socket == -1)
        break;
      perror("An error occured while accepting a connection");
    }

    printf("Connection made with %s:%d on socket %d\n",
           inet_ntoa(client_addr.sin_addr), client_addr.sin_port, client_sock);

    // start a thread for handling the client connection
    pthread_t client_thread;
    if (pthread_create(&client_thread, NULL, conn_handler,
                       (void *)&client_sock) == 0) {
      pthread_detach(client_thread);
    } else {
      perror("An error occured while creating a thread for the client\n");
    }
  }

  printf("Server was closed\n");
  return 0;
}
