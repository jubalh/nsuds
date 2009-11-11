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
#include <time.h>
#include <ncurses.h>

#include "nsuds.h"
#include "timer.h"
#include "grid.h"

static void init_ncurses(void);
static void init_windows(void);
static void draw_grid(void);
static void draw_stats();
static void draw_title();
static void draw_xs();

WINDOW *grid, *timer, *stats, *title;
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
   /* It seems that there's a bug in ncurses where
    * anything written to the virtual screen before 
    * the very first call to refresh() will sometimes 
    * be discarded, depending on a lot of things..
    *
    * Causes a lot of random weirdness otherwise,
    * so please don't remove. */
   refresh();
}

static void init_windows(void) 
{
   title = newwin(1, 64, 0, 1);
   grid=newwin(19, 37, 2, 28);
   timer = newwin(6, 25, 2, 1);
   stats = newwin(13, 25, 8, 1);
}


static void draw_grid(void)
{
   int i, j;
   int left;

   left = 28;
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

   wnoutrefresh(grid);
   draw_grid_contents();
   movec(CUR);
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

   wnoutrefresh(stats);
   movec(CUR);
}

static void draw_title()
{
   mvwaddstr(title, 0, 5, "Welcome to nsuds: The Ncurses Sudoku System");
   wnoutrefresh(title);
}

static void draw_xs()
{
   int i;
   for (i=0; i<row; i++)
      mvhline(i, 0, ACS_CKBOARD, col);
   wnoutrefresh(stdscr);
}


int main(void)
{
   int c;

   init_ncurses();
   init_windows();
   clear();
   draw_xs();
   draw_title();
   generate(30);
   draw_grid();
   start_timer(20, 00);
   draw_stats();
   movec(CUR);
   doupdate();
   
   while ((c = getch())) {
      switch (c) {
         case KEY_LEFT:
         case 'h':
         case 'a':
            movec(LEFT);
            doupdate();
            break;
         case KEY_RIGHT:
         case 'l':
         case 'd':
            movec(RIGHT);
            doupdate();
            break;
         case KEY_UP:
         case 'k':
         case 'w':
            movec(UP);
            doupdate();
            break;
         case KEY_DOWN:
         case 'j':
         case 's':
            movec(DOWN);
            doupdate();
            break;
         case 'q':
            werase(grid);
            delwin(grid);
            endwin();
            goto done;
         case 'p':
            paused=!paused;
            draw_grid();
            doupdate();
            curs_set(!paused);
            break;
         case 'c':
         case KEY_DC:
            gaddch('0');
            draw_stats();
            doupdate();
            break;
         default:
            if (c>='1' && c<='9') {
               gaddch(c);
               draw_stats();
               doupdate();
            }
            break;
      }
   }
done:
   exit(0);
}

