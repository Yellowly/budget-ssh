#include <pty.h>
#include <unistd.h>

/*
 * Pseudo-Terminals (PTYs) are created by the operating system to represent
 * terminals. Internally, they are simply two file descriptors which function
 * similar to sockets (2-way pipes). The "master" end of the socket is used to
 * read and write to processes running in the pseudo-terminal, while the slave
 * end of the socket should be given to the running process itself, with its
 * standard io redirected to the file.
 *
 * Because each user connected to a Secure Shell Host should have access to
 * their own terminal, one of these should be created every time a user
 * connects, and the master end should be redirected to the TCP socket
 * representing the connection.
 *
 * See https://man7.org/linux/man-pages/man7/pty.7.html or do `man pty` for more
 * info
 */
typedef struct PseudoTerminal {
  int master_fd;
  int slave_fd;
} PseudoTerminal;

/*
 * Makes a Pseudo Terminal
 *
 * If the pseudo terminal failed to open, both file descriptors will be -1
 */
PseudoTerminal make_pty();

/*
 * Closes the pseudo terminal by closing both file descriptors representing the
 * terminal
 */
int close_pty(PseudoTerminal *pterminal);
