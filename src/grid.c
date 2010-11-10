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

#include "nsuds.h"
#include "grid.h"
#include "marks.h"

static bool grid_valid(void);
static void sub_move(int *a1, int *a2, int toward);

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

      /* Go to the center of the an adjacent sub-square */
      case SUB_RIGHT:
         sub_move(&curx, &cury, 9);
         break;
      case SUB_LEFT:
         sub_move(&curx, &cury, 0);
         break;
      case SUB_UP:
         sub_move(&cury, &curx, 0);
         break;
      case SUB_DOWN:
         sub_move(&cury, &curx, 9);
         break;

      /* Move to current position (after a redraw) */
      case CUR:
         smove(cury, curx);
         break;
   }
}


/* Move to the center of an adjacent subsection by:
 *  - Moving along axis1 toward the cell 'toward' 
 *  - Centering axis2 */
static void sub_move(int *a1, int *a2, int toward) {
   /* Moving along axis1 toward 0 */
   if (toward == 0) {
      if (*a1 <= 2) return;
      else if (*a1 >= 3 && *a1 <= 5) *a1=1;
      else *a1=4;
   /* Moving along axis1 toward 9 */
   } else {
      if (*a1 <= 2) *a1=4;
      else if (*a1 >= 3 && *a1 <= 5) *a1=7;
      else return;
   }

   /* Center axis2 */
   if (*a2 <= 2) *a2=1;
   else if (*a2 >= 3 && *a2 <= 4) *a2=4;
   else *a2=7;

   smove(cury, curx);
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


/* Fill in the current grid location */
void gsetcur(char ch)
{
   /* If char is immutable, do nothing */
   if (grid_data[cury][curx]<0) return;

   gmove(cury,curx);
   grid_data[cury][curx] = ch;
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
         if (colf[j] > 1 || rowf[j] > 1) return 0;
      }
   }

   /* Check segments (segment start=(i,j)) */
   for (i=0; i<9; i+=3) {
      for (j=0; j<9; j+=3) {
         memset(&rowf, 0, 9);
         
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


/* Attributes for nubmer we're highlighting */
static attr_t show_attr(int mark, int uline)
{
   int attr=0;

   if (uline) attr |= A_UNDERLINE;
   if (!use_colors) {
      attr |= A_REVERSE;
   } else {
      switch (mark) {
         case 1:
            attr |= COLOR_PAIR(C_MARKS1);
            break;
         case 2:
            attr |= COLOR_PAIR(C_MARKS2);
            break;
         case 3:
            attr |= COLOR_PAIR(C_MARKS3);
            break;
      }
   }
   return attr;
}

/* Attribute for a regular number, user inputted or a default */
static attr_t user_attr(int user)
{
   return (user > 0 && use_colors ? COLOR_PAIR(C_INPUT) : 0);
}

/* Draw the contents of the grid, including mark highlighting (if set) */
void draw_grid_contents(void)
{
   int i, j, k;

   if (is_paused()) return;

   /* For each square */
   for (i=0; i<9; i++) {
      for (j=0; j<9; j++) {
         /* Move to the square */
         gmovel(i, j);

         /* If we're not highlighting any marks */
         if (!showmarks[1]) {
            if (grid_data[i][j]) {
               /* Square is filled, show */
               waddch(grid, ' ');
               waddch(grid, (abs(grid_data[i][j]) + '0') | user_attr(grid_data[i][j]));
            }
         /* If we're highlighting something */
         } else {
            int output=0; /* Have we output a number? */

            /* For each of the 3 positions */
            for (k=0; k <= 2; k++) {
               if (showmarks[k]) {
                  /* Square is filled with a number to be highlighted */
                  if (abs(grid_data[i][j]) == showmarks[k]) {
                     output=1;
                     waddch(grid, (abs(grid_data[i][j]) + '0') | show_attr(k+1,1));
                  /* Square is empty, but a mark is set */
                  } else if (!grid_data[i][j] && marks[i][j][showmarks[k]]) {
                     output=1;
                     waddch(grid, (showmarks[k] + '0') | show_attr(k+1,0));
                  } else waddch(grid, ' ');
               } else waddch(grid, ' ');
            } 

            /* Grid is filled, but it isn't anything we're highlighting */
            if (grid_data[i][j]  && !output) {
                  gmove(i,j);
                  waddch(grid, (abs(grid_data[i][j]) + '0') | user_attr(grid_data[i][j]));
            }
         }
      }
   }

   wnoutrefresh(grid);
}

