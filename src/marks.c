/* nsuds - The ncurses sudoku program
 * Text-graphical sudoku with campaign or free-play
 * Copyright (C) Vincent Launchbury 2009.
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
#include <stdio.h>
#include <ncurses.h>

#include "nsuds.h"
#include "marks.h"
#include "grid.h"

/* TODO: Perhaps use a sparse matrix */
bool marks[9][9][9] = {{{0}}};
short show=0;

/* Headers */
static int ask_int(char *question);

/* Mark current square with a number.  Similar to
 * writing a pencilmark in the square, indicating
 * a possible candidate */
void mark_square(void)
{
   int num;
   num = ask_int("Mark square with which number?");
   if (!num) return;
   marks[cury][curx][num]=1;

   if (show==num) draw_grid();
}


/* Show all marks for a number. Basically shows
 * all the squares that the user has marked as
 * candidates for that number. */
void marks_show(void)
{
   int num;
   num = ask_int("Reveal squares marked with which number?");

   show=num;
   draw_grid();
}


/* Clear all marks for a number. */
void marks_clear(enum clear_type type)
{
   int num;
   int i, j;

   switch (type) {
      default:
      case SINGLE:
         num = ask_int("Clear which mark from this square?");
         if (!num) return;
         marks[cury][curx][num]=0;
         break;
      case ALL:
         num = ask_int("Clear all marks for which number?");
         if (!num) return;

         for (i=0; i<9; i++) {
            for (j=0; j<9; j++) {
               marks[i][j][num]=0;
            }
         }
         break;
   }
   if (show==num) draw_grid();
}


/* Ask user for an integer input.
 * Returns 1-9 or 0 for anything else */
static int ask_int(char *question)
{
   int c;
   /* Print question on bottom line */
   move(row-1, 0);
   hide_fbar();
   printw("%s (1-9)", question);
   movec(CUR);

   /* Wait for input */
   while ((c = getch())) {
      if (c==ERR) continue;
      /* Real input occured, erase line */
      mvhline(row-1, 0, ACS_CKBOARD, col);
      movec(CUR);
      /* Return int or invalid */
      if (c>='1' && c<='9') {
         return c - '0';
      } else {
         return 0;
      }
   }
}

