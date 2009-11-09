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
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

#include "nsuds.h"
#include "timer.h"

static int curx,cury;
signed char grid_data[9][9]; /* grid_data[y/row][x/col] */
int paused=0;
static int colors=0;
static int row,col;

/* Set up decent defaults */
static void init_ncurses(void)
{
   initscr(); /* Enter curses */
   if (has_colors()) {
      start_color(); 
   }

   init_pair(1, COLOR_BLUE, COLOR_BLACK);
   cbreak();      /* Disable line buffering */
   noecho();      /* Don't echo typed chars */
   keypad(stdscr, TRUE); /* Catch special keys */
   getmaxyx(stdscr, row, col);
   refresh();
}

static void init_windows(void) 
{
   title = newwin(1, 64, 0, 1);
   grid=newwin(19, 37, 2, 28);
   timer = newwin(6, 25, 2, 1);
   stats = newwin(13, 25, 8, 1);
}

static void init_grid(void)
{
   memset(grid_data, 0, 9*9);
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
         if (grid_data[i][j]) colf[grid_data[i][j]-1]++;
         if (grid_data[j][i]) rowf[grid_data[j][i]-1]++;
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
            if (grid_data[i+k][j])   rowf[grid_data[i+k][j]-1]++;
            if (grid_data[i+k][j+1]) rowf[grid_data[i+k][j+1]-1]++;
            if (grid_data[i+k][j+2]) rowf[grid_data[i+k][j+2]-1]++;
         }

         for (k=0; k<9; k++) {
            if (rowf[k] > 1) return 0;
         }
      }
   }

   return 1;
}

static int grid_filled(void)
{
   int i, j, ret=0;
   for (i=0; i<9; i++)
      for (j=0;j<9;j++)
         if (grid_data[i][j]) ret++;
   return ret;
}
                  

static void draw_grid(void)
{
   int i, j;
   int left;

   left = 28;
   refresh();
   werase(grid);

   box(grid, 0, 0);

   if (paused) {
      mvwaddstr(grid, 9, 15, "Paused");
   } else {

      /* Horizontal insides */
      for (i=2; i<18; i+=2) {
         if (i%6==0) continue;
         mvwhline(grid, i, 1, '-', 35);
      }
      /* Vertical insides */
      for (i=4; i<36; i+=4) {
         if (i%12==0) continue;
         mvwvline(grid, 1, i, '|', 17);
      }
      /* Verticals */
      for (i=1; i<18; i+=1) {
         if (i%6==0) continue;
         for (j=0; j<=36;j+=12) {
            mvwaddch(grid, i, j, ACS_VLINE);
         }
      }

      /* Horizontal */
      for (i=6; i<18; i+=6) {
         mvwaddch(grid, i, 0, ACS_LTEE);
         mvwhline(grid, i, 1, ACS_HLINE, 36);
         for (j=12; j<=36; j+=12)
            mvwaddch(grid, i, j, ACS_PLUS);
         mvwaddch(grid, i, 36, ACS_RTEE);
      }
   }

   wrefresh(grid);
   doupdate();
   refresh();
}


static void draw_stats()
{
   int left;
   werase(stats);

   left = 1;
   box(stats, 0, 0);
   mvwaddstr(stats, 1, 1, "Mode: Campaign");
   mvwaddstr(stats, 4, 1, "Difficulty: 10/10");
   mvwprintw(stats, 5, 1, "Numbers:    %2d/81", grid_filled());
   mvwprintw(stats, 6 ,1, "Remaining:  %2d left", 81-grid_filled());
   mvwprintw(stats, 7, 1, "Valid: %s", (grid_valid() ? "Yes":"No"));
   mvwaddstr(stats, 9,1, "Time Taken: 3m 24s");
   mvwhline(stats, 10, 1, ACS_HLINE, 23);
   mvwaddstr(stats, 11,1, " Score:    34327");

   wrefresh(stats);
   doupdate();
   refresh();
}

static void draw_title()
{
   mvwaddstr(title, 0, 5, "Welcome to nsuds: The Ncurses Sudoku System");
   wrefresh(title);
   doupdate();
   refresh();
}

static void draw_xs()
{
   int i;
   refresh();
   for (i=0; i<row; i++)
      mvhline(i, 0, ACS_CKBOARD, col);
   refresh();
}

static void movec(int dir)
{
   int updown=2;
   int leftright=4;
   int first_row = 3;
   int first_col = 30;

   switch (dir) {
      case UP:
         if (cury > first_row)
            cury-=updown;
         break;
      case DOWN:
         if (cury < (first_row + updown * 8))
            cury+=updown;
         break;
      case LEFT:
         if (curx > first_col)
            curx -= leftright;
         break;
      case RIGHT:
         if (curx < (first_col + leftright * 8))
            curx+=leftright;
         break;
      default:
         break;
   }
   move(cury,curx);
   refresh();
}

static void fillch(char ch)
{
   if (ch == '0') 
      mvaddch(cury,curx, ' ');
   else 
      mvaddch(cury, curx, ch);
   grid_data[(cury-3)/2][(curx-30)/4]= ch-'0';
   move(cury,curx);
}

int main(void)
{
   int c;

   init_ncurses();
   init_windows();
   init_grid();
   clear();
   draw_xs();
   draw_title();
   draw_grid();
   start_timer(20, 00);
   draw_stats();
   curx=30;
   cury=3;
   movec(0);
   while ((c = getch())) {
      switch (c) {
         case KEY_LEFT:
         case 'h':
         case 'a':
            movec(LEFT);
            break;

         case KEY_RIGHT:
         case 'l':
         case 'd':
            movec(RIGHT);
            break;
         case KEY_UP:
         case 'k':
         case 'w':
            movec(UP);
            break;
         case KEY_DOWN:
         case 'j':
         case 's':
            movec(DOWN);
            break;
         case 'q':
            werase(grid);
            wnoutrefresh(grid);
            delwin(grid);
            endwin();
            goto done;
         case 'p':
            paused=!paused;
            draw_grid();
            curs_set(!paused);
            break;
         case 'c':
         case KEY_DC:
            fillch('0');
            draw_stats();
            break;
         default:
            if (c>='1' && c<='9') {
               fillch(c);
               draw_stats();
            }
            break;
      }
   }
done:
   exit(0);
}

