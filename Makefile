# Temp makefile, until we add autotools

CC = gcc
CFLAGS = -g -Wall -W -lncurses -Wno-unused

all:
	@echo CC		timer.o
	@$(CC) -c timer.c   -o timer.o $(CFLAGS)
	@echo CC		nsuds.c
	@$(CC) -c nsuds.c   -o nsuds.o $(CFLAGS)
	@echo Made binary file ./nsuds
	@$(CC) *.o -o nsuds $(CFLAGS)
