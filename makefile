CC=gcc
CFLAGS=-c -std=c99 -Wall

all: server

server: main.o server.o query.o http_utils.o
	$(CC) main.o server.o query.o http_utils.o -o server

main.o: src/main.c
	$(CC) $(CFLAGS) src/main.c

server.o: src/server.c
	$(CC) $(CFLAGS) src/server.c

query.o: src/query.c
	$(CC) $(CFLAGS) src/query.c

http_utils.o: src/http_utils.c
	$(CC) $(CFLAGS) src/http_utils.c

clean:
	rm -rf *.o server

clean_object_file:
	rm -rf *.o