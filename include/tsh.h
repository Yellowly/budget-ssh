#include <stdio.h>
#include <stdlib.h>

/*
 * Used to construct commands which execute processes
 */
typedef struct Process {
  // [stdin, stdout]
  int stdio_fds[2];
  int argc;
  int capacity;
  char **args;
  Process *next;
} Process;

/*!
 * Represents a series of commands which will run consecutively
 *
 * Consecutive processes may or may not be piped together
 *
 * @args
 * `int size`
 * `Process *head`
 * `Process *tail`
 */
typedef struct ProcessChain {
  int size;
  Process *head;
  Process *tail;
} ProcessChain;

/*
 * Entry point for running the tsh shell
 */
int run_tsh();
