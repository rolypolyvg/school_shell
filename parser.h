#ifndef PARSER_H
#define PARSER_H

#include "cmd.h"

int gettoken(char **ps, char *es, char **q, char **eq);
int peek(char **ps, char *es, char *toks);
char*mkcopy(char *s, char *es);

struct cmd *parsecmd(char *s);
struct cmd *parseline(char**, char*);
struct cmd *parsepipe(char**, char*);
struct cmd *parseredirs(struct cmd *cmd, char **ps, char *es);
struct cmd *parseexec(char**, char*);

#endif
