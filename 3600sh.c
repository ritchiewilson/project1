/*
 * CS3600, Spring 2013
 * Project 1 Starter Code
 * (c) 2013 Alan Mislove
 *
 * Ritchie Wilson
 * Ryan Strassberger
 *
 */

#include "3600sh.h"

#define USE(x) (x) = (x)

int ESC_SPACES = 0;


int get_user_input(char *argv[]);
char *convert_escaped_chars(char *cmd);
char *get_argument(char *cmd, int i);
int test_if_valid_syntax(int argc, char *argv[]);
int valid_next_two_args(char *arg1, char *arg2);
int setup_io_redirects(int childargc, char *childargv[]);
void execute(int childargc, char *childargv[]);
void free_args(int childargc, char *childargv[]);


/**
 * Our shell. See README
 */
int main(int argc, char*argv[]) {
  // Code which sets stdout to be unbuffered
  // This is necessary for testing; do not change these lines
  USE(argc);
  USE(argv);
  setvbuf(stdout, NULL, _IONBF, 0); 
  
  char *username = getenv("USER");
  char hostname[128];
  gethostname(hostname, 128);

  int MAXARGS = 150;
  char *childargv[MAXARGS];
  int childargc = 0;

  // Main loop that reads a command and executes it
  while (1) {
    // Always free whatever values may still be in childargc
    free_args(childargc, childargv);

    // print current directoy
    char cur_dir[100];
    getcwd(cur_dir, 100);
    printf("%s@%s:%s> ", username, hostname, cur_dir);

    // Get User Input
    childargc = get_user_input(childargv);

    // From user the given user input, exit, print an error, or execute
    if ( childargc > 0 && strcmp(childargv[0], "exit") == 0){
      do_exit();
    }
    else if ( childargc == -1 ){
      printf("Error: Unrecognized escape sequence.\n");
    }
    else if ( childargc == -2 ){
      printf("Error: Invalid syntax.\n");
    }
    else{
      execute(childargc, childargv);
    }

    if (feof(stdin))
      do_exit();
  }

  return 0;
}


/**
 * 1) Gets and stores raw user input 2) Parses input to convert
 * escaped characters 3) Splits the result into an array of arguments
 * which are 4) stored into them in argv. 
 *
 * Returns the number of arguments in argv on success
 * Returns -1 for an invalid escape sequence error.
 * Returns -2 for an invalid syntax error
 */
int get_user_input(char *argv[]){
    
  // create a buffer for raw user input
  unsigned int curr_len = 128;
  char *cmd = calloc(curr_len, sizeof(char));

  if(cmd != NULL)
  {
    int c = EOF;
    unsigned int j = 0;
    
    //Read user input until "Enter" or EOF
    while(( c = getchar() ) != '\n' && c != EOF)
    {
	cmd[j++] = (char)c;

	// allocate more space if needed
	if(j == curr_len)
	{
	  curr_len += 128;
	  cmd = realloc(cmd, curr_len);
	}
    }

    cmd[j] = '\0';
  }	
  
  char *new_cmd;
  new_cmd = convert_escaped_chars(cmd);

  // raw user input is no longer needed
  free(cmd);

  // A NULL indicates a bad escpae sequence. Return error
  if (new_cmd == NULL)
    return -1;

  int i = 0;
  int arg_num = 0;

  // Convert command string into an array of arguments
  while(new_cmd[i] != '\0' && new_cmd[i] != '\n'){
    // Move through the command to the start of the next argument
    while (new_cmd[i] == ' '){
      i ++;
      if (new_cmd[i] == '\0' || new_cmd[i] == '\n')
        break;
    }
    if (new_cmd[i] == '\0' || new_cmd[i] == '\n')
      break;
    
    // Get the next argument, which is in new_cmd and starts at i
    char *arg =get_argument(new_cmd, i);

    argv[arg_num] = arg;
    arg_num++;

    // get_argument moved forward in the string the length of the
    // argument plus any escaped spaces it handled.
    i += strlen(arg);
    i += ESC_SPACES;
    ESC_SPACES = 0;   
  }
  // NULL indicates end of arguments for execvp
  argv[arg_num] = NULL;

  // args are now stored in argv, so we don't need the string
  free(new_cmd);


  if ( test_if_valid_syntax(arg_num, argv) == -1 )
    return -2;

  return arg_num;
}


/**
 * Takes the raw user input and converts escape sequences to their
 * intended values. This is the first step of input processing.
 *
 * Escaped spaces are not altered here, but are left as the two chars
 * '\' and ' '. This is because they are more easily handled later
 * when spliting this string into an array of arguments.
 *
 * Returns a malloced char* on success.
 *
 * Returns NULL pointer for invalid escape characters.
 */
char *convert_escaped_chars(char *cmd) {
  
  // Allocate 128 bytes for the new cmd
  int len = strlen(cmd) + 1;
  char *new_cmd = calloc(len, sizeof(char));

  // Indices for parsing cmd (i) & new_cmd (j)
  unsigned int i = 0;
  unsigned int j = 0;

  // Escaped char/regular char to be added to new_cmd 
  char c;

  // Parse raw command string and look for escape characters
  while(cmd[i] != '\0'){ 
    c = cmd[i];
    if(c == '\t') // just treat tabs as spaces
      c = ' ';
    // If escape character is found
    if(c == '\\'){
      switch(cmd[i+1]){
	case '\\':
	  c = '\\';
	  i++;
	  break;
	case ' ':
	  c = '\\';  // need to keep in this char for when splitting cmd into arg array
	  break;
	case 't':
	  c = '\t';
	  i++;
	  break;
	case '&':
	  c = '&';
	  i++;
	  break;
	// If it was none of these, return NULL (error)
        default:
	  free(new_cmd);
	  return NULL;
	  break;
      }
    }
    // Add c to new_cmd
    new_cmd[j++] = c;
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
  
  unsigned int curr_len = 512;		// Size for the receiving array
  char *arg = malloc(curr_len);       // Allocating 128bytes for receiving array
  
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
    else
	c = cmd[i];
    // Store the string in the return array
    arg[j++] = c;
    // Check if arg has reached the end of allocated space
    if(j == curr_len)
    {
	curr_len += 512;
	arg = realloc(arg, curr_len);
    }
    i++;
  
  }
  
  if( arg[j-1] == '\n')
    j--;
  arg[j] = '\0';
    
  return arg;
}


/**
 * Tests the given array of arguments to see if this should throw an
 * "Invalid syntax" error.
 *
 * The only invalid commands we are concerned with are 1) arguments
 * after an '&', 2) arguments after IO redirections other than '&', 3)
 * IO redirects without a corresponding file argument and 4) setting
 * any redirect more that once.
 *
 * Returns 0 for valid, -1 otherwise
 */
int test_if_valid_syntax(int argc, char *argv[]){

  // if any of these aguments is found twice, invalid command
  int in = 0; // stdin (<) 
  int out = 0; // stdout (>) 
  int err = 0; // stderr (2>)

  int i;
  for (i = 0; i < argc; i++){
    char *arg = argv[i];
    if ( strcmp(arg, "&") == 0){
      // & must be the last argument
      if( argv[i+1] == NULL )
	return 0;
      return -1;
    }
    if ( strcmp(arg, "<") == 0){
      if(in)
	return -1;  // another < command was already found
      if ( valid_next_two_args(argv[i+1], argv[i+2]) == -1)
	return -1;
      in = 1;
    }
    if ( strcmp(arg, ">") == 0){
      if(out)
	return -1;  // another > command was already found
      if ( valid_next_two_args(argv[i+1], argv[i+2]) == -1)
	return -1;
      out = 1;
    }
    if ( strcmp(arg, "2>") == 0){
      if(err)
	return -1;  // another 2> command was already found
      if ( valid_next_two_args(argv[i+1], argv[i+2]) == -1)
	return -1;
      out = 1;
    }
  }
  return 0;
}


/**
 * This checks the values of the next two arguments after an IO
 * redirection to test if the syntax is valid. To be valid, the next
 * argument CANNOT be NULL, &, <, >, or 2>. Additionally the argument
 * after that MUST be NULL, &, <, >, or 2>.
 *
 * Returns 0 if valid, -1 otherwise.
 */
int valid_next_two_args(char *arg1, char *arg2){
  // Test if the first argument is invalid
  if ( ( arg1 == NULL ) ||
       ( strcmp(arg1, "&") == 0 ) || ( strcmp(arg1, "<") == 0 ) ||
       ( strcmp(arg1, ">") == 0 ) || ( strcmp(arg1, "2>") == 0 ))
    return -1;

  // test if the second argument is valid
  if ( (arg2 == NULL ) || 
       ( strcmp(arg2, "&") == 0 ) || ( strcmp(arg2, "<") == 0 ) ||
       ( strcmp(arg2, ">") == 0 ) || ( strcmp(arg2, "2>") == 0 ) )
    return 0;

  // otherwise return 'invalid'
  return -1;
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
 *
 * Returns 0 on success
 * Returns -1 on any error opening a file
 */
int setup_io_redirects(int childargc, char *childargv[]){
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

    if( setup_io_redirects(childargc, childargv) == -1){
      dup2(orig_stdin, 0);
      dup2(orig_stdout, 1);
      dup2(orig_stderr, 2);    
      printf("Error: Unable to open redirection file.\n");
      exit(1);
    }

    // Magic hands!
    execvp(childargv[0], childargv);
    
    // if anything executes past this point, execvp has failed. Restore
    // the original IOs, print an Error and exit
    dup2(orig_stdin, 0);
    dup2(orig_stdout, 1);
    dup2(orig_stderr, 2);    

    if (errno == EACCES)
      printf("Error: Permission denied.\n");
    else
      printf("Error: Command not found.\n");
    exit(1);
  }
  // If no background process flag, wait for child to finish
  if (*childargv[childargc-1] != '&')
    waitpid(childpid, NULL, 0);
  return;
}

/**
 * Before each user command is executed, free the malloced arguments
 * from the previous user command.
 */
void free_args(int childargc, char *childargv[]){
  int i;
  for (i = 0; i < childargc; i++){
    free(childargv[i]);
  }
}

// Function which exits, printing the necessary message
//
void do_exit() {
  printf("So long and thanks for all the fish!\n");
  exit(0);
}
