#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "cmd.h"
#include "helper.h"

// Execute cmd.  Never returns.
void
runcmd(struct cmd *cmd)
{
  int p[2], r;
  struct execcmd *ecmd;
  struct pipecmd *pcmd;
  struct redircmd *rcmd;
	struct ampersandcmd *ampcmd;
	int redir_fd;	// file descripter for I/O redirection
	int pipe_fd[2];	// file descripter for pipe
	int left_pid, left_child_status, right_pid, right_child_status;	// for pipe command waitpid

  if(cmd == 0)
    exit(0);
  
  switch(cmd->type){
  default:
    fprintf(stderr, "unknown runcmd\n");
    exit(-1);

  case ' ':
    ecmd = (struct execcmd*)cmd;
    if(ecmd->argv[0] == 0)
      exit(0);
		// catch internal command

		execvp(ecmd->argv[0], ecmd->argv);
    break;

  case '>':
  case '<':
    rcmd = (struct redircmd*)cmd;
		redir_fd = open(rcmd->file, rcmd->mode, 0644);	// open file for redirecting standard I/O
		if (redir_fd == -1){	// file open error
			perror("open");
			exit(-1);
		}
		dup2(redir_fd, rcmd->fd);	// redirect standard I/O
    runcmd(rcmd->cmd);
    break;

  case '|':
    pcmd = (struct pipecmd*)cmd;
		if (pipe(pipe_fd) == -1){	// make pipe
			perror("pipe");
			exit(-1);
		}
		if((left_pid = fork1()) == 0){	// left cmd
			close(pipe_fd[0]);	// close read end of pipe
			dup2(pipe_fd[1], 1);	// redirect standard output to write end of pipe
			handle_cmd(pcmd->left);
		}
		else if((right_pid = fork1()) == 0){	// right cmd
			close(pipe_fd[1]);	// close write end of pipe
			dup2(pipe_fd[0], 0);	// redirect standard input to read end of pipe
			handle_cmd(pcmd->right);
		}
		else{
			// wait for left & right
			waitpid(left_pid, &left_child_status, 0);
			waitpid(right_pid, &right_child_status, 0);
		}
    break;

	case '&':
	case ';':
		handle_cmd(cmd);
		break;
  }    
  exit(1);
}

/* handle commands before actually runnning them */
void handle_cmd(struct cmd* cmd){
	int r;
	struct execcmd *ecmd;
	struct semicmd *semcmd;
	struct parenthcmd *parcmd;
	struct ampersandcmd *ampcmd;

	switch(cmd->type){
	default:
    fprintf(stderr, "unknown handle_cmd\n");
    exit(-1);

	case ' ':
		ecmd = (struct execcmd *)cmd;
		// internal command
		
		// regular exec
		if(fork1() == 0)
			runcmd(cmd);
		wait(&r);
		break;

	case '>':
  case '<':
	case '|':
		if(fork1() == 0)
			runcmd(cmd);
		wait(&r);
		break;

	case ';':	// handle the list of commands in the current shell
		semcmd = (struct semicmd *)cmd;
		handle_cmd(semcmd->cur);
		handle_cmd(semcmd->next);
		break;

	case '&':
		ampcmd = (struct ampersandcmd *)cmd;
		if(fork1() == 0)
			handle_cmd(ampcmd->cur);
		handle_cmd(ampcmd->next);
		// no wait! (let the command run in the background)
		break;

	case '(':
		parcmd = (struct parenthcmd *)cmd;
		if(fork1() == 0)
			handle_cmd(parcmd->cmd);	// handle the command inside the parentheses in a subshell
		wait(&r);
		break;
	}
}

int
getcmd(char *buf, int nbuf)
{
  
  if (isatty(fileno(stdin)))
    fprintf(stdout, "$ ");
  memset(buf, 0, nbuf);
  fgets(buf, nbuf, stdin);
  if(buf[0] == 0) // EOF
    return -1;
  return 0;
}

struct cmd*
execcmd(void)
{
  struct execcmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = ' ';
  return (struct cmd*)cmd;
}

struct cmd*
redircmd(struct cmd *subcmd, char *file, int type)
{
  struct redircmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = type;
  cmd->cmd = subcmd;
  cmd->file = file;
  cmd->mode = (type == '<') ?  O_RDONLY : O_WRONLY|O_CREAT|O_TRUNC;
  cmd->fd = (type == '<') ? 0 : 1;
  return (struct cmd*)cmd;
}

struct cmd*
pipecmd(struct cmd *left, struct cmd *right)
{
  struct pipecmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = '|';
  cmd->left = left;
  cmd->right = right;
  return (struct cmd*)cmd;
}

struct cmd* semicmd(struct cmd *cur, struct cmd *next){
  struct semicmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = ';';
  cmd->cur = cur;
  cmd->next = next;
  return (struct cmd*)cmd;
}


struct cmd* ampersandcmd(struct cmd *cur, struct cmd*next){
  struct ampersandcmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = '&';
  cmd->cur = cur;
  cmd->next = next;
  return (struct cmd*)cmd;
}

struct cmd* parenthcmd(struct cmd *cmd){
  struct parenthcmd *rcmd;

  rcmd = malloc(sizeof(*rcmd));
  memset(rcmd, 0, sizeof(*rcmd));
  rcmd->type = '(';
  rcmd->cmd = cmd;
  return (struct cmd*) rcmd;
}

void free_cmd(struct cmd *cmd){
  struct execcmd *ecmd;
  struct redircmd *rcmd;
  struct pipecmd *pcmd;
  struct semicmd *scmd;
  struct ampersandcmd *acmd;
  struct parenthcmd *ptcmd;
  char **s;

  if (cmd == NULL)
    ;
  else if (cmd->type == ' '){
    ecmd = (struct execcmd*) cmd;
    for (s=ecmd->argv; *s; s++)
      free(*s);

    free(ecmd);
  }else if (cmd->type == '<' || cmd->type == '>'){
    rcmd = (struct redircmd *) cmd;
    free_cmd(rcmd->cmd);
    free(rcmd->file);
    free(rcmd);
  }else if (cmd->type == '|'){
    pcmd = (struct pipecmd *) cmd;
    free_cmd(pcmd->left);
    free_cmd(pcmd->right);
    free(pcmd);
  }else if (cmd->type == ';'){
    scmd = (struct semicmd *) cmd;
    free_cmd(scmd->cur);
    free_cmd(scmd->next);
    free(scmd);
  }else if (cmd->type == '&'){
    acmd = (struct ampersandcmd *) cmd;
    free_cmd(acmd->cur);
    free_cmd(acmd->next);
    free(acmd);
  }else if (cmd->type == '('){
    ptcmd = (struct parenthcmd *) cmd;
    free_cmd(ptcmd->cmd);
    free(ptcmd);
  }else{
    printf("wrong\n");
    exit(1);
  }

  return;
}

