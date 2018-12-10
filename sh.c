#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "sh.h"
#include "cmd.h"
#include "parser.h"
#include "helper.h"
#include "in_cmd.h"
#include "history.h"

void print_cmd(struct cmd* cmd);

int
main(void)
{
	struct sh shell;
  	char *start, *end, *next, *tmp;
  	struct cmd *cmd;

	init_hl(&shell.hl, 10);

  // Read and run input commands.
  while(getcmd(shell.buf, sizeof(shell.buf)) >= 0){
  	// remove newline
  	tmp = strchr(shell.buf, '\n');
	if (tmp)
		*tmp = '\0';

  	add_new_hl(&shell.hl, shell.buf);

  	// parse command by ; and run
	cmd = parsecmd(shell.buf);

	print_cmd(cmd);

	free_cmd(cmd);
  }

  clean_hl(&shell.hl);
  exit(0);
}

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

