CFLAGS = -std=c99 -Wall
CC = gcc
LIB_FILES = string.o \
			json.o

test: test.o $(LIB_FILES)
	gcc $^ && ./a.out

clean:
	rm *.o a.out
