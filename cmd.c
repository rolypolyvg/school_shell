#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "cmd.h"
#include "sh.h"
#include "helper.h"
#include "background.h"

extern struct sh shell;

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
  struct hnode *cur;
  struct hlist *tmp;

  if(cmd == 0)
    exit(0);
  
  switch(cmd->type){
  default:
    fprintf(stderr, "unknown runcmd\n");
    exit(-1);

  case ' ':
    ecmd = (struct execcmd*)cmd;
		// catch internal command
		if(!strcmp(ecmd->argv[0], "cd")){
			if (chdir(ecmd->argv[1]) < 0)
				fprintf(stderr, "cannot cd to %s\n", (ecmd->argv[1]));
		}
		else if(!strcmp(ecmd->argv[0], "history")){
      tmp = &shell.hl;
      while(cur=traverse_hl(tmp), cur){
        tmp = NULL;
        printf("%d: %s\n", cur->nb, cur->cmd);
      }
		}else if(execvp(ecmd->argv[0], ecmd->argv) == -1)
			perror("exec");
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
		if((left_pid = myfork()) == 0){	// left cmd
			close(pipe_fd[0]);	// close read end of pipe
			dup2(pipe_fd[1], 1);	// redirect standard output to write end of pipe
			runcmd(pcmd->left);
		}
		else if((right_pid = myfork()) == 0){	// right cmd
			close(pipe_fd[1]);	// close write end of pipe
			dup2(pipe_fd[0], 0);	// redirect standard input to read end of pipe
			runcmd(pcmd->right);
		}
		else{
			// wait for left & right
      close(pipe_fd[0]);
      close(pipe_fd[1]);
			waitpid(left_pid, &left_child_status, 0);
			waitpid(right_pid, &right_child_status, 0);
		}
    break;

	case '&':
	case ';':
  case '(':
		handle_cmd(cmd);
		break;
  }    
  
  return;
}

/* handle commands before actually runnning them */
void handle_cmd(struct cmd* cmd){
	int r;
  pid_t pid;
	struct execcmd *ecmd;
	struct semicmd *semcmd;
	struct parenthcmd *parcmd;
	struct ampersandcmd *ampcmd;
  struct bg*add;

	if(cmd == 0)
    return;

	switch(cmd->type){
	default:
    fprintf(stderr, "unknown handle_cmd\n");
    exit(-1);

	case ' ':
		ecmd = (struct execcmd *)cmd;
		// internal command
    if(!strcmp(ecmd->argv[0], "cd") || !strcmp(ecmd->argv[0], "history")){
        runcmd(cmd);
    }else{
  		// regular exec
  		if(myfork() == 0)
  			runcmd(cmd);
  		wait(&r);
    }
		break;

	case '>':
  case '<':
	case '|':
		if(myfork() == 0){
			runcmd(cmd);
      exit(1);
		}
		wait(&r);
		break;

	case ';':	// handle the list of commands in the current shell
		semcmd = (struct semicmd *)cmd;
		handle_cmd(semcmd->cur);
		handle_cmd(semcmd->next);
		break;

	case '&':
		ampcmd = (struct ampersandcmd *)cmd;

		if((pid = myfork()) == 0){
			handle_cmd(ampcmd->cur);
			exit(1);
		}else{
      add = new_bg(ampcmd->cur, pid);
      add_bglist(&shell.bgl, add);
      printf("[%d] %d\n", add->order, add->pid);
    // error after here
      //(ls>a.txt)|cd .. > d.txt&
		  handle_cmd(ampcmd->next);
    }
		// no wait! (let the command run in the background)
		break;

	case '(':
		parcmd = (struct parenthcmd *)cmd;
		if(myfork() == 0){
			handle_cmd(parcmd->cmd);	// handle the command inside the parentheses in a subshell
			exit(1);
		}
		wait(&r);
		break;
	}
	
  return;
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

void remake_cmd(int *index, char *buf, struct cmd *cmd){
  struct execcmd *ecmd;
  struct redircmd *rcmd;
  struct pipecmd *pcmd;
  struct semicmd *scmd;
  struct ampersandcmd *acmd;
  struct parenthcmd *ptcmd;
  char **s;

  if (cmd == NULL)
    return;
  else if (cmd->type == ' '){
    ecmd = (struct execcmd*) cmd;
    for (s=ecmd->argv; *s; s++){
      strcpy(buf+*index, *s);
      *index += strlen(*s);
      buf[(*index)++] = ' ';
    }

    return;
  }else if (cmd->type == '<' || cmd->type == '>'){
    rcmd = (struct redircmd *) cmd;

    remake_cmd(index, buf, rcmd->cmd);
    buf[(*index)++] = cmd->type;
    strcpy(buf+*index, rcmd->file);
    *index += strlen(rcmd->file);

    return;
  }else if (cmd->type == '|'){
    pcmd = (struct pipecmd *) cmd;
    remake_cmd(index, buf, pcmd->left);
    buf[(*index)++] = (char)cmd->type;
    remake_cmd(index, buf, pcmd->right);
    return;
  }else if (cmd->type == ';'){
    scmd = (struct semicmd *) cmd;
    remake_cmd(index, buf, scmd->cur);
    buf[(*index)++] = cmd->type;
    remake_cmd(index, buf, scmd->next);
    return;
  }else if (cmd->type == '&'){
    acmd = (struct ampersandcmd *) cmd;
    remake_cmd(index, buf, acmd->cur);
    buf[(*index)++] = cmd->type;
    remake_cmd(index, buf, acmd->next);
    return;
  }else if (cmd->type == '('){
    ptcmd = (struct parenthcmd *) cmd;
    buf[(*index)++] = '(';
    remake_cmd(index, buf, ptcmd->cmd);
    buf[(*index)++] = ')';
  }else{
    printf("wrong\n");
    exit(1);
  }

  return;
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
    return;
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

