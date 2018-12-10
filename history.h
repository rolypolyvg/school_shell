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
void clean_hl(struct hlist*hl);			// free all hnode and delete in hlist hl
int is_empty_hl(struct hlist*hl);		// true if hlist hl has no hnode

// traverse hlist from front to back
// first function call initialize which hlist to use
// preceding function call should put NULL in hl in order to traverse
// calling with new hl will begin traversing from new hlist
// return : current hnode in while traversing hlist hl
// return : if there is no more element return NULL
struct hnode*traverse_hl(struct hlist*hl); 

// add new command to history
// if history current size is same as max size
// it will delete first command to keep the max size
void add_new_hl(struct hlist*hl, char *cmd);

#endif
