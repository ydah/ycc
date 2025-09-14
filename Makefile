CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

ycc: $(OBJS)
				$(CC) -o ycc $(OBJS) $(LDFLAGS)

$(OBJS): ycc.h

test: ycc
				gcc -o test_ycc ./test/test_ycc.c
				./test_ycc

clean:
				rm -f ycc *.o *~ tmp*

.PHONY: test clean
