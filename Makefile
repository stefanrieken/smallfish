#CFLAGS=-Wall -Os
CFLAGS=-Wall -g -DQUICK_EVAL

%.c : %.h

smallfish: src/smallfish.c src/parse.c src/gc.c src/type/dict.c src/type/parr.c src/type/expr.c src/type/method.c src/type/block.c src/type/string.c src/type/int.c src/type/prim.c src/type/object.c
	gcc $(CFLAGS) $^ -o $@

clean:
	rm -rf smallfish *.o
