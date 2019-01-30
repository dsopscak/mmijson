CFLAGS = -std=c99 -Wall -Werror -g -D_GNU_SOURCE
CC = gcc
LIB_FILES = string.o \
			json.o

test: test.o $(LIB_FILES)
	$(CC) $^ -o test && ./test < test.json

clean:
	rm *.o test
