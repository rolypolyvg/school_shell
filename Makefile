# 참조: https://codereview.stackexchange.com/questions/2547/makefile-dependency-generation/11109#11109
CC = gcc
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)
EXE = mysh.out

$(EXE) : $(OBJS)
	$(CC) -o $@ $^

$(OBJS):%.o:%.c
	$(CC) -MMD -c $<

.PHONY: clean

clean:
	-rm -f $(EXE) $(OBJS) $(DEPS)
# - in front means to continue even error occur

-include $(DEPS)
