# Temp makefile, until we add autotools

CC = gcc
CFLAGS = -g -Wall -W -lncurses -Wno-unused -ansi -pedantic

all:
	@echo CC		timer.o
	@$(CC) -c timer.c   -o timer.o $(CFLAGS)
	@echo CC		grid.o
	@$(CC) -c grid.c   -o grid.o $(CFLAGS)
	@echo CC		nsuds.o
	@$(CC) -c nsuds.c   -o nsuds.o $(CFLAGS)
	@echo Made binary file ./nsuds
	@$(CC) *.o -o nsuds $(CFLAGS)
