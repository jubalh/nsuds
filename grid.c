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
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "nsuds.h"
#include "grid.h"

static void gaddimch(int y, int x, char ch);

static char grid_data[9][9]; /* grid_data[y/row][x/col] */
static int curx=0,cury=0;    /* Current (selected) grid coords */
static bool initialized=0;   /* Is grid initialized? */

/* Get screen coords from grid coords */
#define gy2scr(y) (3  + (y * 2))
#define gx2scr(x) (30 + (x * 4))
/* Get grid window coords from  grid coords */
#define gy2win(y) (1 + (y * 2))
#define gx2win(x) (2 + (x * 4))
/* Move grid cursor to grid coord */
#define gmove(y, x) wmove(grid, gy2win(y), gx2win(x))
/* Move screen cursor to grid coord */
#define smove(y, x) move(gy2scr(y), gx2scr(x))


/* Initialize the grid */
void init_grid(void)
{
   memset(grid_data, 0, 9*9);
}


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
      case CUR:
         smove(cury, curx);
         break;
   }
}

/* Add an immutable char to the grid */
static void gaddimch(int y, int x, char ch)
{
   gmove(y, x);
   if (ch == '0') waddch(grid, ' ');
   else waddch(grid, ch);
   grid_data[y][x] = -(ch-'0');
}

/* Add a mutable char to the current grid location */
void gaddch(char ch)
{
   /* If char is immutable, do nothing */
   if (grid_data[cury][curx]<0) return;
   gmove(cury,curx);
   if (ch == '0') waddch(grid, ' ');
   else waddch(grid, ch);
   grid_data[cury][curx]= ch-'0';
   wnoutrefresh(grid);
}


/* Check if a full or partially filled
 * sudoku grid is valid or not */
bool grid_valid(void)
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

int grid_filled(void)
{
   int i, j, ret=0;
   for (i=0; i<9; i++)
      for (j=0;j<9;j++)
         if (grid_data[i][j]) ret++;
   return ret;
}

void draw_grid_contents(void)
{
   int i, j;

   if (paused) return;
   for (i=0; i<9; i++) {
      for (j=0; j<9; j++) {
         gmove(i, j);
         if (grid_data[i][j] != 0) 
            waddch(grid, abs(grid_data[i][j]) + '0');
      }
   }

   wnoutrefresh(grid);
}

/* Poorly generate a puzzle, for testing */
void generate(int num)
{
   int i;
   int findx, findy;
   srand(time(NULL));
   for (i=0; i < num; i++) {
      /* Find random empty square */
      while (1) {
         findy = rand()%9;
         findx = rand()%9;

         if (!grid_data[findy][findx]) {
            gaddimch(findy, findx, '1' + (rand()%8) );
            if (!grid_valid()) {
               gaddimch(findy, findx, '0');
               continue;
            }
            break;
         }
      }
   }
}

