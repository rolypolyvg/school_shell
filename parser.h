#ifndef PARSER_H
#define PARSER_H

#include "cmd.h"

extern char whitespace[];

int gettoken(char **ps, char *es, char **q, char **eq);

// strip leading whitespace starting from s
// and stops if encounter non-whitespace or es is reached
// return : char * to where it stopped
char *strip_leading_space(char *s, char *es);
int peek(char **ps, char *es, char *toks);
char*mkcopy(char *s, char *es);

// how each command is structured
// redir cmd = {< file} or {> file}
// exec cmd = consist of redir cmd and arguments(not having any |&;())
// single cmd = {exec cmd} or {subshell}
// pipe cmd = {single cmd | single cmd} or {single cmd}
// list cmd = {pipe cmd[; or &]} or {pipe cmd[; or &] list cmd}
// subshell = (list cmd)
struct cmd *parsecmd(char *s);			// parse command from string s
struct cmd *parselist(char**, char*);	// parse list cmd
struct cmd *parsepipe(char**, char*);	// parse pipe cmd
struct cmd *parsesinglecmd(char**, char*);	// parse single cmd 
struct cmd *parseparenth(char**, char*);	// parse subshell
struct cmd *parseredirs(struct cmd *cmd, char **ps, char *es); // parse redir cmd
struct cmd *parseexec(char**, char*);						   // parse exec cmd

#endif
