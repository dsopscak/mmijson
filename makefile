CFLAGS = -std=c99 -Wall
CC = gcc

test: test.o string.o
	gcc $^ && ./a.out

clean:
	rm *.o a.out
