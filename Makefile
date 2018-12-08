
mysh: sh.o parser.o cmd.o helper.o
	gcc -o $@ $^

sh.o: sh.c cmd.h parser.h helper.h
	gcc -c $^

cmd.o: cmd.c cmd.h helper.h
	gcc -c $^

parser.o: parser.c cmd.h parser.h
	gcc -c $^

helper.o: helper.c helper.h
	gcc -c $^

clean:
	rm -f *.o mysh
