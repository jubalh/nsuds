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

#if 0
/* Spare matrix of marks (9x81) */
static int *mark_rows;  /* Row of each mark (number 1-9)*/
static int *mark_cols;  /* Column of each mark (square 1-81)*/
#endif

/* Headers */
static int ask_int(char *question);

/* Mark current square with a number.  Similar to
 * writing a pencilmark in the square, indicating
 * a possible candidate */
void mark_square(void)
{
   int sq;
   sq = ask_int("Mark square with which number?");
   
   if (!sq) return;
}


/* Show all marks for a number. Basically shows
 * all the squares that the user has marked as
 * candidates for that number. */
/* TODO: Already filled in squares should be 
 * highlighted too.
 * */
void marks_show(void)
{
   int sq;
   sq = ask_int("Reveal squares marked with which number?");

   if (!sq) return;
   /* 
    * if (!sq) hightlight none
    * if (sq)  hightlight(sq)
    */
}


/* Clear all marks for a number. */
void marks_clear(void)
{
   int sq;
   sq = ask_int("Clear marks for which number?");

   if (!sq) return;

}


/* Ask user for an integer input.
 * Returns 1-9 or 0 for anything else */
static int ask_int(char *question)
{
   int c;
   /* Print question on bottom line */
   move(row-1, 0);
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

