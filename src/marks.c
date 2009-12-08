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
#include <stdarg.h>
#ifdef HAVE_NCURSES_H
   #include <ncurses.h>
#else 
   #include <curses.h>
#endif

#include "nsuds.h"
#include "marks.h"
#include "grid.h"

/* TODO: Perhaps use a sparse matrix */
bool marks[9][9][9] = {{{0}}};
short showmarks[3]={0};

/* Headers */
static int ask_int(char *question, ...);

/* Mark current square with a number.  Similar to
 * writing a pencilmark in the square, indicating
 * a possible candidate */
void mark_square(void)
{
   int num;
   num = ask_int("Mark square with which number? (1-9)");
   if (!num) return;
   marks[cury][curx][num]=1;

   if (showmarks[0] == num 
      || showmarks[1]==num 
      || showmarks[2]) 
      draw_grid();
}


/* Show all marks for a number. Basically shows
 * all the squares that the user has marked as
 * candidates for that number. */
void marks_show(enum show_type type)
{
   int num;

   switch(type) {
      default:
      case ONE:
         num = ask_int("Reveal squares marked with which number? (1-9)");
         showmarks[1]=num;
         showmarks[0]=showmarks[2]=0;
         draw_grid();
         break;
      case MULTIPLE:
         num = ask_int("Reveal squares marked with which numbers? (1-9)");
         if (!num) {
            showmarks[0]=showmarks[1]=showmarks[2]=0;
            goto done;
         }
         showmarks[0]=num;

second:
         num = ask_int("%d and..? (1-9, Enter for just `%d')", 
             showmarks[0], showmarks[0]);
         if (!num) {
            showmarks[1] = showmarks[0];
            showmarks[0]=showmarks[2]=0;
            goto done;
         }
         if (num==showmarks[0]) goto second;
         showmarks[1]=num;

third:
         num = ask_int("%d,%d and..? (1-9, Enter for just `%d,%d')", 
             showmarks[0], showmarks[1], showmarks[0], showmarks[1]);
         if (!num) {
            showmarks[2]=0;
            goto done;
         }
         if (num == showmarks[0] || num == showmarks[1]) goto third;
         showmarks[2]=num;
done:
         draw_grid();
   }
}


/* Clear all marks for a number. */
void marks_clear(enum clear_type type)
{
   int num;
   int i, j;

   switch (type) {
      default:
      case SINGLE:
         num = ask_int("Clear which mark from this square? (1-9)");
         if (!num) return;
         marks[cury][curx][num]=0;
         break;
      case ALL:
         num = ask_int("Clear all marks for which number? (1-9)");
         if (!num) return;

         for (i=0; i<9; i++) {
            for (j=0; j<9; j++) {
               marks[i][j][num]=0;
            }
         }
         break;
   }
   if (showmarks[0] == num 
      || showmarks[1]==num 
      || showmarks[2]) 
      draw_grid();
}


/* Ask user for an integer input.
 * Returns 1-9 or 0 for anything else */
static int ask_int(char *question, ...)
{
   va_list ap;
   int c;
   /* Print question on bottom line */
   move(row-1, 0);
   hide_fbar();

   va_start(ap, question);
   vwprintw(stdscr, question, ap);
   va_end(ap);
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

