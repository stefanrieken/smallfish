#CFLAGS=-Wall -Os
CFLAGS=-Wall -ggdb

%.c : %.h

smallfish: src/smallfish.c src/gc.c src/type/dict.c src/type/expr.c src/type/string.c src/type/int.c src/type/prim.c
	gcc $(CFLAGS) $^ -o $@

clean:
	rm -rf smallfish *.o
