/*
 * CS3600, Spring 2013
 * Project 1 Starter Code
 * (c) 2013 Alan Mislove
 *
 * You should use this (very simple) starter code as a basis for 
 * building your shell.  Please see the project handout for more
 * details.
 */

#include "3600sh.h"

#define USE(x) (x) = (x)

int MAXLINE = 80;

char *strip_n(char *old) {
  char *new = (char *) calloc(strlen(old), sizeof(char));
  unsigned int i;
  for(i = 0; i < strlen(old); i++) {
    if((old[i] == '\\' && old[i+1] == 'n') || (old[i] == '\0')) {
	new[i] = '\0';
	break;
    } else {
	new[i] = old[i];
    }
  }
  
  char *saved_arg = (char *) calloc(strlen(new) + 1, sizeof(char));
  strcpy(saved_arg, new);

  return saved_arg;
}
  

char *get_argument(char *cmd, int i){
  char arg[30];
  int j = 0;
  while(cmd[i] != ' ' && cmd[i] != '\0'){
     
     arg[j] = cmd[i];
     i++;
     j++;
     
  }
  
  if( arg[j-1] == '\n')
    j--;
  arg[j] = '\0';
    
  char *saved_arg = (char *) calloc(strlen(arg) + 1, sizeof(char));
  strcpy(saved_arg, arg);

  return saved_arg;
}

/**
 * Gets user input. Stores raw input into cmd. Split arguments in an
 * array of strings, and stores them in argv. Returns the number of
 * arguments, or -1 for an error.
 */
int getargs(char cmd[], char *argv[]){
    
  char *f = fgets(cmd, MAXLINE, stdin);
  if(f == NULL && feof(stdin)){
    //printf("Couldn't read from stdin");
    do_exit();
  }

  int i = 0;
  int arg_num = 0;
  while(cmd[i] != '\0' && cmd[i] != '\n'){
    while (cmd[i] == ' '){
      i ++;
      if (cmd[i] == '\0' || cmd[i] == '\n')
        break;
    }
    if (cmd[i] == '\0' || cmd[i] == '\n')
      break;

    char *arg =get_argument(cmd, i);

    argv[arg_num] = arg;
    arg_num++;
    i += strlen(arg);
      
  }
  
  argv[arg_num] = NULL;

  return arg_num;
}

void free_args(int argc, char *argv[]){
  int i;
  for (i = 0; i < argc; i++){
    free(argv[i]);
  }
}

int parse_io_redirects(int childargc, char *childargv[]){
  int num_of_redirects = 0;
  
  int i;
  for(i = 0; i < childargc; i++){
    if(strcmp(childargv[i], "<") == 0){
      char *path = childargv[i+1];
      freopen(path, "r", stdin);
      num_of_redirects++;
    }
    else if(strcmp(childargv[i], ">") == 0){
      char *path = childargv[i+1];
      freopen(path, "w", stdout);
      num_of_redirects++;
    }
    else if(strcmp(childargv[i], "2>") == 0){
      char *path = childargv[i+1];
      freopen(path, "w", stderr);
      num_of_redirects++;
    }
  }

  childargc -= (num_of_redirects * 2);

  childargv[childargc] = NULL;

  return childargc;
}

void execute(int childargc, char *childargv[]){

  
  pid_t childpid;

  childpid = fork();
  if (childpid == -1){
    perror("FORK FAILED");
  }
  if (childpid == 0){

    if (*childargv[childargc-1] == '&'){
      childargv[childargc-1] = NULL;
      childargc--;
    }
    
    int orig_stdin, orig_stdout, orig_stderr;
    orig_stdin = dup(0);
    orig_stdout = dup(1);
    orig_stderr = dup(2);

    
    parse_io_redirects(childargc, childargv);

    execvp(childargv[0], childargv);
    
    // if anything executes past this point, execvp has failed. Reset
    // the IOs, print an Error and exit
    dup2(orig_stdin, 0);
    dup2(orig_stdout, 1);
    dup2(orig_stderr, 2);    

    if (errno == EACCES)
      printf("Error: Permission denied.\n");
    else
      printf("Error: Command not found.\n");
    exit(1);
  }
  
  // check if this should be a background process
  if (*childargv[childargc-1] != '&')
    waitpid(childpid, NULL, 0);  /* wait until child process finishes */
}


int main(int argc, char*argv[]) {
  // Code which sets stdout to be unbuffered
  // This is necessary for testing; do not change these lines
  USE(argc);
  USE(argv);
  setvbuf(stdout, NULL, _IONBF, 0); 
  
  char *username = getenv("USER");
  char hostname[64];
  gethostname(hostname, 64);

  char cmd[MAXLINE]; // a place to store user input
  int MAXARGS = 50;
  char *childargv[MAXARGS]; // split user input into args
  int childargc = 0;

  // Main loop that reads a command and executes it
  while (1) {
    free_args(childargc, childargv);
    char cur_dir[100];
    getcwd(cur_dir, 100);
    printf("%s@%s:%s> ", username, hostname, cur_dir);
      
    // You should read in the command and execute it here
    childargc = getargs(cmd, childargv);

    if ( childargc > 0 && strcmp(childargv[0], "exit") == 0){
      do_exit();
    }

    execute(childargc, childargv);

    if (feof(stdin))
      do_exit();
  }

  return 0;
}

// Function which exits, printing the necessary message
//
void do_exit() {
  printf("So long and thanks for all the fish!\n");
  exit(0);
}
