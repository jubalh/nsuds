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
#include <ncurses.h>

static int paused=0;
static int colors=0;
static int row,col;
static WINDOW *grid, *timer, *stats, *title;

static void init_ncurses(void)
{
   initscr(); /* Enter curses */
   if (has_colors()) {
      start_color(); 
   }

   init_pair(1, COLOR_BLUE, COLOR_BLACK);
   cbreak();      /* Disable line buffering */
   noecho();      /* Don't echo typed chars */
   curs_set(0);   /* Hide cursor */
   keypad(stdscr, TRUE); /* Catch special keys */
   getmaxyx(stdscr, row, col);
   refresh();
}

static void init_windows(void) 
{
   title = newwin(1, 64, 0, 1);
   grid=newwin(19, 37, 2, 28);
   timer = newwin(5, 25, 2, 1);
   stats = newwin(14, 25, 7, 1);
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

static void draw_timer()
{
   int left;

   left = 1;
   box(timer, 0, 0);
   mvwaddstr(timer, 1, 5, "Time Remaining");

   wrefresh(timer);
   doupdate();
   refresh();
}

static void draw_stats()
{
   int left;

   left = 1;
   box(stats, 0, 0);
   mvwaddstr(stats, 1, 1, "Mode: Campaign");
   mvwaddstr(stats, 3, 1, "Skill:   10");
   mvwaddstr(stats, 4, 1, "Numbers: x/81 (x left)");
   mvwhline(stats, 11, 1, ACS_HLINE, 23);
   mvwaddstr(stats, 12,1, " Score:    34327");

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
   int i,j;
   refresh();
   for (i=0; i<row; i++)
      mvhline(i, 0, ACS_CKBOARD, col);
   refresh();
}

int main(void)
{
   char c;

   init_ncurses();
   init_windows();
   clear();
   draw_xs();
   draw_title();
   draw_grid();
   draw_timer();
   draw_stats();
   while ((c = getch())) {
      switch (c) {
         case 'q':
            werase(grid);
            wnoutrefresh(grid);
            delwin(grid);
            endwin();
            goto done;
         case 'p':
            paused=!paused;
            draw_grid();
            break;
         default:
            break;
      }
   }
done:
   exit(0);
}

