# Temp makefile, until we add autotools

CC = gcc
CFLAGS = -g -Wall -W -lncurses

all:
	$(CC) nsuds.c   -o a.out $(CFLAGS)
