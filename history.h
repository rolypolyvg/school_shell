#ifndef HISTORY_H
#define HISTORY_H

// linked list node
struct hnode{
	struct hnode *next, *before;
	int nb;
	char *cmd;
};

// linked list of history
struct hlist{
	struct hnode*front, *back;
	int max_sz, cur_sz;
};

struct hnode* new_hnode(char *cmd, int nb);
void free_hnode(struct hnode*hn);

void init_hl(struct hlist*hl, int max_sz);
void clean_hl(struct hlist*hl);
int is_empty_hl(struct hlist*hl);
struct hnode*traverse_hl(struct hlist*hl); // if hl is NULL return next hnode in hl
void add_new_hl(struct hlist*hl, char *cmd); // add new command and delete first cmd if size is over max_sz
	
#endif
