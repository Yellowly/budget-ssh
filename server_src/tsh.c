#include <string.h>
#include <tsh.h>
#include <unistd.h>

/*!
 * Adds an argument to a process
 */
void add_arg(Process *proc, char *arg) {
  if (proc->args == NULL) {
    // initialize the args list if it is null
    proc->args = (char **)malloc(2 * sizeof(char *));
    proc->capacity = 2;
  } else if (proc->argc + 1 >= proc->capacity) {
    // if the args list is full, then double the capacity and copy over old
    // values
    char **temp = (char **)malloc(2 * sizeof(char *) * proc->capacity);
    memcpy(temp, proc->args, sizeof(char *) * proc->capacity);
    char **old = proc->args;
    proc->args = temp;
    proc->capacity *= 2;
    free(old);
  }
  // add the argument
  proc->args[proc->argc] = arg;
  proc->args[proc->argc + 1] = NULL;
  proc->argc++;
}

/*!
 * Creates a new empty process chain
 */
ProcessChain make_process_chain() { return (ProcessChain){0, NULL, NULL}; }

/*!
 * Creates a new process at the end of the process chain
 *
 * If piped is set to false (0) or the process chain is empty, the new process
 * will not get its output piped from the previous process in the chain.access
 * Otherwise, the last process will have its output piped to the new process in
 * the chain.
 */
Process *add_process(ProcessChain *process_list, char piped) {
  // create new process
  Process *new_proc = (Process *)malloc(sizeof(Process));
  new_proc->argc = 0;
  new_proc->args = NULL;
  new_proc->capacity = 0;
  new_proc->next = NULL;

  // set up pipes
  if (process_list->tail != NULL && piped) {
    int pipes[2];
    pipe(pipes);
    process_list->tail->stdio_fds[1] = pipes[1];
    new_proc->stdio_fds[0] = pipes[0];
  }

  // append to process chain
  process_list->tail->next = new_proc;
  process_list->size++;

  return new_proc;
}

/*!
 * Parses the given input command and populates a ProcessChain
 *
 * this should be similar to Project 1 - Shell
 */
void parse_input(char *input, ProcessChain *process_list) {
  // TODO
}

/*!
 * @brief Execute a list of commands using processes and pipes.
 *
 * this should be similar to Project 1 - Shell
 *
 * additionally, before calling fork(), check if the command is either 'cd' or
 * 'pwd'. in this case, use `chdir` / `getcwd` respectively. (see the man page
 * for usage)
 */
char run_commands(ProcessChain *process_list) {
  // TODO
  return 0;
}

/*
 * Entry point for the tsh shell
 * Should read input until new line, parse the input into a ProcessChain, then
 * run the process
 */
int run_tsh() {
  // TODO
  return -1;
}
