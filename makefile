CFLAGS = -std=c99 -Wall -Werror -g -D_GNU_SOURCE
CC = gcc
LIB_FILES = string.o \
            json.o

libmmijson.a: $(LIB_FILES)
	ar rcs $@ $^

test: test.o libmmijson.a
	$(CC) $^ -o test && ./test < test.json

testcpp.o: test.c
	g++ -c -o testcpp.o test.c

testcpp: testcpp.o libmmijson.a
	g++ $^ -o testcpp && ./testcpp < test.json

clean:
	rm *.o *.a *.exe test testcpp
