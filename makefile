CFLAGS = -std=c99 -Wall -Werror -g -D_GNU_SOURCE
CC = gcc
LIB_FILES = string.o \
			json.o

libmmijson.a: $(LIB_FILES)
	ar rcs $@ $^

test: test.o libmmijson.a
	$(CC) $^ -o test && ./test < test.json

clean:
	rm *.o *.a test
