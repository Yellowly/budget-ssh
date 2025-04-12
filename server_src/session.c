#include <session.h>

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
        if (sessions[i].num_conns == 0) {
          kill(sessions[i].pid, -9);
        }
        int status;
        if (waitpid(sessions[i].pid, &status, WNOHANG) > 0) {
          sessions[i].pid = -1;
          close_pty(&sessions[i].terminal);
          pthread_mutex_destroy(&sessions[i].session_lock);
        }
      }
    }
  }
}

int init_sessions(int _max_sessions) {
  sessions = (Session *)malloc(sizeof(Session) * _max_sessions);
  max_sessions = _max_sessions;
}

Session *make_session() {
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
  sessions[sid].pid = pid;
  sessions[sid].sid = sid;

  // starts a session reaper
  if (session_reaper == -1) {
    // if this fails then the program probably just explodes idk
    pthread_create(&session_reaper, NULL, reap_task, NULL);
    pthread_detach(session_reaper);
  }

  return &sessions[sid];
}

Session *get_session(int sid) { return &sessions[sid]; }

int close_session(Session *session) { session->num_conns--; }
