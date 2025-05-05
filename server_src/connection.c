#include <connection.h>
// #include <pthread.h>
// #include <stdio.h>
// #include <string.h>

// SessionReader session_readers[16];

/*!
 * Responsible from reading from a session and writing the data to all its
 * connections
 */
/*
void *session_reader(void *args) {
  SessionReader *reader = (SessionReader *)args;
  char msg_buffer[1024];
  while (1) {
    int num_bytes = read(reader->session->terminal.master_fd, msg_buffer, 1024);
    pthread_mutex_lock(&reader->lock);
    for (int i = 0; i < reader->num_conns; i++) {
      pthread_mutex_lock(&reader->conns[i].socket_lock);
      send(reader->conns[i].socket_fd, msg_buffer, num_bytes, 0);
      pthread_mutex_unlock(&reader->conns[i].socket_lock);
    }
    pthread_mutex_unlock(&reader->lock);
  }
}

int handle_connection(int socket_fd) {
  Session *session = make_session();
  if (session == NULL)
    return -1;

  Connection conn;
  conn.socket_fd = socket_fd;
  conn.session = session;
  pthread_mutex_init(&conn.socket_lock, NULL);

  SessionReader s_reader;
  s_reader.session = session;
  pthread_mutex_init(&s_reader.lock, NULL);
  s_reader.num_conns = 1;
  s_reader.conns = &conn;

  pthread_t writer;
  pthread_create(&writer, NULL, session_reader, &conn);

  char msg_buffer[1024];
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
      if (memcmp(msg_buffer, "bssh conns", num_bytes) == 0) {
        int active_sids[256];
        int num_sessions = list_sessions(active_sids, 256);
        pthread_mutex_lock(&conn.socket_lock);
        for (int i = 0; i < num_sessions; i++) {
          sprintf(msg_buffer, "%d, ", active_sids[i]);
          write(conn.socket_fd, msg_buffer, 6);
        }
        write(conn.socket_fd, "\n\n", 2);
        pthread_mutex_unlock(&conn.socket_lock);
        // int active_sids[256] = ;
        // sprintf(msg_buffer, "echo bsh conns; echo %d"
        continue;
      } else if (memcmp(msg_buffer, "bssh join ", 10)) {
        int sid;
        sscanf(msg_buffer, "bssh join %d", &sid);
        Session *s = get_session(sid);
      }

      // write to the session
      pthread_mutex_lock(&session->session_lock);
      char ends_with_newline = msg_buffer[num_bytes - 1] == '\n';
      write(session->terminal.master_fd, msg_buffer, num_bytes);

      if (ends_with_newline)
        pthread_mutex_unlock(&session->session_lock);
    }
  }
  pthread_mutex_unlock(&session->session_lock);
  pthread_cancel(writer);
  pthread_join(writer, NULL);

  close_session(session);

  return res;
}*/

int write_to_conn(Connection *c, char *buf, ssize_t len) {
  pthread_mutex_lock(&c->socket_lock);
  int res = send(c->socket_fd, buf, len, 0);
  int res2 = send(c->socket_fd, "\n", 1, 0);
  pthread_mutex_unlock(&c->socket_lock);
  return res | res2;
}
