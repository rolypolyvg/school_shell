#include "helper.h"
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>

int
fork1(void)
{
  int pid;
  
  pid = fork();
  if(pid == -1)
    perror("fork");

	if(pid == 0){
		// unblock all signals
		unblock_signals();
	}
  return pid;
}

void unblock_signals(void){
	sigset_t ss;
	sigfillset(&ss);
	sigprocmask(SIG_UNBLOCK, &ss, (sigset_t*)NULL);
}
