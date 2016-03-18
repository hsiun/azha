CC = gcc
CFLAGS = -g -O2 -Wall -I .

LIB = -lpthread

all: azha client

azha: azha.c utils.o rio.o http_1_0.o
	$(CC) $(CFLAGS) -o azha azha.c utils.o rio.o http_1_0.o $(LIB)

client: client.c utils.o
	$(CC) $(CFLAGS) -o client client.c utils.o $(LIB)

utils.o: utils.c
	$(CC) $(CFLAGS) -c utils.c

rio.o: rio.c
	$(CC) $(CFLAGS) -c rio.c

http_1_0.o:http_1_0.c
	$(CC) $(CFLAGS) -c http_1_0.c

clean:
	rm -rf *.o azha client
