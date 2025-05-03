#include <netinet/in.h>
#include <pthread.h>
#include <sys/socket.h>

typedef struct Connection {
  int socket_fd;
  pthread_mutex_t socket_lock;
  int id; // index of this connection in its corresponding session
} Connection;

// typedef struct SessionReader {
//  Session *session;
//  pthread_mutex_t lock;
//  int num_conns;
//  Connection *conns;
//} SessionReader;
//
//
int write_to_conn(Connection *c, char *buf, ssize_t len);
