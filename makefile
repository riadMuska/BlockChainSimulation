CC=gcc
CFLAGS=-std=c99 -pedantic -D_GNU_SOURCE
#-Wall -Werror -std=c89 -pedantic -D_GNU_SOURCE
#-std=c89 -pedantic

all: master node user

master: master.c funzioni.o
	$(CC) $(CFLAGS) master.c funzioni.o -o master

node: processi_nodo.c funzioni.o
	$(CC) $(CFLAGS) processi_nodo.c funzioni.o -o node

user: processi_utente.c funzioni.o
	$(CC) $(CFLAGS) processi_utente.c funzioni.o -o user

funzioni.o: funzioni.c
	$(CC) $(CFLAGS) funzioni.c -c

clean:
	rm -f *.o master node user *~