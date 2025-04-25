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
  int redirect_output;  // Flag for output redirection
  char *output_file;    // Output file name if redirecting
  int background; // Flag for background execution
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

/*!
 * Parses the given input command and populates a ProcessChain
 */
void parse_input(char *input, ProcessChain *process_list);

/*!
 * Execute a list of commands using processes and pipes
 */
char run_commands(ProcessChain *process_list);

/*
 * Entry point for running the tsh shell
 */
int run_tsh();
