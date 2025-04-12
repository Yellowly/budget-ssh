#include <connection.h>

void *sock_writer(void *args) {
  Connection *conn = (Connection *)args;
  unsigned char msg_buffer[1024];
  while (1) {
    int num_bytes = read(conn->session->terminal.master_fd, msg_buffer, 1024);
    send(conn->socket_fd, msg_buffer, num_bytes, 0);
  }
}

int handle_connection(int socket_fd) {
  Session *session = make_session();
  if (session == NULL)
    return -1;

  Connection conn = {socket_fd, session};

  pthread_t writer;
  pthread_create(&writer, NULL, sock_writer, &conn);

  unsigned char msg_buffer[1024];
  int res = 0;
  while (1) {
    int num_bytes = recv(socket_fd, msg_buffer, 1024, 0);
    if (num_bytes < 0) {
      printf("Could not receive message\n");
      res = -1;
      break;
    } else if (num_bytes == 0) {
      break;
    } else {
      // check for keywords (such as joining sessions or signals)
      // TODO

      // write to the session
      pthread_mutex_lock(&session->session_lock);
      char ends_with_newline = msg_buffer[num_bytes - 1] == '\n';
      write(session->terminal.master_fd, msg_buffer, num_bytes);

      if (ends_with_newline)
        pthread_mutex_unlock(&session->session_lock);
    }
  }
  pthread_mutex_unlock(&session->session_lock);
  pthread_join(writer, NULL);

  close_session(session);

  return res;
}
