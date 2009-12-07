# We want /usr/bin, not /usr/local/bin
bexecdir = /usr/bin
bexec_PROGRAMS = nsuds

nsuds_SOURCES = gen.c nsuds.c timer.c grid.c marks.c help.c util.c
noinst_HEADERS = gen.h nsuds.h timer.h grid.h marks.h help.h util.h
nsuds_CFLAGS = -pedantic -ansi -Wall -W
nsuds_LDFLAGS = -lncurses -lm
 
# Doesn't work on some distros I tested.
#man_MANS = nsuds.1

# So we find it ourselves instead
ourman1dir = @MAN1_DIR@
ourman1_DATA = nsuds.1

# Make sure manpage is in 'make dist' tarball
dist_data_DATA = nsuds.1
