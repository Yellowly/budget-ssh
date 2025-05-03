#include <session.h>
#include <string.h>
#include <unistd.h>

Session *sessions;
int max_sessions = 0;
pthread_mutex_t sessions_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_t session_reaper = -1;

int null_idx() {
  for (int i = 0; i < max_sessions; i++)
    if (sessions[i].pid == -1)
      return i;
  return -1;
}

void *reap_task(void *arg) {
  while (1) {
    for (int i = 0; i < max_sessions; i++) {
      if (sessions[i].pid > 0) {
        if (pthread_mutex_trylock(&sessions[i].conns_lock) == 0 &&
            sessions[i].num_conns == 0) {
          kill(sessions[i].pid, 9);
          pthread_mutex_unlock(&sessions[i].conns_lock);
        }
        int status;
        if (waitpid(sessions[i].pid, &status, WNOHANG) > 0) {
          pthread_join(sessions[i].reader, NULL);
          sessions[i].pid = -1;
          close_pty(&sessions[i].terminal);
          pthread_mutex_destroy(&sessions[i].session_lock);
          free(sessions[i].conns);
        }
      }
    }
  }
}

void *session_reader(void *arg) {
  Session *session = (Session *)arg;
  char msg_buffer[1024];
  while (1) {
    int num_bytes = read(session->terminal.master_fd, msg_buffer, 1024);
    if (num_bytes == 0)
      break;

    pthread_mutex_lock(&session->conns_lock);
    if (session->num_conns == 0)
      break;

    for (int i = 0; i < session->num_conns; i++)
      write_to_conn(session->conns[i], msg_buffer, num_bytes);

    pthread_mutex_unlock(&session->conns_lock);
  }
  return NULL;
}

int init_sessions(int _max_sessions) {
  sessions = (Session *)malloc(sizeof(Session) * _max_sessions);
  max_sessions = _max_sessions;
  for (int i = 0; i < max_sessions; i++) {
    sessions[i].sid = i;
    sessions[i].pid = -1;
    pthread_mutex_init(&sessions[i].conns_lock, NULL);
    sessions[i].num_conns = 0;
  }
  return max_sessions;
}

Session *make_session(Connection *conn, int max_conns) {
  pthread_mutex_lock(&sessions_lock);
  // get an unusued session
  int sid = null_idx();
  if (sid == -1) {
    pthread_mutex_unlock(&sessions_lock);
    return NULL;
  }
  sessions[sid].pid = 0;
  pthread_mutex_unlock(&sessions_lock);

  // initialize session's pseudo-terminal mutex
  if (pthread_mutex_init(&sessions[sid].session_lock, NULL) == -1) {
    sessions[sid].pid = -1;
    return NULL;
  }

  // initialize the pseudo-terminal
  sessions[sid].terminal = make_pty();
  if (sessions[sid].terminal.master_fd == -1) {
    sessions[sid].pid = -1;
    return NULL;
  }

  // fork a child process running a shell
  int pid = fork();
  if (pid == -1) {
    sessions[sid].pid = -1;
    return NULL;
  } else if (pid == 0) {
    close(sessions[sid].terminal.master_fd);
    dup2(sessions[sid].terminal.slave_fd, STDIN_FILENO);
    dup2(sessions[sid].terminal.slave_fd, STDOUT_FILENO);
    dup2(sessions[sid].terminal.slave_fd, STDERR_FILENO);

    // run tsh, exiting once it terminates.
    // alternatively, you could just exec bash
    int status = run_tsh();
    exit(status);
  }
  close(sessions[sid].terminal.slave_fd);
  sessions[sid].terminal.slave_fd = -1;

  sessions[sid].num_conns = 1;
  sessions[sid].max_conns = max_conns;
  sessions[sid].conns = (Connection **)malloc(sizeof(Connection *) * max_conns);
  conn->id = 0;
  sessions[sid].conns[0] = conn;
  sessions[sid].pid = pid;
  sessions[sid].sid = sid;

  pthread_create(&sessions[sid].reader, NULL, session_reader, &sessions[sid]);

  // starts a session reaper
  if (session_reaper == -1) {
    // if this fails then the program probably just explodes idk
    pthread_create(&session_reaper, NULL, reap_task, NULL);
    pthread_detach(session_reaper);
  }

  return &sessions[sid];
}

Session *get_session(int sid) { return &sessions[sid]; }

Session *join_session(Connection *conn, int sid) {
  pthread_mutex_lock(&sessions[sid].conns_lock);
  if (sessions[sid].pid <= 0 || sessions[sid].num_conns == 0 ||
      sessions[sid].num_conns >= sessions[sid].max_conns) {
    pthread_mutex_unlock(&sessions[sid].conns_lock);
    return NULL;
  }
  conn->id = sessions[sid].num_conns;
  sessions[sid].conns[sessions[sid].num_conns++] = conn;
  pthread_mutex_unlock(&sessions[sid].conns_lock);
  return &sessions[sid];
}

int list_sessions(int *sids, int n) {
  int count = 0;
  for (int i = 0; i < max_sessions && count < n; i++)
    if (sessions[i].sid > 0)
      sids[count++] = sessions[i].sid;

  return count;
}

/*!
 * @brief signal that a client is disconnecting from the session
 *
 * @note the session will not actually be closed unless all connections
 * to it have been closed.
 *
 * @important MUST BE CALLED WHEN A CONNECTION GETS CLOSED
 * VERY BAD UNDEFINED BEHAVIOR HAPPENS IF NOT
 */
int close_session(Connection *conn, int sid) {
  pthread_mutex_lock(&sessions[sid].conns_lock);
  for (int i = conn->id; i < sessions[sid].num_conns - 1; i++) {
    sessions[sid].conns[i] = sessions[sid].conns[i + 1];
    sessions[sid].conns[i]->id = i;
  }

  sessions[sid].num_conns--;
  pthread_mutex_unlock(&sessions[sid].conns_lock);
  return 0;
}

/*!
 * Handle a connection made to the server
 */
int handle_connection(int socket_fd) {
  Session *session = make_session();
  if (session == NULL)
    return -1;

  Connection conn;
  conn.socket_fd = socket_fd;
  // conn.session = session;
  pthread_mutex_init(&conn.socket_lock, NULL);

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
          int len = sprintf(msg_buffer, "%d, ", active_sids[i]);
          write(conn.socket_fd, msg_buffer, len);
        }
        write(conn.socket_fd, "\n\n", 2);
        pthread_mutex_unlock(&conn.socket_lock);

        continue;

      } else if (memcmp(msg_buffer, "bssh join ", 10)) {
        int sid;
        if (sscanf(msg_buffer, "bssh join %d", &sid) > 0) {
          Session *new_ses = join_session(&conn, sid);

          if (new_ses == NULL)
            write_to_conn(&conn, (char *)"Could not join session\n\n", 24);
          else {
            close_session(&conn, session->sid);
            session = new_ses;
          }
        }
        continue;
      }

      // write to the session
      if (pthread_mutex_trylock(&session->session_lock) == 0) {
        char ends_with_newline = msg_buffer[num_bytes - 1] == '\n';
        write(session->terminal.master_fd, msg_buffer, num_bytes);

        if (ends_with_newline)
          pthread_mutex_unlock(&session->session_lock);
      } else {
        write_to_conn(&conn,
                      (char *)"Could not run command: Someone else just sent a "
                              "command\n\n",
                      58);
      }
    }
  }
  pthread_mutex_unlock(&session->session_lock);

  close_session(session);

  return res;
}
