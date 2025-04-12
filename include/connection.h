#include "session.h"
#include <netinet/in.h>
#include <sys/socket.h>

typedef struct Connection {
  int socket_fd;
  Session *session;
} Connection;

/*!
 * Entry point for handling client connections.
 *
 * Client connections are handled by reading from the socket, checking for
 * certain keywords, writing to the client's associated shell session, and
 * sending back the output.
 *
 * If an error occurs while handling the connection, returns -1
 */
int handle_connection(int socket_fd);
