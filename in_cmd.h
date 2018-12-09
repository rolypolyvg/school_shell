#ifndef IN_CMD_H
#define IN_CMD_H

#include "sh.h"

void run_all_cmd(char*buf, struct sh *shell);
void run_normal_cmd(char *buf);
void run_cd(char *buf);
void run_history(struct sh*shell);

#endif
