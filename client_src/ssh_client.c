#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int flag = 0; // For thread coordination

void *reader(void *arg) {
  int socket_fd = *(int *)arg;
  char buffer[1024];

  while (!flag) {
    ssize_t bytes_read = read(socket_fd, buffer, sizeof(buffer) - 1);
    if (bytes_read <= 0) {
      flag = 1;
      break;
    }

    buffer[bytes_read] = '\0';
    buffer[bytes_read - 1] = '\0';

    // This stores what's written in the socket fd  and prints to terminal, but
    // idk what else it needs to do
    write(STDOUT_FILENO, buffer, bytes_read);
    fsync(STDOUT_FILENO);
  }

  return NULL;
}

/*!
 * Connect a client to the given address
 * Creates and binds a socket to connection
 *
 * @returns the file descriptor of the connection socket, or -1 if opening
 * the socket failed
 */
int connect_client(struct sockaddr_in *addr) {
  // Create a socket
  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

  if (socket_fd < 0) {
    perror("Error opening socket\n");
    return -1;
  }

  // Connect to server
  if (connect(socket_fd, (struct sockaddr *)addr, sizeof(struct sockaddr_in)) <
      0) {
    perror("Error connecting to server\n");
    close(socket_fd);
    return -1;
  }

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

  // dup2(socket_fd, STDIN_FILENO);
  // dup2(socket_fd, STDOUT_FILENO);

  printf("Connected to server! Type 'exit' to quit.\n");

  char buffer[1024];
  // Loops until user exits the program
  pthread_t tid;

  pthread_create(&tid, NULL, reader, &socket_fd);

  // char buffer[1024];
  while (!flag) {
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
      flag = 1;
      break;
    }

    // Checks for exit command, quit if so
    // if (!strncmp(buffer, "exit", 4)) {
    //  flag = 1;
    //  break;
    //}

    write(socket_fd, buffer, strlen(buffer));
  }
  pthread_join(tid, NULL);
  close(socket_fd);

  printf("Client was closed\n");
  return 0;
  // you will need to create a separate thread to either read or write to the
  // socket exit once the user types 'exit' or 'EOF' is read from the socket
  // (this will happen if server closes our connection)
}
