dnl nsuds - The ncurses sudoku program
dnl Text-graphical sudoku with campaign or free-play
dnl Copyright (C) 2009, 2010 Vincent Launchbury.
dnl -------------------------------------------
dnl
dnl This program is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 2 of the License, or
dnl (at your option) any later version.
dnl 
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl  
dnl You should have received a copy of the GNU General Public License
dnl along with this program; if not, write to the Free Software
dnl Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  US

AC_PREREQ([2.63])
t://nsuds.git.sourceforge.net/gitroot/nsuds/nsuds
AC_INIT([nsuds], [0.6B], [vincent@doublecreations.com])
AC_COPYRIGHT([Parts Copyright (C) 2009, 2010 Vincent Launchbury])
AC_CONFIG_AUX_DIR([build])
AM_INIT_AUTOMAKE([-Wportability])
AC_CONFIG_SRCDIR([src/nsuds.c])
AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_FILES([Makefile
                 src/Makefile])

dnl Programs
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_INSTALL
AC_PROG_MAKE_SET

dnl Headers
AC_HEADER_STDC

dnl Check for ncurses 
CURSES_LIB=""
AC_CHECK_LIB([ncurses], [wbkgd], CURSES_LIB="-lncurses", )
AC_CHECK_LIB([curses], [wbkgd], test -z "$CURSES_LIB" && CURSES_LIB="-lcurses", )
AC_CHECK_HEADERS(ncurses.h curses.h,,
              [echo "Error: ncurses headers not found. You may need to\
               install an ncurses development package.";exit 1])
test "$silent" != yes && echo -n "checking for curses library to use... "
if test -n "$CURSES_LIB"; then
   test "$silent" != yes && echo "$CURSES_LIB"
else
   test "$silent" != yes && echo "none found!"
   echo "Error: Cannot find a curses library on the system. (curses or ncurses!)"
   exit 1
fi
AC_SUBST([CURSES_LIB])


# Find man dirs better than autoconf does.
# (Using the default man functionality
#  doesn't work on some distros I've tested,
#  so we have to find it manually)
MAN1_DIR=""
test "$silent" != yes && echo -n "checking for man1 directory... "
for dir in /usr/share/man/man1 \
   /usr/local/share/man/man1 \
   /usr/local/man/man1 \
   /usr/man/man1 \
   /usr/contrib/man/man1; do
   test -d $dir && MAN1_DIR=$dir && break
done
if test -n "$MAN1_DIR"; then
   test "$silent" != yes && echo "$MAN1_DIR"
else
   test "$silent" != yes && echo "no. Using /usr/share/man/man1"
   MAN1_DIR="/usr/share/man/man1"
fi
AC_SUBST([MAN1_DIR])

AC_OUTPUT

