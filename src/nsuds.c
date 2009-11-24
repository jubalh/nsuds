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
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <ncurses.h>
#include <getopt.h>

#include "config.h"
#include "nsuds.h"
#include "timer.h"
#include "grid.h"

static void init_ncurses(void);
static void init_windows(void);
static void draw_grid(void);
static void draw_title(void);
static void draw_xs(void);
static void draw_all(void);
static void draw_fbar(void);
static bool launch_confirm(char *question);

WINDOW *grid, *timer, *stats, *title, *fbar;
int paused=0, difficulty=0;
bool campaign=0;
int score=0;
int fbar_time = 0;   /* Seconds to keep fbar up */
static int colors=0;
int row,col;

/* Set up decent defaults */
static void init_ncurses(void)
{
   initscr(); /* Enter curses */
   if (has_colors()) {
      start_color(); 
      init_pair(1, COLOR_CYAN, COLOR_BLACK);  /* Filled numbers/Fbar */
      init_pair(2, COLOR_WHITE, COLOR_RED);   /* G/O Screen */
      init_pair(3, COLOR_WHITE, COLOR_GREEN); /* Win Screen */
      init_pair(4, COLOR_WHITE, COLOR_BLUE);  /* Confirm dialog */
   }
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
   fbar = newwin(1, col, row-1, 0);
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
      for (i=12; i<36; i+=12) {
         mvwaddch(grid, 0, i, ACS_TTEE);
         mvwvline(grid, 1, i, ACS_VLINE, 17);
         mvwaddch(grid, 18,i, ACS_BTEE);
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


void draw_stats(void)
{
   int left;
   werase(stats);

   left = 1;
   box(stats, 0, 0);
   mvwprintw(stats, 1, 1, "Mode: %s", campaign?"Campaign":"Free Play");
   if (campaign) mvwaddstr(stats, 2, 1, "Level: 1");
   mvwprintw(stats, 4, 1, "Difficulty: %s", "Medium");
   mvwprintw(stats, 5, 1, "Numbers:    %2d/81", grid_filled());
   mvwprintw(stats, 6 ,1, "Remaining:  %2d left", 81-grid_filled());
   mvwprintw(stats, 8,1, "Time Taken: %dm %2ds", ltime.mins, ltime.secs);
   mvwprintw(stats, 9,1, "Game total: %dh %2dm", gtime.hours, gtime.mins);
   mvwhline(stats, 10, 1, ACS_HLINE, 23);
   mvwprintw(stats, 11,1, " Score:   %d", score);

   wnoutrefresh(stats);
   movec(CUR);
}

static void draw_title(void)
{
   werase(title);
   mvwaddstr(title, 0, 5, "Welcome to nsuds: The Ncurses Sudoku System");
   wnoutrefresh(title);
}

/* Add highlighted string */
#define waddhlstr(w, str)         \
   do {                           \
      wattrset(w, COLOR_PAIR(1)); \
      waddstr(w, str);            \
      wattrset(w, 0);             \
   } while(0)

/* Draw function bar at bottom of screen
 * Always on the last line, full width */
static void draw_fbar(void)
{
   werase(fbar);
   wmove(fbar, 0, 0);
   /* New game */
   waddch(fbar, 'N' | COLOR_PAIR(1));
   waddstr(fbar, "ew ");
   /* Pause */
   waddch(fbar, 'P' | COLOR_PAIR(1));
   waddstr(fbar, "ause ");
   /* Quit */
   waddch(fbar, 'Q' | COLOR_PAIR(1));
   waddstr(fbar, "uit | ");

   /* 1-9 add number */
   waddstr(fbar, "Add:");
   waddhlstr(fbar, "1-9  ");

   /* DEL/C remove number */
   waddstr(fbar, "Del:");
   waddhlstr(fbar, "DEL/C  ");

   /* Move */
   waddstr(fbar, "Move:");
   waddhlstr(fbar, "Arrows/WASD/HJKL ");
   wnoutrefresh(fbar);

   /* Restore Cursor*/
   movec(CUR);
}

static void draw_xs(void)
{
   int i;
   werase(grid);
   for (i=0; i<row; i++)
      mvhline(i, 0, ACS_CKBOARD, col);
   wnoutrefresh(stdscr);
}

static void draw_all(void)
{
   clear();
   draw_xs();
   draw_title();
   draw_timer();
   draw_grid();
   draw_stats();
   movec(CUR);
   doupdate();
}

/* Launch a dialog that asks OK/Cancel for a question,
 * pausing the game while it waits for input */
static bool launch_confirm(char *question)
{
   int c;
   bool status=false;
   WINDOW *confirm = newwin(row * 0.4, col * 0.7, row * 0.3, col * 0.15);

   alarm(0); /* Cancel alarm */
   /* Pause */
   paused=1;
   draw_grid();
   doupdate();
   curs_set(!paused);

   /* Draw dialog */
   wbkgd(confirm, COLOR_PAIR(4));
   box(confirm, 0, 0);
   mvwaddstr(confirm, 0, col * 0.35 - (strlen("Confirm..") / 2), "Confirm");
   mvwaddstr(confirm, 2, col * 0.35 - (strlen(question) / 2), question);
   wattrset(confirm, A_REVERSE);
   mvwaddstr(confirm, (row * 0.4) - 3, col * 0.35, " Cancel ");
   wattroff(confirm, A_REVERSE);
   mvwaddstr(confirm, (row * 0.4) - 3, col * 0.35 - 9, "   OK   ");
   wrefresh(confirm);
   overwrite(confirm, stats);
   while ((c = getch())!=ERR) {
      switch (c) {
         case KEY_RESIZE:
            werase(confirm);
            wnoutrefresh(confirm);
            delwin(confirm);
            getmaxyx(stdscr, row, col);
            draw_all();
            return launch_confirm(question);
         case KEY_LEFT:
         case KEY_RIGHT:
            wattrset(confirm, A_REVERSE);	
            if (!status) {
               mvwaddstr(confirm, (row * 0.4) -3, col *0.35 - 9, "   OK   ");
               wattroff(confirm, A_REVERSE);
               mvwaddstr(confirm, (row * 0.4) -3, col *0.35, " Cancel ");
            } else {
               mvwaddstr(confirm, (row * 0.4) -3, col *0.35, " Cancel ");
               wattroff(confirm, A_REVERSE);
               mvwaddstr(confirm, (row * 0.4) -3, col *0.35 - 9, "   OK   ");
            }
            status = !status;
            wrefresh(confirm);
            break;				
         /* Enter pressed */
         case 10:
            wbkgd(confirm, COLOR_PAIR(0));
            werase(confirm);
            wnoutrefresh(confirm);
            delwin(confirm);
            paused=0;
            curs_set(!paused);
            draw_all();
            catch_alarm(0);
            return status;
         default:
            break;
      }
   }
   return 0;
}


/* Draw game over screen */
void game_over(void)
{
   int c;
   WINDOW *go;
   /* Pause clock */
   paused = 1;
   /* Draw game over windown */
   go = newwin(10, 44, 5, 12);
   wattrset(go, COLOR_PAIR(2));
   wbkgd(go, COLOR_PAIR(2));
   werase(go);
   mvwprintw(go, 0, 15, "!GAME OVER!");
   mvwprintw(go, 2, 1, "You failed to complete the puzzle in time. "
      "Keep practicing, to improve your skills, and don't forget "
      "to use 'p' to pause the game.");
   mvwprintw(go, 6, 10, "Total score: %d", score);
   mvwprintw(go, 8, 5, "Press any key to start a new game");
   wrefresh(go);

   /* Wait for input */
   while ((c = getch()) == ERR) ;

   /* Start a new game */
   score = 0;
   gtime.hours = gtime.mins = 0;
   generate();
   start_timer(20, 0);
   paused = !paused;
   draw_all();
}

/* Draw level win screen */
void game_win(void)
{
   int c;
   WINDOW *go;
   /* Pause clock */
   paused = 1;
   /* Draw level win  window */
   go = newwin(12, 44, 5, 12);
   wattrset(go, COLOR_PAIR(3));
   wbkgd(go, COLOR_PAIR(3));
   werase(go);
   mvwprintw(go, 0, 15, "Congratulations");
   mvwprintw(go, 2, 1, "You successfully completed the puzzle! "
      "You finished the game with %d minutes and %d seconds remaining.",
     cdown.mins, cdown.secs );
   mvwprintw(go, 6, 7, "Level Score = %d", 
      (cdown.mins * 60 + cdown.secs) * (1+difficulty/50));
   score += (cdown.mins * 60 + cdown.secs) * (1+difficulty/100);
   mvwprintw(go, 7, 7, "Total Score = %d", score);

   mvwprintw(go, 9, 5, "Press any key to start a new game");
   wrefresh(go);

   /* Wait for input */
   while ((c = getch()) == ERR) ;

   /* Start a new game */
   generate();
   start_timer(20, 0);
   paused = !paused;
   draw_all();
}


int main(int argc, char **argv)
{
   int c;
   int opt, opti;
   static struct option long_opts[] =
   {
      {"help",      no_argument,       0, 'h'},
      {"version",   no_argument,       0, 'v'},
      {0, 0, 0, 0}
   };

   /* Parse arguments */
   while ((opt = getopt_long_only(argc, argv, "hv", long_opts, &opti))) {
      if (opt == EOF) break;
      switch (opt) {
         case 'h':
           fputs("Usage: nsuds [OPTIONS]...\n\
Nsuds: The Ncurses Sudoku System\n\
   -h --help      Show this help screen\n\
   -v --version   Print version info\n\
Report bugs to <" PACKAGE_BUGREPORT ">\n\
Home Page: http://www.sourceforge.net/projects/nsuds/\n",
             stdout);
            exit(EXIT_SUCCESS);
         case 'v':
            fputs("nsuds version " VERSION "\n\
Copyright (C) Vincent Launchbury 2008,2009.\n\
License GPLv2+: GNU GPL version 2 or later.\n\
This is free software, you are free to modify and redistribute it.\n\n\
Send bug reports to <" PACKAGE_BUGREPORT ">\n",
               stdout);
            exit(EXIT_SUCCESS);
      }
   }

   /* Setup ncurses and windows */
   init_ncurses();
   init_windows();
   generate();
   start_timer(20, 0);
   draw_all();
   
   /* Main input loop */
   while ((c = getch())) {
      switch (c) {
         case ERR:
            break;
         case KEY_LEFT:
         case 'H':
         case 'h':
         case 'A':
         case 'a':
            movec(LEFT);
            doupdate();
            break;
         case KEY_RIGHT:
         case 'L':
         case 'l':
         case 'D':
         case 'd':
            movec(RIGHT);
            doupdate();
            break;
         case KEY_UP:
         case 'K':
         case 'k':
         case 'W':
         case 'w':
            movec(UP);
            doupdate();
            break;
         case KEY_DOWN:
         case 'J':
         case 'j':
         case 'S':
         case 's':
            movec(DOWN);
            doupdate();
            break;
         case 'Q':
         case 'q':
            if (launch_confirm("Really quit?")) {
               werase(grid);
               delwin(grid);
               endwin();
               goto done;
            }
            break;
         case 'P':
         case 'p':
            paused=!paused;
            draw_grid();
            doupdate();
            curs_set(!paused);
            break;
         case 'C':
         case 'c':
         case KEY_DC:
            gaddch('0');
            draw_grid();
            draw_stats();
            doupdate();
            break;
         /* New game, in freeplay */
         case 'N':
         case 'n':
            if (campaign) break;
            if (launch_confirm("End current game and start a fresh?")) {
               score = 0;
               gtime.hours = gtime.mins = 0;
               generate();
               start_timer(20, 0);
               draw_grid();
               draw_stats();
            }
            break;
#if 0
         case 'm':
            campaign=!campaign;
            start_timer(20, 0);
            draw_grid();
            draw_stats();
#endif
         case KEY_RESIZE:
            getmaxyx(stdscr, row, col);
            draw_all();
            break;
         default:
            /* Handle number input */
            if (c>='1' && c<='9') {
               if (!paused) {
                  gaddch(c);
                  draw_stats();
                  doupdate();
               }
            /* Key unknown, show function bar */
            } else if (!fbar_time) {
               draw_fbar();
               doupdate();
               fbar_time = 5;
            }
            break;
      }
   }
done:
   exit(0);
}
