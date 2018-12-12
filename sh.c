#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "sh.h"
#include "cmd.h"
#include "parser.h"
#include "helper.h"
#include "history.h"
#include "background.h"

void print_cmd(struct cmd* cmd);
void handle_cmd(struct cmd* cmd);

struct sh shell;

int
main(void)
{
  	char *start, *end, *next, *tmp;
  	struct cmd *cmd;
  	int r;
  	pid_t pid;
  	int is_changed;

  	init_bglist(&shell.bgl);
	init_hl(&shell.hl, 10);
	init_block_signals();	// blocking signals

  // Read and run input commands.
  while(getcmd(shell.buf, sizeof(shell.buf)) >= 0){
  	// remove newline
  	tmp = strchr(shell.buf, '\n');
	if (tmp)
		*tmp = '\0';

  	add_new_hl(&shell.hl, shell.buf);

  	// parse command by ; and run
	cmd = parsecmd(shell.buf);

#ifdef DEBUG
	print_cmd(cmd);
#endif
	handle_cmd(cmd);

	free_cmd(cmd);

	// wait builtin command
	
	is_changed = 0;
	while (pid = waitpid(-1, &r, WNOHANG), pid > 0){
		mark_end_bglist(&shell.bgl, pid);
		is_changed = 1;
	}

	if (is_changed)
		clean_bglist(&shell.bgl);
  }

  clean_hl(&shell.hl);
  exit(0);
		
}

// block signals when starting shell
void init_block_signals(void){
	sigset_t ss;	// signal set to block
	
	sigemptyset(&ss);
	sigaddset(&ss, SIGINT);
	sigaddset(&ss, SIGQUIT);
	sigprocmask(SIG_BLOCK, &ss, (sigset_t*)NULL);
}

/* for debugging */
void print_cmd(struct cmd* cmd){
	struct execcmd *ecmd;
	struct redircmd *rcmd;
	struct pipecmd *pcmd;
	struct semicmd *scmd;
	struct ampersandcmd *acmd;
	struct parenthcmd *ptcmd;
	char **s;

	if (cmd == NULL)
		printf("NULL\n");
	else if (cmd->type == ' '){
		ecmd = (struct execcmd*) cmd;
		printf("execcmd: ");
		for (s = ecmd->argv; *s != 0; s++)
			printf("%s ", *s);
		printf("\n");
	}else if (cmd->type == '<' || cmd->type == '>'){
		rcmd = (struct redircmd *) cmd;
		printf("redir: ");
		printf("%c %s\n", rcmd->type, rcmd->file);
		print_cmd(rcmd->cmd);
	}else if (cmd->type == '|'){
		pcmd = (struct pipecmd *) cmd;
		printf("pipe\n");
		printf("left\n");
		print_cmd(pcmd->left);
		printf("right\n");
		print_cmd(pcmd->right);
	}else if (cmd->type == ';'){
		scmd = (struct semicmd *) cmd;
		printf("semicolon\n");
		printf("cur\n");
		print_cmd(scmd->cur);
		printf("next\n");
		print_cmd(scmd->next);
	}else if (cmd->type == '&'){
		acmd = (struct ampersandcmd *) cmd;
		printf("ampersand\n");
		printf("cur\n");
		print_cmd(acmd->cur);
		printf("next\n");
		print_cmd(acmd->next);
	}else if (cmd->type == '('){
		ptcmd = (struct parenthcmd *) cmd;
		printf("parenthesis\n");
		print_cmd(ptcmd->cmd);
	}else{
		printf("wrong\n");
		exit(1);
	}

	return;
}

