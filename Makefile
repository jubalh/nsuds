# Temp makefile, until we add autotools

CC = gcc
CFLAGS = -g -Wall -W -lncurses

all:
	$(CC) -c timer.c   -o timer.o $(CFLAGS)
	$(CC) -c nsuds.c   -o nsuds.o $(CFLAGS)
	$(CC) *.o -o nsuds $(CFLAGS)
