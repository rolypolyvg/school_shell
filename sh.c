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

  char *start, *end, *next;

  // Read and run input commands.
  while(getcmd(buf, sizeof(buf)) >= 0){
  	// parse command by ; and run

	next = buf;
	while (next){
		start = next;
		end = strchr(start, ';');
		if (end == NULL)
			next = NULL;
		else{
			next = end + 1;
			*end = '\0';
		}

		run_all_cmd(start);
	}
  }
  exit(0);
}
