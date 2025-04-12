#include <pseudo_terminal.h>

/*!
 * Opens a pseudo-terminal
 *
 * @returns -1 if the pseudoterminal failed to be opened, 0 otherwise.
 */
PseudoTerminal make_pty() {
  PseudoTerminal res = {-1, -1};
  openpty(&res.master_fd, &res.slave_fd, NULL, NULL, NULL);
  return res;
}

/*
 * Closes the pseudo terminal by closing both file descriptors representing the
 * terminal
 */
int close_pty(PseudoTerminal *pterminal) {
  int res = 0;
  if (pterminal->master_fd != -1)
    res |= close(pterminal->master_fd);
  if (pterminal->slave_fd != -1)
    res |= close(pterminal->slave_fd);

  pterminal->master_fd = -1;
  pterminal->slave_fd = -1;
  return res;
}
