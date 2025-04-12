#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

/*!
 * Connect a client to the given address
 * Creates and binds a socket to connection
 *
 * @returns the file descriptor of the connection socket, or -1 if opening
 * the socket failed
 */
int connect_client(struct sockaddr_in *addr) {
  // look at "Statistic Bot" from cs230 to do this

  // TODO: create a socket
  int socket_fd = -1;
  // TODO: connect to server

  return socket_fd;
}

/*!
 * Connect to the given address and run the client
 *
 * This should be called once in main.c
 *
 * @returns -1 if an error occurs while the client is running
 */
int run_client(struct sockaddr_in addr) {
  int socket_fd = connect_client(&addr);

  dup2(socket_fd, STDIN_FILENO);
  dup2(socket_fd, STDOUT_FILENO);

  while (1)
    ;

  return -1;
  // you will need to create a separate thread to either read or write to the
  // socket exit once the user types 'exit' or 'EOF' is read from the socket
  // (this will happen if server closes our connection)
}
