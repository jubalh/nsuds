# nsuds - The ncurses sudoku program
# Text-graphical sudoku with campaign or free-play
# Copyright (C) Vincent Launchbury 2009.
# -------------------------------------------
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  US

AC_PREREQ([2.63])
t://nsuds.git.sourceforge.net/gitroot/nsuds/nsuds
AC_INIT([nsuds], [0.1B], [vincent@doublecreations.com])
AC_CONFIG_AUX_DIR([build])
AM_INIT_AUTOMAKE([-Wall -Werror])
AC_CONFIG_SRCDIR([src/nsuds.c])
AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_FILES([Makefile
                 src/Makefile])

# Programs
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_INSTALL
AC_PROG_MAKE_SET

# Headers
AC_HEADER_STDC

# Check for ncurses 
AC_CHECK_LIB([ncurses], [wbkgd],, 
             [AC_CHECK_HEADERS(ncurses.h,,
              [echo "Error: ncurses headers not found. You may need to install an ncurses development package.";exit 1])]
)

AC_OUTPUT

