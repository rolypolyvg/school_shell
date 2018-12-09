#include <stdlib.h>
#include <string.h>

#include "cmd.h"
#include "parser.h"
#include "helper.h"
#include "in_cmd.h"

int
main(void)
{
  static char buf[100];

  char *start, *st;

  // Read and run input commands.
  while(getcmd(buf, sizeof(buf)) >= 0){
  	// parse command by ; and run
  	st = buf;
	while (start = strtok(st, ";"), start != NULL){
		st = NULL;
		run_all_cmd(start);
	}
  }
  exit(0);
}
