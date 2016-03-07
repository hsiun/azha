CC = gcc
CFLAGS = -O2 -Wall -I .

LIB = -lpthread

all: azha client

azha: azha.c utils.o
	$(CC) $(CFLAGS) -o azha azha.c utils.o $(LIB)

client: client.c utils.o
	$(CC) $(CFLAGS) -o client client.c utils.o $(LIB)

utils.o: utils.c
	$(CC) $(CFLAGS) -c utils.c

clean:
	rm -rf *.o azha client
