# Temp makefile, until we add autotools

CC = gcc
CFLAGS = -g -Wall -W -lncurses -Wno-unused -ansi -pedantic
MODULES = $(shell ls *.c |sed 's_\.c_\.o_')


all: depend $(MODULES) Makefile
	@echo Made binary file ./nsuds
	@$(CC) *.o -o nsuds $(CFLAGS)

clean:
	@rm *.o
	@rm nsuds

%.o: %.c
	@$(CC) -c  $< -o $@ $(CFLAGS)
	@echo CC $@

depend:
	@makedepend -Y  -- $(CFLAGS) -- *.c  2>/dev/null
# DO NOT DELETE

gen.o: grid.h
grid.o: nsuds.h gen.h grid.h
nsuds.o: nsuds.h timer.h grid.h
timer.o: nsuds.h timer.h
