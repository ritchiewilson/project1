/**
 * This is just a program that takes a while (more than 1 second) to
 * execute, then it prints a result. Helpful for testing background
 * processes in the shell.
 */

#include <stdio.h>

int fib(int i){
  if (i == 0)
    return 0;
  if (i == 1)
    return 1;

  int f = fib(i-1) + fib(i-2);
  return f;
  
}


int main(){
  
  int i = fib(41);
  printf("Fib is %d\n", i);
  return 0;
}
