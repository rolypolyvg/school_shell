#include <stdlib.h>
#include <string.h>
#include "history.h"

struct hnode* new_hnode(char *cmd, int nb){
	struct hnode*ret;

	ret = malloc(sizeof(*ret));
	ret->next = ret->before = NULL;
	ret->cmd = malloc((strlen(cmd)+1)*sizeof(char));
	strcpy(ret->cmd, cmd);
	ret->nb = nb;

	return ret;
}

void free_hnode(struct hnode*hn){
	free(hn->cmd);
	free(hn);
	return;
}

void init_hl(struct hlist*hl, int max_sz){
	hl->front = hl->back = NULL;
	hl->max_sz = max_sz;
	hl->cur_sz = 0;
	return;
}

struct hnode*traverse_hl(struct hlist*hl){
	static struct hnode*cur;

	if (hl != NULL)
		cur = hl->front;
	else
		cur = cur->next;

	return cur;
}

void clean_hl(struct hlist*hl){
	struct hnode*cur, *next;

	cur = hl->front;
	while (cur != NULL){
		next = cur->next;
		free_hnode(cur);
		cur = next;
	}

	return;
}

int is_empty_hl(struct hlist*hl){
	return hl->cur_sz == 0;
}

void add_new_hl(struct hlist*hl, char *cmd){
	struct hnode*add, *old;
	int nb;

	if (is_empty_hl(hl)){
		add = new_hnode(cmd, 1);
		hl->front = hl->back = add;
	}else{
		nb = hl->back->nb+1;
		if (nb > 10000)
			nb = 1;

		add = new_hnode(cmd, nb);
		hl->back->next = add;
		add->before = hl->back;
		hl->back = add;
	}

	if (hl->cur_sz == hl->max_sz){
		old = hl->front;
		hl->front->next->before = NULL;
		hl->front = hl->front->next;
		free_hnode(old);
	}else
		hl->cur_sz++;

	return;
}
