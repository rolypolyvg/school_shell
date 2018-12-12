#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <sys/types.h>
#include "cmd.h"

#define NONE	(0)
#define RUNNING (1)
#define END     (2)
#define DONE    (3)

struct bg{
	struct bg*next;
	char *cmd;
	pid_t pid;
	int order;
	int state;
};

struct bglist{
	struct bg*start, *end;
	int count;
};

struct bg* new_bg(struct cmd *cmd, pid_t pid);
void free_bg(struct bg*fbg);

void init_bglist(struct bglist* bgl);
void free_bglist(struct bglist* bgl);
void add_bglist(struct bglist*bgl, struct bg*add);
int mark_end_bglist(struct bglist *bgl, pid_t pid);
void clean_bglist(struct bglist *bgl);
void print_bg_state(struct bg*cur, int total, int index);
#endif
