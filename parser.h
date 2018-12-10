#ifndef PARSER_H
#define PARSER_H

#include "cmd.h"

extern char whitespace[];

int gettoken(char **ps, char *es, char **q, char **eq);
char *strip_leading_space(char *s, char *es);
int peek(char **ps, char *es, char *toks);
char*mkcopy(char *s, char *es);

struct cmd *parsecmd(char *s);
struct cmd *parselist(char**, char*);
struct cmd *parsepipe(char**, char*);
struct cmd *parsesinglecmd(char**, char*);
struct cmd *parseparenth(char**, char*);
struct cmd *parseredirs(struct cmd *cmd, char **ps, char *es);
struct cmd *parseexec(char**, char*);

#endif
