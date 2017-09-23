/* Written By:Jaspreet Singh,MAC,ID:104658273
   Dated:20th FEB 2017
   This Code Creates a shell and executes commands as per user Input
*/
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*
  Declarations of functions for builtin shell commands:
 */
int sh_cd(char **args);
int sh_help(char **args);
int sh_exit(char **args);

/*
  List of builtin commands.
 */
char *builtin_str[] = {
  "cd",
  "help",
  "exit"
};

int (*builtin_func[]) (char **) = {
  &sh_cd,
  &sh_help,
  &sh_exit
};

int sh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

/*
  Builtin function implementations.
*/

/**
   //brief Bultin commands: change directory.
  // param args List of args.  args[0] is "cd".  args[1] is the directory.
  // return Always returns 1, to continue executing.
 */
int sh_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "sh: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("sh");
    }
  }
  return 1;
}

/**
   brief Builtin command: print help.
   param args List of args.  Not examined.
   return Always returns 1, to continue executing.
 */
int sh_help(char **args)
{
  int i;
  printf("Jaspreet Singh's Shell\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");

  for (i = 0; i < sh_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs.\n");
  return 1;
}

/**
   Builtin command: exit.
   param args List of args.  Not examined.
   return Always returns 0, to terminate execution.
 */
int sh_exit(char **args)
{
  return 0;
}

/**
  brief Launch a program and wait for it to terminate.
  param args Null terminated list of arguments (including program).
  return Always returns 1, to continue execution.
 */
int sh_launch(char **args)
{
  pid_t pid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process
    if (execvp(args[0], args) == -1) {
      perror("sh");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    perror("sh");
  } else {
    // Parent process
    do {
      waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

/**
    Execute shell built-in or launch program.
    args Null terminated list of arguments.
    return 1 if the shell should continue running, 0 if it should terminate
 */
int sh_execute(char **args)
{
  int i;

  if (args[0] == NULL) {
    // An empty command was entered.
    return 1;
  }

  for (i = 0; i < sh_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return sh_launch(args);
}

#define SH_RL_BUFSIZE 1024
/**
   brief Read a line of input from stdin.
   return The line from stdin.
 */
char *sh_read_line(void)
{
  int bufsize = SH_RL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) {
    fprintf(stderr, "sh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    // Read a character
    c = getchar();

    // If we hit EOF, replace it with a null character and return.
    if (c == EOF || c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;

    // If we have exceeded the buffer, reallocate.
    if (position >= bufsize) {
      bufsize += SH_RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, "sh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}

#define SH_TOK_BUFSIZE 64
#define SH_TOK_DELIM " \t\r\n\a"
/**
  Split a line into tokens (very naively).
  line The line.
  Null-terminated array of tokens.
 */
char **sh_split_line(char *line)
{
  int bufsize = SH_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token, **tokens_backup;

  if (!tokens) {
    fprintf(stderr, "sh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, SH_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += SH_TOK_BUFSIZE;
      tokens_backup = tokens;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
		free(tokens_backup);
        fprintf(stderr, "sh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, SH_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

/**
    Loop getting input and executing it.
 */
void sh_loop(void)
{
  char *line;
  char **args;
  int status;

  do {
    printf("> ");
    line = sh_read_line();
    args = sh_split_line(line);
    status = sh_execute(args);

    free(line);
    free(args);
  } while (status);
}

/**
    Main entry point.
    argc Argument count.
    argv Argument vector.
    status code
 */
int main(int argc, char **argv)
{
  // Load config files, if any.

  // Run command loop.
  sh_loop();

  // Perform any shutdown/cleanup.

  return EXIT_SUCCESS;
}
