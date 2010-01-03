/* nsuds - The ncurses sudoku program
 * Text-graphical sudoku with pencil-marking support.
 * Copyright (C) 2009, 2010 Vincent Launchbury.
 * -------------------------------------------
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  US
 */

/* util.c
 * ------
 * General functions that should be used throughout nsuds */
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_NCURSES_H
   #include <ncurses.h>
#else 
   #include <curses.h>
#endif

#include "util.h"

/* malloc w/ error checking.
 * Note: Try to avoid using tmalloc(NULL), some older
 * systems don't support it. */
void *tmalloc(size_t n)
{
   void *p = malloc(n);
   if (!p && n != 0) {
      endwin();
      fprintf(stderr, "Error: Out of memory!\n");
      exit(EXIT_FAILURE);
   }
   return p;
}

/* realloc w/ error checking 
 * Note: Try to avoid using trealloc(NULL, size), some older
 * systems don't support it. */
void *trealloc(void *p, size_t n)
{
   p = realloc(p, n);
   if (!p && n != 0) {
      endwin();
      fprintf(stderr, "Error: Out of memory!\n");
      exit(EXIT_FAILURE);
   }
   return p;
}

