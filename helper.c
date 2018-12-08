#include "helper.h"
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

int
fork1(void)
{
  int pid;
  
  pid = fork();
  if(pid == -1)
    perror("fork");
  return pid;
}
