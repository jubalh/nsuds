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
#include "config.h"

#ifdef HAVE_NCURSES_H
   #include <ncurses.h>
#else 
   #include <curses.h>
#endif
#if STDC_HEADERS || HAVE_STRING_H
   #include <string.h>
#else /* Old system with only <strings.h> */
   #include <strings.h>
#endif
#include <stdlib.h>
#include <time.h>

#include "nsuds.h"
#include "gen.h"
#include "grid.h"
#include "marks.h"

static bool grid_valid(void);

char grid_data[9][9]={{0}}; /* grid_data[y/row][x/col] */
int curx=0,cury=0;          /* Current (selected) grid coords */

/* Get screen coords from grid coords */
#define gy2scr(y) (3  + (y * 2))
#define gx2scr(x) (30 + (x * 4))
/* Get grid window coords from grid coords */
#define gy2win(y) (1 + (y * 2))
#define gx2win(x) (2 + (x * 4))
/* Get grid coords from screen coords (for mouse events) */
#define scry2g(y) ((y - 3) / 2)
#define scrx2g(x) ((x - 30) / 4)
/* Move grid cursor to grid coord */
#define gmove(y, x) wmove(grid, gy2win(y), gx2win(x))
/* Move grid cursor to left of grid coord */
#define gmovel(y, x) wmove(grid, gy2win(y), gx2win(x)-1)
/* Move screen cursor to grid coord */
#define smove(y, x) move(gy2scr(y), gx2scr(x))


/* Move cursor to another grid space */
void movec(int dir)
{
   switch (dir) {
      case UP:
         if (cury > 0) smove(--cury, curx);
         break;
      case DOWN:
         if (cury < 8) smove(++cury, curx);
         break;
      case LEFT:
         if (curx > 0) smove(cury, --curx);
         break;
      case RIGHT:
         if (curx < 8) smove(cury, ++curx);
         break;
      case TOP:
         cury=0;
         smove(cury, curx);
         break;
      case BOTTOM:
         cury=8;
         smove(cury, curx);
         break;
      case HOME:
         curx=0;
         smove(cury, curx);
         break;
      case END:
         curx=8;
         smove(cury, curx);
         break;
      case CUR:
         smove(cury, curx);
         break;
   }
}

/* Move to specified screen location, if user
 * clicked on a valid grid square */
void movec_mouse(int x, int y)
{
   int gx, gy;
   /* Get grid coords from screen coords.
    * If we didn't add 1 to x, clicking on the left part of (2,1)
    * would move to (1,1), because of horizontal padding. */
   gx = scrx2g(x+1);
   gy = scry2g(y);
   /* If user clicked on a valid square, move to it */
   if (gx >= 0 && gx < 9 &&
       gy >= 0 && gy < 9) {
          cury = gy;
          curx = gx;
          smove(cury,curx);
   }
}


/* Add a mutable char to the current grid location */
void gaddch(char ch)
{
   /* If char is immutable, do nothing */
   if (grid_data[cury][curx]<0) return;
   gmove(cury,curx);
   grid_data[cury][curx]= ch-'0';
   draw_grid();

   /* Check if compelted */
   if (grid_filled() == 81 && grid_valid())
      game_win();
}


/* Check if a full or partially filled
 * sudoku grid is valid or not */
static bool grid_valid(void)
{
   int i,j,k;
   char rowf[9], colf[9];

   /* Check rows/cols */
   for (i=0; i<9; i++) {
      /* Reset row/col finds */
      memset(&rowf, 0, 9);
      memset(&colf, 0, 9);

      /* Add finds to colf/rowf */
      for (j=0; j<9; j++) {
         if (abs(grid_data[i][j])) colf[abs(grid_data[i][j])-1]++;
         if (abs(grid_data[j][i])) rowf[abs(grid_data[j][i])-1]++;
      }
      
      /* Check if a number was found more than once per
       * row/col */
      for (j=0; j<9; j++) {
         if (colf[j] > 1|| rowf[j] > 1) return 0;
      }
   }

   /* Check segments (segment start=(i,j)) */
   for (i=0; i<9; i+=3) {
      for (j=0; j<9; j+=3) {
         memset(&rowf, 0, 9);
         memset(&colf, 0, 9);
         
         /* Check #'s within each segment */
         for (k=0;k<3;k++) {
            if (abs(grid_data[i+k][j]))   rowf[abs(grid_data[i+k][j])-1]++;
            if (abs(grid_data[i+k][j+1])) rowf[abs(grid_data[i+k][j+1])-1]++;
            if (abs(grid_data[i+k][j+2])) rowf[abs(grid_data[i+k][j+2])-1]++;
         }

         for (k=0; k<9; k++) {
            if (rowf[k] > 1) return 0;
         }
      }
   }

   return 1;
}

/* Return how many squares in the grid are filled, for stats window. */
int grid_filled(void)
{
   int i, j, ret=0;
   for (i=0; i<9; i++)
      for (j=0;j<9;j++)
         if (grid_data[i][j]) ret++;
   return ret;
}

/* Draw the contents of the grid, which includes handling the highlighting of
 * selected marks, as well as color. */
void draw_grid_contents(void)
{
   int i, j;

   /* Check if paused */
   if (paused) return;

   /* Draw grid contents */
   for (i=0; i<9; i++) {
      for (j=0; j<9; j++) {
         gmovel(i, j);

         /* If we're showing marks in the first space */
         if (showmarks[0]) {
            /* If grid is filled in, and equals showmarks[0] */
            if (grid_data[i][j] && abs(grid_data[i][j]) == showmarks[0]) {
               if (use_colors) {
                  waddch(grid, (abs(grid_data[i][j]) + '0') 
                     | COLOR_PAIR(6) | A_UNDERLINE);
               } else {
                  waddch(grid, (abs(grid_data[i][j]) + '0') 
                     | A_REVERSE | A_UNDERLINE);
               }
            /* If grid is empty, but mark showmarks[0] is set */
            } else if (!grid_data[i][j] && marks[i][j][showmarks[0]]) {
               if (use_colors) {
                  waddch(grid, (showmarks[0] + '0') | COLOR_PAIR(6));
               } else {
                  waddch(grid, (showmarks[0] + '0') | A_REVERSE);
               }
            /* Otherwise print a blank space */
            } else waddch(grid, ' ');
         } else waddch(grid, ' ');


         /* If we're showing a mark in the second space */
         if (showmarks[1]) {
            /* If grid is filled in and equals showmarks[1] */
            if (grid_data[i][j] && abs(grid_data[i][j]) == showmarks[1]) {
               if (use_colors) {
                  waddch(grid, (abs(grid_data[i][j]) + '0') 
                     | COLOR_PAIR(5) | A_UNDERLINE);
               } else {
                  waddch(grid, (abs(grid_data[i][j]) + '0') 
                     | A_REVERSE | A_UNDERLINE);
               }
            /* Grid is filled, but isn't a mark. Just show the value,
             * unless it's shown in the first or third spaces */
            } else if (grid_data[i][j] && (!showmarks[0] || 
                 (abs(grid_data[i][j]) != showmarks[0] 
                   && abs(grid_data[i][j]) != showmarks[2]))) {
               /* Square value was input by user, show in cyan */
               if (grid_data[i][j] > 0 && has_colors()) {
                  waddch(grid, (abs(grid_data[i][j]) + '0') | COLOR_PAIR(1));
               /* Square value is part of the generated puzzle */
               } else {
                  waddch(grid, abs(grid_data[i][j]) + '0');
               }
            /* If grid is empty, but mark showmarks[1] is set */
            } else if (!grid_data[i][j] && marks[i][j][showmarks[1]]) {
               if (use_colors) {
                  waddch(grid, (showmarks[1] + '0') | COLOR_PAIR(5));
               } else {
                  waddch(grid, (showmarks[1] + '0') | A_REVERSE);
               }
              /* If we're only showing one set of marks, output a question
               * mark beside them */
               if (!showmarks[0]) waddch(grid, '?');
               /* Otherwise print a blank space */
               } else waddch(grid, ' ');
         /* If we're not showing marks, show contents of the square, if any */
         } else if (!showmarks[0]) {
            if (grid_data[i][j]) {
               /* Square value was input by user, show in cyan */
               if (grid_data[i][j] > 0 && has_colors()) {
                  waddch(grid, (abs(grid_data[i][j]) + '0') | COLOR_PAIR(1));
                  /* Square value is part of the generated puzzle */
               } else {
                  waddch(grid, abs(grid_data[i][j]) + '0');
               }
               /* Square is empty */
            } else waddch(grid, ' ');
         }

         
         /* If we're showing marks in the third space */
         if (showmarks[2]) {
            /* If grid is filled in, and equals showmarks[2] */
            if (grid_data[i][j] && abs(grid_data[i][j]) == showmarks[2]) {
                  if (use_colors) {
                     waddch(grid, (abs(grid_data[i][j]) + '0') 
                        | COLOR_PAIR(7) | A_UNDERLINE);
                  } else {
                     waddch(grid, (abs(grid_data[i][j]) + '0') 
                        | A_REVERSE | A_UNDERLINE);
                  }
            /* If grid is empty, but mark showmarks[2] is set */
            } else if (!grid_data[i][j] && marks[i][j][showmarks[2]]) {
               if (use_colors) {
                  waddch(grid, (showmarks[2] + '0') | COLOR_PAIR(7));
               } else {
                  waddch(grid, (showmarks[2] + '0') | A_REVERSE);
               }
            /* Otherwise print nothing */
            }
         }

      }
   }

   wnoutrefresh(grid);
}

/* Generate a puzzle */
void generate(void)
{
   do_generate(35);
}

