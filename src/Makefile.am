bin_PROGRAMS = nsuds
nsuds_SOURCES = gen.c nsuds.c timer.c grid.c marks.c scroller.c \
					 util.c score.c menu.c
noinst_HEADERS = gen.h nsuds.h timer.h grid.h marks.h scroller.h \
					  util.h score.h menu.h
nsuds_CFLAGS = -pedantic -ansi -Wall -W -DHELPDIR='"$(datadir)/doc/$(PACKAGE_NAME)-${VERSION}/"'
nsuds_LDFLAGS = @CURSES_LIB@ -lm
 
dist_man_MANS = nsuds.1
