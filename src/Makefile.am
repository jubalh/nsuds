bin_PROGRAMS = nsuds
nsuds_SOURCES = gen.c nsuds.c timer.c grid.c marks.c scroller.c \
					 util.c score.c menu.c highscores.c
noinst_HEADERS = gen.h nsuds.h timer.h grid.h marks.h scroller.h \
					  util.h score.h menu.h highscores.h
nsuds_CFLAGS = -pedantic -ansi -Wall -W -DHELPDIR='"$(datadir)/doc/$(PACKAGE)-${VERSION}/"' \
					-DSCOREDIR='"$(localstatedir)/games/$(PACKAGE)/"'
nsuds_LDFLAGS = @CURSES_LIB@ -lm
 
highscoredir = $(localstatedir)/games/$(PACKAGE)
dist_highscore_DATA=high_scores

dist_man_MANS = nsuds.1

# Set group to games, and setgid unless asked not to
install-exec-hook:
	chgrp games $(DESTDIR)$(bindir)/nsuds
if USE_SETGID
	chmod 2555  $(DESTDIR)$(bindir)/nsuds
else 
	chmod 0555  $(DESTDIR)$(bindir)/nsuds
endif
