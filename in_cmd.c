#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "in_cmd.h"
#include "parser.h"
#include "helper.h"

void run_all_cmd(char *buf){
	char *start = strip_leading_space(buf, NULL);

	if (strncmp(start, "cd", 2) == 0){
		run_cd(start+2);
	}else
		run_normal_cmd(start);

	return;
}

void run_normal_cmd(char *buf){
	int r;

	if (fork1() == 0)
		runcmd(parsecmd(buf));

	wait(&r);

	return;
}

// only cd to first argument
void run_cd(char *buf){
	char *start = strip_leading_space(buf, NULL);
	char *end = start;

	while (*end != '\0' && !strchr(whitespace, *end))
		end++;

	*end = '\0';

	if (chdir(start) < 0)
		fprintf(stderr, "cannot cd to %s\n", start);

	return;
}

