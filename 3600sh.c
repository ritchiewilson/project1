/**
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
    printf("Couldn't read from stdin");
    exit(1);
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

void execute(int childargc, char *childargv[]){

  // check if this should be a background process
  int bg = 0;
  if (*childargv[childargc-1] == '&'){
    bg = 1;
    childargv[childargc-1] = NULL;
    childargc--;
  }
  
  pid_t childpid;

  childargc++;
  childargc--;

  childpid = fork();
  if (childpid == -1){
    perror("FORK FAILED");
  }
  if (childpid == 0){
    

    int i;
    for(i = 0; i < childargc; i++){
      if(strcmp(childargv[i], ">") == 0){
	char *path = childargv[i+1];
	freopen(path, "w", stdout);
	childargv[i] = NULL;
	childargc -= 2;
	break;
      }
    }

    for(i = 0; i < childargc; i++){
      if(strcmp(childargv[i], "2>") == 0){
	char *path = childargv[i+1];
	freopen(path, "r", stderr);
	childargv[i] = NULL;
	childargc -= 2;
	break;
      }
    }

    for(i = 0; i < childargc; i++){
      if(strcmp(childargv[i], "<") == 0){
	char *path = childargv[i+1];
	freopen(path, "r", stdin);
	childargv[i] = NULL;
	childargc -= 2;
	break;
      }
    }

    execvp(childargv[0], childargv);
    
    // This code only runs if exec fails. Print error and exit
    perror("exec failed");
    exit(1);
  }
  
  if (bg == 0)
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
  }

  return 0;
}

// Function which exits, printing the necessary message
//
void do_exit() {
  printf("So long and thanks for all the fish!\n");
  exit(0);
}
