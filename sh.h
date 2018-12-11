#ifndef SH_H
#define SH_H

#include <sys/types.h>
#include "history.h"

struct sh{
	char buf[100];
	struct hlist hl;

	char bg[100][200];
	pid_t bgpid[100];
	int cnt;
};

#endif
