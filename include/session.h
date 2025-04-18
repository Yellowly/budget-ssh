#include "pseudo_terminal.h"
#include "tsh.h"
#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <signal.h>
#include <sys/wait.h>

/*!
 * Abstracts a process representing a shell session running on the server
 *
 * Contains the PseudoTerminal the shell is running in, a lock for accessing the
 * terminal, and the pid of the shell's process
 *
 * These should be initialized once when init_sessions() is called, and
 * activated using `make_session()` Make sure to call `close_session()` before a
 * client drops a session
 */
typedef struct Session {
  PseudoTerminal terminal;
  pthread_mutex_t session_lock;
  // process id of underlying shell
  int pid;
  // index of this session in the internal sessions array
  int sid;
  int num_conns;
} Session;

/*!
 * Initializes a certain number of sessions that this server host can create and
 * use
 */
int init_sessions(int _max_sessions);

/*!
 * Makes a new client session running a shell
 * Increments a internal reference counter to the session
 */
Session *make_session();

/*!
 * Gets an already running session by session id
 */
Session *get_session(int sid);

/*!
 * Attempts to close a session.
 * Note that the session will only actually close if it has no more connections
 * referencing it.
 */
int close_session(Session *session);

/*!
 * Entry point for running a client session.
 *
 * Should get called in a new process created for the client.
 *
 * An argument of type int should be provided to the client session,
 * representing the file descriptor of the socket used to communicate with the
 * client
 */
void run_session(int socket_fd);

/*!
 * Closes the given session
 */
int close_session(struct Session *session);
