#include <stdlib.h>
#include <string.h>

#include "sh.h"
#include "cmd.h"
#include "parser.h"
#include "helper.h"
#include "in_cmd.h"
#include "history.h"

int
main(void)
{
	struct sh shell;
  char *start, *end, *next, *tmp;

	init_hl(&shell.hl, 10);

  // Read and run input commands.
  while(getcmd(shell.buf, sizeof(shell.buf)) >= 0){
  	// remove newline
  	tmp = strchr(shell.buf, '\n');
	if (tmp)
		*tmp = '\0';

  	add_new_hl(&shell.hl, shell.buf);

  	// parse command by ; and run
	next = shell.buf;
	while (next){
		start = next;
		end = strchr(start, ';');
		if (end == NULL)
			next = NULL;
		else{
			next = end + 1;
			*end = '\0';
		}

		run_all_cmd(start, &shell);
	}
  }

  clean_hl(&shell.hl);
  exit(0);
}
