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
unsigned int BUFFER_LEN = 512;
int ESC_SPACES = 0;

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

char *handle_escape_char(char *cmd) {
  
  // Allocate 128 bytes for the new cmd
  char *new_cmd = malloc(BUFFER_LEN);
  unsigned int curr_length = BUFFER_LEN;

  // Indices for parsing cmd (i) & new_cmd (j)
  unsigned int i = 0;
  unsigned int j = 0;

  // Escaped char/regular char to be added to new_cmd 
  char c;

  // Parse raw command string and look for escape characters
  while(cmd[i] != '\0')
  { 
    // If escape character is found
    if(cmd[i] == '\\')
    {
      i++;
      switch(cmd[i])
      {
	case '\\':
	  c = '\\';
	  break;
	case ' ':
	  c = '\\';  
   	  new_cmd[j++] = c;
    	  if(j == curr_length)
    	  {
	    curr_length = j+BUFFER_LEN;
    	    new_cmd = realloc(new_cmd, BUFFER_LEN);
    	  }
    	  //i++;
	  c = ' ';
	  break;
	case 't':
	  c = '\t';
   	  //new_cmd[j++] = c;
    	  //if(j == curr_length)
    	  //{
	   // curr_length = j+BUFFER_LEN;
    	   // new_cmd = realloc(new_cmd, BUFFER_LEN);
    	  //}
    	  //i++;
	  //c = 't';
	  break;
	case '&':
	  c = '&';
	  break;
	// Print error message if unlisted escape sequence is used
        default:
	  printf("Error: Unrecognized escape sequence.\n");
	  do_exit();
	  break;
      } 
    }
    // If ampersand not following an escape char
    else if(cmd[i] == '&')    
    {  
       unsigned int k = i;			// New index (preserves i for later)
       k++;
       while (cmd[k] != '\0' && cmd[k] != '\n')	// Parse until NULL or newline
       {
	  if(cmd[k] == ' ')		// Keep going if char is space
	  {
	    k++;
	  }
	  else				// Break if it is any other char
	  {
	    printf("Error: Invalid syntax.\n");
  	    do_exit();
          }
	}
	c = cmd[i];
    }
    else if(cmd[i] == '\t')
    {
      c = ' ';
    }
    // If no escape char, store the next char in cmd to c 
    else
    {
      c = cmd[i];
    }
    // Add c to new_cmd, check if j reached BUFFER_LEN, if so allocate mem
    new_cmd[j++] = c;
    if(j == curr_length)
    {
	curr_length = j+BUFFER_LEN;
    	new_cmd = realloc(new_cmd, BUFFER_LEN);
    }
    i++;
  }

  new_cmd[j] = '\0';
  return new_cmd; 
}  

/**
 * Takes a string and the index of that string pointing to an argument.
 * Parses until a space, newline, or null byte, then returns that argument.
 */
char *get_argument(char *cmd, int i){
  
  unsigned int max_len = BUFFER_LEN;    // To be deleted
  unsigned int curr_len = 0;		// Size for the receiving array

  char *arg = malloc(BUFFER_LEN);       // Allocating 128bytes for receiving array
  curr_len = BUFFER_LEN;                // Correct the size for receiving array to 128
  
  char c;

  unsigned int j = 0; 			// Index for the receiving array
 
  // Parse the command string until a space or NULL is reached
  while(cmd[i] != ' ' && cmd[i] != '\0')
  { 
    // If there is an escaped space, store it and increment an extra time
    if(cmd[i] == '\\' && cmd[i+1] == ' ')
    {
	c = ' ';
	i++;    
	ESC_SPACES++;
    }
    //else if(cmd[i] == '\\' && cmd[i+1] == 't')
    //{
	//c = '\t';
	//i++;
	//ESC_SPACES++;
    //} 
    else
    {
	c = cmd[i];
    }
    // Store the char at arg[j]
    arg[j++] = c;
    // Check if arg has reached BUFFER_LEN, and allocate more mem if needed
    if(j == curr_len)
    {
	curr_len = j+max_len;
	arg = realloc(arg, curr_len);
    }
    i++;
  
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
int getargs(char *argv[]){
    
  unsigned int len_max = 128;
  unsigned int curr_len = 0;

  char *cmd = malloc(len_max * sizeof(char));
  curr_len = len_max;

  if(cmd != NULL)
  {
    int c = EOF;
    unsigned int j = 0;
    
    //Read user input until "Enter" or EOF
    while(( c = getchar() ) != '\n' && c != EOF)
    {
	cmd[j++] = (char)c;

	//If j reached the end of the current string, allocate more space
	if(j == curr_len)
	{
	  curr_len = j+len_max;
	  cmd = realloc(cmd, curr_len);
	}
    }

    cmd[j] = '\0';
  }	
  
  char *new_cmd = malloc(sizeof cmd);
  new_cmd = handle_escape_char(cmd);

  int i = 0;
  int arg_num = 0;

  //Parse raw string of command until NULL
  while(new_cmd[i] != '\0' && new_cmd[i] != '\n'){
    
    //Ignore extra spaces
    while (new_cmd[i] == ' '){
      i ++;
      if (new_cmd[i] == '\0' || new_cmd[i] == '\n')
        break;
    }
    //Break if end is reached
    if (new_cmd[i] == '\0' || new_cmd[i] == '\n')
      break;
    
    //Extract arg pointed to by i from cmd
    char *arg =get_argument(new_cmd, i);

    argv[arg_num] = arg;
    arg_num++;
    i += strlen(arg);
    i += ESC_SPACES;
    ESC_SPACES = 0;   
  }

  // Free unused cmd; args are now stored in argv
  free(cmd);
  cmd = NULL;

  argv[arg_num] = NULL;

  return arg_num;
}

/**
 * Free all arguments in argv
 */
void free_args(int argc, char *argv[]){
  int i;
  for (i = 0; i < argc; i++){
    free(argv[i]);
  }
}


/**
 * Take a valid command (as far as our shell is concerned) and apply
 * all the IO redirection.
 *
 * The given childargv is a malloced list of commands and arguments
 * which will be passed to execvp. So if any IO redirects are found,
 * they must be removed. This is done by putting a NULL into the array
 * at the end of the command meant to be passed to execvp.
 *
 * This is only ever called from the child process, so we don't need
 * to free deleted arguments. They will be cleared when this child
 * execs or exits.
 */
int parse_io_redirects(int childargc, char *childargv[]){
  // keep track of how many commands to remove from end
  int num_of_redirects = 0;
  int i;
  for(i = 0; i < childargc; i++){
    if(strcmp(childargv[i], "<") == 0){
      char *path = childargv[i+1];
      if (freopen(path, "r", stdin) == NULL)
	return -1;
      num_of_redirects++;
    }
    else if(strcmp(childargv[i], ">") == 0){
      char *path = childargv[i+1];
      if (freopen(path, "w", stdout) == NULL)
	return -1;
      num_of_redirects++;
    }
    else if(strcmp(childargv[i], "2>") == 0){
      char *path = childargv[i+1];
      if (freopen(path, "w", stderr) == NULL)
	return -1;
      num_of_redirects++;
    }
  }

  // Mark the end of the command, minus IO redirect commands (for
  // execvp)
  childargc -= (num_of_redirects * 2);
  childargv[childargc] = NULL;

  return 0;
}


/**
 * execute is where the shell process is forked, and the command is
 * executed. Within the child fork branch is also where the IO
 * redirection is parsed and performed.
 *
 * At this point childargv must have been parsed, escaped characters
 * put in place, and all checked for validity.
 */
void execute(int childargc, char *childargv[]){

  pid_t childpid;

  childpid = fork();
  if (childpid == -1){
    perror("FORK FAILED");
  }
  if (childpid == 0){
    // child process branch

    // delete background procces command before exec
    if (*childargv[childargc-1] == '&'){
      childargv[childargc-1] = NULL;
      childargc--;
    }
    
    // Make copies of the original file descriptors in case of exec
    // error
    int orig_stdin, orig_stdout, orig_stderr;
    orig_stdin = dup(0);
    orig_stdout = dup(1);
    orig_stderr = dup(2);

    if( parse_io_redirects(childargc, childargv) == -1){
      dup2(orig_stdin, 0);
      dup2(orig_stdout, 1);
      dup2(orig_stderr, 2);    
      printf("Error: Unable to open redirection file.\n");
      exit(1);
    }

    // Magic hands!
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
  // Parent process branch
  if (childpid > 0) 
  {  
    // If no background process flag, wait for child to finish
    if (*childargv[childargc-1] != '&')
    {
      waitpid(childpid, NULL, 0);
    }
    else
    {
      return;
    }
  }
  return;
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
    childargc = getargs(childargv);

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
