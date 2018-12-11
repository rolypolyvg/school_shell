#ifndef CMD_H
#define CMD_H

#define MAXARGS 10

// All commands have at least a type. Have looked at the type, the code
// typically casts the *cmd to some specific cmd type.
struct cmd {
  int type;          //  ' ' (exec), | (pipe), '<' or '>' for redirection
};                   //  ';' (semicolon) run more command, '&' (ampersand) run background
                     //  '(' (parenthsis) subshell

struct execcmd {
  int type;              // ' '
  char *argv[MAXARGS];   // arguments to the command to be exec-ed
};

struct redircmd {
  int type;          // < or > 
  struct cmd *cmd;   // the command to be run (e.g., an execcmd)
  char *file;        // the input/output file
  int mode;          // the mode to open the file with
  int fd;            // the file descriptor number to use for the file
};

struct pipecmd {
  int type;          // |
  struct cmd *left;  // left side of pipe
  struct cmd *right; // right side of pipe
};

struct semicmd{
  int type;         // ;
  struct cmd *cur;  // command left of ;
  struct cmd *next; // command right of ; may be NULL if there is no command
};

struct ampersandcmd{
  int type;         // &
  struct cmd *cur;  // command left of &
  struct cmd *next; // command right of & may be NULL if there is no command
};

struct parenthcmd{
  int type;         // (
  struct cmd *cmd;  // command inside subshell
};

void runcmd(struct cmd *cmd);
void handle_cmd(struct cmd* cmd);
int getcmd(char *buf, int nbuf);

// make commands
struct cmd* execcmd(void);
struct cmd* redircmd(struct cmd *subcmd, char *file, int type);
struct cmd* pipecmd(struct cmd *left, struct cmd *right);
struct cmd* semicmd(struct cmd *cur, struct cmd *next);
struct cmd* ampersandcmd(struct cmd *cur, struct cmd*next);
struct cmd* parenthcmd(struct cmd *cmd);

// free command recursive starting from cmd
void remake_cmd(int *index, char *buf, struct cmd *cmd);
void free_cmd(struct cmd *cmd);

#endif
