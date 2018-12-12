#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "cmd.h"
#include "background.h"
#include "sh.h"

extern struct sh shell;

struct bg* new_bg(struct cmd *cmd, pid_t pid){
	struct bg*ret;
	int index = 0;
	char buf[200];

	ret = malloc(sizeof(*ret));
	memset(buf, 0, sizeof(buf));
	remake_cmd(&index, buf, cmd);
	buf[index] = 0;

	ret->cmd = malloc((index+1)*sizeof(char));
	strcpy(ret->cmd, buf);
	ret->pid = pid;
	ret->next = NULL;
	ret->order = 0;
	ret->state = NONE;

	return ret;
}

void free_bg(struct bg*fbg){
	free(fbg->cmd);
	free(fbg);

	return;
}

void init_bglist(struct bglist* bgl){
	bgl->start =bgl->end = NULL;
	bgl->count = 0;
}

void free_bglist(struct bglist*bgl){
	struct bg*cur, *next;
	cur = bgl->start;
	while (cur){
		next = cur->next;
		free_bg(cur);
		cur = next;
	}

	return;
}

void add_bglist(struct bglist*bgl, struct bg*add){
	if (bgl->end == NULL){
		bgl->start = add;
		bgl->end = add;
		add->order = 1;
	}else{
		add->order = bgl->end->order + 1;
		bgl->end->next = add;
		bgl->end = add;
	}

	add->state = RUNNING;
	bgl->count++;

	return;
}

int mark_end_bglist(struct bglist *bgl, pid_t pid){
	struct bg*cur;

	cur = bgl->start;
	while (cur){
		if (cur->pid == pid)
			break;
		cur = cur->next;
	}

	if (cur == NULL)
		return 0;

	cur->state = END;
	return 1;
}

void clean_bglist(struct bglist *bgl){
	struct bg *tmp_head, *tmp_cur;
	struct bg *cur, *next;
	int count = 0, index = 0;
	
	cur = bgl->start;
	tmp_head = tmp_cur= NULL;
	while (cur){
		next = cur->next;
		if (cur->state == RUNNING){
			count++;
			if (tmp_head == NULL){
				tmp_head = cur;
				tmp_cur = cur;
			}else{
				tmp_cur->next = cur;
				tmp_cur = tmp_cur->next;
			}
		}else{
			cur->state = DONE;
			print_bg_state(cur, bgl->count, index);
			free_bg(cur);
			shell.end_cnt--;
		}

		index++;
		cur = next;
	}

	if (tmp_cur != NULL)
		tmp_cur->next = NULL;

	bgl->count = count;
	bgl->start = tmp_head;
	bgl->end = tmp_cur;

	return;
}

void print_bg_state(struct bg*cur, int total, int index){
	char mps;
	char *saying;
	if (index == total-1)
		mps = '+';
	else if (index == total-2)
		mps = '-';
	else
		mps = ' ';

	switch (cur->state){
	case NONE:
		saying = "None";
		break;
	case RUNNING:
		saying = "Running";
		break;
	case END:
		saying = "End";
		break;
	case DONE:
		saying = "Done";
		break;
	default:
		saying = "Unknown";
		break;
	}

	printf("[%d]%c\t%s\t\t%s\n",
		cur->order, mps, saying,
		cur->cmd);

	return;
}




