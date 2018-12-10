#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "parser.h"
#include "cmd.h"

char whitespace[] = " \t\r\n\v";
char symbols[] = "|<>()&;";

int
gettoken(char **ps, char *es, char **q, char **eq)
{
  char *s;
  int ret;
  
  s = strip_leading_space(*ps, es);
  if(q)
    *q = s;
  ret = *s;

  if (*s == 0)
      ;
  else if (strchr(symbols, *s))
    s++;
  else{
    ret = 'a';
    while(s < es && !strchr(whitespace, *s) && !strchr(symbols, *s))
      s++;
  }
 
  if(eq)
    *eq = s;
  
  *ps = strip_leading_space(s, es);
  return ret;
}

// remove leading space and return pointer to first none space
// or es is reached 
// if es is NULL strip until end of string
char *strip_leading_space(char *s, char *es){
	if (es == NULL)
		while (*s != 0 && strchr(whitespace, *s))
			s++;
	else
		while (s < es && strchr(whitespace, *s))
			s++;

	return s;
}

int
peek(char **ps, char *es, char *toks)
{
  char *s;
  
  s = strip_leading_space(*ps, es);
  *ps = s;
  return *s && strchr(toks, *s);
}


// make a copy of the characters in the input buffer, starting from s through es.
// null-terminate the copy to make it a string.
char 
*mkcopy(char *s, char *es)
{
  int n = es - s;
  char *c = malloc(n+1);

  // current parser is done by shell
  // this will kill shell change it
  assert(c);
  strncpy(c, s, n);
  c[n] = 0;
  return c;
}

struct cmd*
parsecmd(char *s)
{
  char *es;
  struct cmd *cmd;

  es = s + strlen(s);
  cmd = parselist(&s, es);
  peek(&s, es, "");
  if (s != es){
    fprintf(stderr, "leftovers: %s\n", s);
    free_cmd(cmd);
    return NULL;
  }

  return cmd;
}

struct cmd* parselist(char **ps, char *es){
  struct cmd *cmd, *tmp;

  cmd = parsepipe(ps, es);
  if (!cmd)
    return NULL;

  if (peek(ps, es, "&")){
    gettoken(ps, es, 0, 0);
    tmp = parselist(ps, es);
    cmd = ampersandcmd(cmd, tmp);
  }else if (peek(ps, es, ";")){
    gettoken(ps, es, 0, 0);
    tmp = parselist(ps, es);
    cmd = semicmd(cmd, tmp);
  }

  return cmd;
}

struct cmd*
parsepipe(char **ps, char *es)
{
  struct cmd *cmd, *tmp;

  cmd = parsesinglecmd(ps, es);
  if (!cmd)
    return NULL;

  if(peek(ps, es, "|")){
    gettoken(ps, es, 0, 0);
    tmp = parsepipe(ps, es);
    if (!tmp)
      return NULL;

    cmd = pipecmd(cmd, tmp);
  }
  return cmd;
}

struct cmd* parsesinglecmd(char **ps, char *es){
  struct cmd *cmd;

  if (peek(ps, es, "(")){
    gettoken(ps, es, 0, 0);
    cmd = parseparenth(ps, es);
    cmd = parseredirs(cmd, ps, es);
  }else
    cmd = parseexec(ps, es);

  return cmd;
}

struct cmd* parseparenth(char **ps, char *es){
  struct cmd *cmd;

  cmd = parselist(ps, es);
  if (peek(ps, es, ")")){
    gettoken(ps, es, 0, 0);
    cmd = parenthcmd(cmd);
  }else{
    fprintf(stderr, "missing )\n");
    free_cmd(cmd);
    return NULL;
  }

  return cmd;
}

struct cmd*
parseredirs(struct cmd *cmd, char **ps, char *es)
{
  int tok;
  char *q, *eq;

  while(peek(ps, es, "<>")){
    tok = gettoken(ps, es, 0, 0);
    if(gettoken(ps, es, &q, &eq) != 'a') {
      fprintf(stderr, "missing file for redirection\n");
      free_cmd(cmd);
      return NULL;
    }
    switch(tok){
    case '<':
      cmd = redircmd(cmd, mkcopy(q, eq), '<');
      break;
    case '>':
      cmd = redircmd(cmd, mkcopy(q, eq), '>');
      break;
    }
  }
  return cmd;
}

struct cmd*
parseexec(char **ps, char *es)
{
  char *q, *eq;
  int tok, argc;
  struct execcmd *cmd;
  struct cmd *ret;
  
  ret = execcmd();
  cmd = (struct execcmd*)ret;

  argc = 0;
  ret = parseredirs(ret, ps, es);
  while(!peek(ps, es, "|;&)")){
    if((tok=gettoken(ps, es, &q, &eq)) == 0)
      break;
    if(tok != 'a') {
      fprintf(stderr, "syntax error\n");
      free_cmd(ret);
      return NULL;
    }
    cmd->argv[argc] = mkcopy(q, eq);
    argc++;
    if(argc >= MAXARGS) {
      fprintf(stderr, "too many args\n");
      free_cmd(ret);
      return NULL;
    }
    ret = parseredirs(ret, ps, es);
  }
  cmd->argv[argc] = 0;

  if (ret->type == ' ' && argc == 0){
    free_cmd(ret);
    return NULL;
  }

  return ret;
}
