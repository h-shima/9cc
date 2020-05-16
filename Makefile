CFLAGS=-std=c11 -g -static
SRCS=$(wildcard	*.c)
OBJS=$(SRCS:.c=.o)

9cc: $(OBJS)
		$(CC)	-o	9cc	$(OBJS)	$(LDFLAGS)

$(OBJS):	9cc.h

test: 9cc
	./9cc tests/tests.c > tmp.s
	gcc -static -o tmp tmp.s
	./tmp
clean:
	rm -rf 9cc *.o *~ tmp* tests/*~ tests/*.o

.PHONY: test clean