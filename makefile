CFLAGS = -std=c99 -Wall -Werror -g
CC = gcc
LIB_FILES = string.o \
			json.o

test: test.o $(LIB_FILES)
	$(CC) $^ && ./a.out

clean:
	rm *.o a.out
