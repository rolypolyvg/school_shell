#ifndef SH_H
#define SH_H

#include <sys/types.h>
#include "background.h"
#include "history.h"

struct sh{
	char buf[100];
	struct hlist hl;

	struct bglist bgl;
	int end_cnt;
};

// block signals when starting shell
void init_block_signals(void);

#endif
