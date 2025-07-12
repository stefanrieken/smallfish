#CFLAGS=-Wall -Os
CFLAGS=-Wall -g -DQUICK_EVAL

%.c : %.h

smallfish: src/smallfish.c src/parse.c src/gc.c src/type/*.c
	gcc $(CFLAGS) $^ -o $@

clean:
	rm -rf smallfish smallfish.dSYM *.o
