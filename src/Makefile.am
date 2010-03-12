bin_PROGRAMS = nsuds
nsuds_SOURCES = dialog.c gen.c grid.c highscores.c marks.c \
					 menu.c nsuds.c score.c scroller.c timer.c util.c
noinst_HEADERS = dialog.h gen.h grid.h highscores.h marks.h \
					 menu.h nsuds.h score.h scroller.h timer.h util.h
nsuds_CFLAGS = -pedantic -ansi -Wall -W \
					-DHELPDIR='"$(datadir)/doc/$(PACKAGE)-${VERSION}/"' \
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

# Make sure games group can write the score file
install-data-hook:
	chgrp games $(DESTDIR)$(localstatedir)/games/$(PACKAGE)/high_scores

