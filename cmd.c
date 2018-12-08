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
	int redir_fd;	// file descripter for I/O redirection
	int pipe_fd[2];	// file descripter for pipe
	int pipe_child_pid, pipe_child_status;	// for pipe command waitpid

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
    // Your code here ...
		execvp(ecmd->argv[0], ecmd->argv);
    break;

  case '>':
  case '<':
    rcmd = (struct redircmd*)cmd;
    // Your code here ...
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
    // Your code here ...
		if (pipe(pipe_fd) == -1){	// make pipe
			perror("pipe");
			exit(-1);
		}
		switch(pipe_child_pid = fork1()){
			case 0:	// child
				close(pipe_fd[0]);	// close read end of pipe
				dup2(pipe_fd[1], 1);	// redirect standard output to write end of pipe
				runcmd(pcmd->left);
				break;
			default:	// parent
				close(pipe_fd[1]);	// close write end of pipe
				dup2(pipe_fd[0], 0);	// redirect standard input to read end of pipe
				waitpid(pipe_child_pid, &pipe_child_status, 0);	// wait for the child to finish executing its command
				runcmd(pcmd->right);
				break;
		}
    break;
  }    
  exit(1);
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

