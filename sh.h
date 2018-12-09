#ifndef SH_H
#define SH_H

#include "history.h"

struct sh{
	char buf[100];
	struct hlist hl;
};

#endif
