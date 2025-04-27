#include <string.h>
#include <tsh.h>
#include <unistd.h>
#include <fcntl.h>    // For open, O_RDONLY, etc.
#include <sys/wait.h> // For waitpid

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
  new_proc->redirect_output = 0;
  new_proc->output_file = NULL;
  new_proc->background = 0;
  new_proc->stdio_fds[0] = STDIN_FILENO;  // Default input
  new_proc->stdio_fds[1] = STDOUT_FILENO; // Default output

  // set up pipes
  if (process_list->tail != NULL && piped) {
    int pipes[2];
    pipe(pipes);
    process_list->tail->stdio_fds[1] = pipes[1];
    new_proc->stdio_fds[0] = pipes[0];
  }

  // append to process chain
  if (process_list->head == NULL) {
    process_list->head = new_proc;
    process_list->tail = new_proc;
  } else {
    process_list->tail->next = new_proc;
    process_list->tail = new_proc;
  }
  process_list->size++;

  return new_proc;
}

/*!
 * Parses the given input command and populates a ProcessChain
 *
 * this should be similar to Project 1 - Shell
 */
void parse_input(char *input, ProcessChain *process_list) {
  char *input_copy = strdup(input);
  char *token;
  Process *current_proc = NULL;
  char is_piped = 0;
  
  // Initialize the process list
  *process_list = make_process_chain();

  // First handle semicolons (command separation)
  char *commands = input_copy;
  char *next_command = NULL;
  
  while (commands != NULL) {
    // Extract the next command separated by semicolon
    next_command = strchr(commands, ';');
    if (next_command) {
      *next_command = '\0'; // Terminate at semicolon
      next_command++; // Move pointer past semicolon
    }
    
    // Skip leading whitespace
    while (*commands == ' ' || *commands == '\t') commands++;
    
    // Skip if command is empty
    if (*commands == '\0') {
      commands = next_command;
      continue;
    }

    // Check for background processing at the end of command
    char background = 0;
    int cmd_len = strlen(commands);
    if (cmd_len > 0 && commands[cmd_len - 1] == '&') {
      commands[cmd_len - 1] = '\0'; // Remove the & symbol
      cmd_len--;
      // Remove trailing whitespace before &
      while (cmd_len > 0 && (commands[cmd_len - 1] == ' ' || commands[cmd_len - 1] == '\t')) {
        commands[cmd_len - 1] = '\0';
        cmd_len--;
      }
      background = 1;
    }

    // Parse commands separated by pipes
    char *cmd_str = strdup(commands);
    char *pipe_token = strtok(cmd_str, "|");
    is_piped = 0;  // Reset pipe flag for new command
    
    while (pipe_token != NULL) {
      // Create new process
      current_proc = add_process(process_list, is_piped);
      
      // Default stdio
      current_proc->stdio_fds[0] = STDIN_FILENO;
      current_proc->stdio_fds[1] = STDOUT_FILENO;
      
      // Make a copy to work with
      char *cmd_copy = strdup(pipe_token);
      
      // Check for output redirection
      char *output_redir = strstr(cmd_copy, ">");
      
      // Check for input redirection
      char *input_redir = strstr(cmd_copy, "<");
      
      // Handle redirections - need to be careful with order of operations
      // First, identify and null-terminate both redirections if they exist
      if (output_redir) {
        *output_redir = '\0';
        output_redir++;
      }
      
      if (input_redir) {
        *input_redir = '\0';
        input_redir++;
      }
      
      // Parse arguments for this process (before any redirection symbols)
      char *arg_tok;
      arg_tok = strtok(cmd_copy, " \t\n");
      while (arg_tok != NULL) {
        add_arg(current_proc, strdup(arg_tok));
        arg_tok = strtok(NULL, " \t\n");
      }
      
      // Handle input redirection if present
      if (input_redir) {
        while (*input_redir == ' ' || *input_redir == '\t') input_redir++; // Skip whitespace
        
        // Extract the input file name
        char *input_file = NULL;
        char *end_of_input_file = NULL;
        
        // Find first non-whitespace character
        while (*input_redir && (*input_redir == ' ' || *input_redir == '\t')) {
          input_redir++;
        }
        
        if (*input_redir) {
          input_file = input_redir;
          
          // Find end of filename (space or redirection symbol)
          end_of_input_file = input_file;
          while (*end_of_input_file && *end_of_input_file != ' ' && 
                 *end_of_input_file != '\t' && *end_of_input_file != '>' &&
                 *end_of_input_file != '<') {
            end_of_input_file++;
          }
          
          if (*end_of_input_file) {
            *end_of_input_file = '\0';
          }
          
          // Open the input file and set as stdin for the process
          int fd = open(input_file, O_RDONLY);
          if (fd >= 0) {
            current_proc->stdio_fds[0] = fd;
          } else {
            perror("open input file");
          }
        }
      }
      
      // Handle output redirection if present
      if (output_redir) {
        while (*output_redir == ' ' || *output_redir == '\t') output_redir++; // Skip whitespace
        
        char *output_file = strtok(output_redir, " \t\n");
        if (output_file) {
          // Store output redirection information
          current_proc->redirect_output = 1;
          current_proc->output_file = strdup(output_file);
        }
      }
      
      // Before advancing to next token, check if this is the last process and set background flag
      char *next_pipe = strtok(NULL, "|");
      if (background && next_pipe == NULL) {
        // This is the last process in the pipeline and background was requested
        current_proc->background = 1;
      }
      
      free(cmd_copy);
      is_piped = 1;  // Set piped flag for subsequent processes
      pipe_token = next_pipe;  // Advance to next token
    }
    
    free(cmd_str);
    
    // Reset pipe flag for the next command after semicolon
    is_piped = 0;
    
    // Move to next command after semicolon
    commands = next_command;
  }
  
  free(input_copy);
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
  if (process_list->head == NULL || process_list->size == 0) {
    return 1;  // Nothing to run
  }
  
  Process *current = process_list->head;
  int status;
  pid_t pid;
  
  // Iterate through each process in the chain
  while (current != NULL) {
    // Handle built-in commands
    if (current->argc > 0) {
      // Handle cd command
      if (strcmp(current->args[0], "cd") == 0) {
        if (current->argc > 1) {
          if (chdir(current->args[1]) != 0) {
            perror("cd");
          }
        } else {
          // cd with no arguments should go to home directory
          char *home = getenv("HOME");
          if (home && chdir(home) != 0) {
            perror("cd");
          }
        }
        current = current->next;
        continue;
      }
      
      // Handle pwd command
      if (strcmp(current->args[0], "pwd") == 0) {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
          printf("%s\n", cwd);
        } else {
          perror("pwd");
        }
        current = current->next;
        continue;
      }
    }
    
    // Fork a new process
    pid = fork();
    
    if (pid < 0) {
      // Fork failed
      perror("fork");
      return -1;
    } else if (pid == 0) {
      // Child process
      
      // Set up pipe redirections
      if (current->stdio_fds[0] != STDIN_FILENO) {
        dup2(current->stdio_fds[0], STDIN_FILENO);
        close(current->stdio_fds[0]);
      }
      
      if (current->stdio_fds[1] != STDOUT_FILENO) {
        dup2(current->stdio_fds[1], STDOUT_FILENO);
        close(current->stdio_fds[1]);
      }
      
      // Handle output redirection
      if (current->redirect_output && current->output_file) {
        int fd = open(current->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) {
          perror("open output file");
          exit(EXIT_FAILURE);
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
      }
      
      // Execute the command
      execvp(current->args[0], current->args);
      
      // If execvp returns, there was an error
      perror("execvp");
      exit(EXIT_FAILURE);
    } else {
      // Parent process
      
      // Close pipe file descriptors in the parent
      if (current->stdio_fds[0] != STDIN_FILENO) {
        close(current->stdio_fds[0]);
      }
      
      if (current->stdio_fds[1] != STDOUT_FILENO) {
        close(current->stdio_fds[1]);
      }
      
      // Handle background process
      if (current->background) {
        printf("[%d] %s running in background\n", pid, current->args[0]);
        // Don't wait for background processes
      } else {
        // Wait for the child to complete
        waitpid(pid, &status, 0);
      }
      
      // Move to the next process in the chain
      current = current->next;
    }
  }
  
  return 0;
}

/*
 * Entry point for the tsh shell
 * Should read input until new line, parse the input into a ProcessChain, then
 * run the process
 */
int run_tsh() {
  char input[1024];
  ProcessChain process_list;
  int running = 1;
  
  // Initialize the shell with a prompt
  printf("tsh> ");
  fflush(stdout);
  
  // Main shell loop
  while (running && fgets(input, sizeof(input), stdin) != NULL) {
    // Remove trailing newline
    size_t len = strlen(input);
    if (len > 0 && input[len-1] == '\n') {
      input[len-1] = '\0';
    }
    
    // Skip if input is empty
    if (strlen(input) == 0) {
      printf("tsh> ");
      fflush(stdout);
      continue;
    }
    
    // Check for exit command
    if (strcmp(input, "exit") == 0) {
      running = 0;
      continue;
    }
    
    // Parse the input and run commands
    parse_input(input, &process_list);
    run_commands(&process_list);
    
    // Display prompt for next command
    printf("tsh> ");
    fflush(stdout);
  }
  
  return 0;
}
