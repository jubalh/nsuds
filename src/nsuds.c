/* nsuds - The ncurses sudoku program
 * Text-graphical sudoku with pencil-marking support.
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#if STDC_HEADERS || HAVE_STRING_H
   #include <string.h>
#else /* Old system with only <strings.h> */
   #include <strings.h>
#endif
#include <time.h>
#ifdef HAVE_NCURSES_H
   #include <ncurses.h>
#else 
   #include <curses.h>
#endif
#include <getopt.h>

#include "nsuds.h"
#include "timer.h"
#include "grid.h"
#include "marks.h"
#include "score.h"
#include "scroller.h"
#include "menu.h"

/* FIXME: Let autotools do this, set to /usr/share/doc/nsuds-VERSION/ 
 * or something. */
#define HELPDIR "./helpfiles/"

static void init_ncurses(void);
static void init_windows(void);
static void draw_title(void);
static void draw_xs(void);
static void draw_fbar(void);
static bool launch_confirm(char *question);

WINDOW *grid, *timer, *stats, *title, *fbar, *intro;
static MEVENT mouse_e;
int paused=0, difficulty=0;
int fbar_time = 0;   /* Seconds to keep fbar up */
enum {NEVER, AUTO, ALWAYS} colors_when=AUTO;
int use_colors=0;
/* Display mode:
 *    0) Select Difficulty & intro
 *    1) Grid & timer/stats window */
int dmode=0; 
int row,col;
char *difficulties[] = {"Easy", "Medium", "Hard", "Expert", "Insane",NULL};

/* Set up decent defaults */
static void init_ncurses()
{
   initscr(); /* Enter curses */
   if ((colors_when == AUTO && has_colors()) || colors_when==ALWAYS) {
      use_colors=1;
      start_color(); 
      init_pair(C_INPUT, COLOR_CYAN, COLOR_BLACK);    /* Input and keys */
      init_pair(C_DIALOG, COLOR_WHITE, COLOR_BLUE);   /* Confirm dialog */
      init_pair(C_MARKS1, COLOR_BLACK, COLOR_GREEN);  /* Marks..*/
      init_pair(C_MARKS2, COLOR_BLACK, COLOR_YELLOW);
      init_pair(C_MARKS3, COLOR_BLACK, COLOR_BLUE);
      init_pair(C_URGENT, COLOR_RED, COLOR_BLACK);    /* Urgent text */
   }
   cbreak();      /* Disable line buffering */
   noecho();      /* Don't echo typed chars */
   keypad(stdscr, TRUE); /* Catch special keys */
   mousemask(BUTTON1_CLICKED, NULL); /* Catch left click */
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
   intro = newwin(19, 37, 2, 28);
}


void draw_grid(void)
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
}


void draw_stats(void)
{
   int left;
   werase(stats);

   left = 1;
   box(stats, 0, 0);
   mvwprintw(stats, 1, 1, "Mode: Free play");
   mvwprintw(stats, 2, 1, "Level: %d/30", level);
   mvwprintw(stats, 4, 1, "Difficulty: %s", difficulties[difficulty-1]);
   mvwprintw(stats, 5, 1, "Numbers:    %2d/81", grid_filled());
   mvwprintw(stats, 6 ,1, "Remaining:  %2d left", 81-grid_filled());
   mvwprintw(stats, 8,1, "Time Taken: %dm %2ds", ltime.mins, ltime.secs);
   mvwprintw(stats, 9,1, "Game total: %dh %2dm", gtime.hours, gtime.mins);
   mvwhline(stats, 10, 1, ACS_HLINE, 23);
   mvwprintw(stats, 11,1, " Score:   %d", score);

   wnoutrefresh(stats);
}

void draw_intro(void)
{
   werase(intro);
   box(intro, 0, 0);
   mvwaddstr(intro, 0, 10, "Welcome to nsuds");
   wattrset(intro, COLOR_PAIR(C_KEY));	
   mvwaddstr(intro, 1, 1, "Press '?' while in-game for help");
   wattrset(intro, 0);	
   mvwaddstr(intro, 3, 1, "Nsuds creates valid single solution");
   mvwaddstr(intro, 4, 1, "puzzles of varying difficulies. The");
   mvwaddstr(intro, 5, 1, "difficulty level affects the number");
   mvwaddstr(intro, 6, 1, "of filled in squares you start with");
   mvwaddstr(intro, 7, 1, "as well as the ammount of time you");
   mvwaddstr(intro, 8, 1, "have to complete each level.");
   mvwaddstr(intro, 10, 1, "  - 30 levels in total.");
   mvwaddstr(intro, 11, 1, "  - Full pencil-marking support.");
   mvwaddstr(intro, 12, 1, "  - 100% Free software");
   mvwaddstr(intro, 13, 1, "  - High score tables to come!");
   mvwaddstr(intro, 17, 1, "By Vincent Launchbury et. al. ");

   wnoutrefresh(intro);
}

static void draw_title(void)
{
   werase(title);
   mvwaddstr(title, 0, 5, "Welcome to nsuds: The Ncurses Sudoku System");
   wnoutrefresh(title);
}

/* Add highlighted string, using color if supported */
#define waddhlstr(w, str)                                      \
   do {                                                        \
      wattrset(w, (use_colors?COLOR_PAIR(C_KEY):A_UNDERLINE)); \
      waddstr(w, str);                                         \
      wattrset(w, 0);                                          \
   } while(0)

/* Add highlighted character, using color if supported */
#define waddhlch(w,c) waddch(w, c | (use_colors?COLOR_PAIR(C_KEY):A_UNDERLINE))

/* Draw function bar at bottom of screen
 * Always on the last line, full width */
static void draw_fbar(void)
{
   werase(fbar);
   wmove(fbar, 0, 0);
   /* New game */
   waddhlch(fbar, 'N');
   waddstr(fbar, "ew ");
   /* Pause */
   waddhlch(fbar, 'P');
   waddstr(fbar, "ause ");
   /* Quit */
   waddhlch(fbar, 'Q');
   waddstr(fbar, "uit | ");

   /* 1-9 add number */
   waddstr(fbar, "Add:");
   waddhlstr(fbar, "1-9");

   /* DEL/C remove number */
   waddstr(fbar, " Del:");
   waddhlstr(fbar, "DEL/X");

   /* Move */
   waddstr(fbar, " Move:");
   waddhlstr(fbar, "Arrows/WASD/HJKL/Click");
   wnoutrefresh(fbar);

}

void hide_fbar(void)
{
   fbar_time=0;
   werase(fbar);
   mvwhline(fbar, 0, 0, ACS_CKBOARD, col);
   wrefresh(fbar);
}


static void draw_xs(void)
{
   int i;
   werase(grid);
   /* FIXME: Makes redrawing VERY slow, 
    * it seems xterm draws characters very slowly, but 
    * black space is cheap. Switching windows with nsuds
    * large is hideously slow! */
   for (i=0; i<row; i++)
      mvhline(i, 0, ACS_CKBOARD, col);
   wnoutrefresh(stdscr);
}

void draw_all(void)
{
   switch (dmode) {
      case 0:
         clear();
         draw_xs();
         draw_title();
         draw_intro();
         difficulty = launch_menu(19, 25, 2, 1, 
                    "Select difficulty", difficulties);
         movec(CUR);
         doupdate();
         break;
      case 1:
         clear();
         draw_xs();
         draw_title();
         draw_timer();
         draw_grid();
         draw_stats();
         movec(CUR);
         doupdate();
         break;
   }
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
   wbkgd(confirm, COLOR_PAIR(C_DIALOG));
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
            fbar_time=0;
            return status;
         default:
            break;
      }
   }
   return 0;
}


/* Draw menu to select difficulty */
void new_game(void) {
   /* Set to main-menu */
   dmode=0;
   /* Pause */
   paused=1;
   curs_set(0);
   /* Draw everything, wait for difficulty input */
   draw_all();
   /* Start a game */
   dmode=1;
   new_level();
}

/* Start a new level */
void new_level(void) 
{
   int i;

   /* Clear all marks */
   for (i=0; i<729; i++) *(&marks[0][0][0] + i) = 0;
   for (i=0;i<3;i++) showmarks[i]=0;

   /* Start a new game */
   generate();
   switch (difficulty) {
      case EASY:
         start_timer(20, 0);
         break;
      case MEDIUM:
         start_timer(17, 30);
         break;
      case HARD:
         start_timer(15, 0);
         break;
      case EXPERT:
         start_timer(10, 0);
         break;
      case INSANE:
         start_timer(7, 30);
         break;
   }
   paused = 0;
   curs_set(1);
   draw_all();
}


int main(int argc, char **argv)
{
   int c;
   int opt, opti;
   static struct option long_opts[] =
   {
      {"color",     optional_argument, 0, 'c'},
      {"no-color",  no_argument,       0, 'C'},
      {"help",      no_argument,       0, 'h'},
      {"version",   no_argument,       0, 'v'},
      {0, 0, 0, 0}
   };

   /* Parse arguments */
   while ((opt = getopt_long_only(argc, argv, "hvc::C", long_opts, &opti))) {
      if (opt == EOF) break;
      switch (opt) {
         case 'c':
            if (!optarg) break; /* AUTO is default */
            if (!strcmp(optarg, "never") || !strcmp(optarg, "no")) {
               colors_when = NEVER;
            } else if (!strcmp(optarg, "always") || !strcmp(optarg, "yes")) {
               colors_when=ALWAYS;
            } else {
               fprintf(stderr, "Error: Invalid option to --color, `%s'\n",
                  optarg);
               exit(EXIT_FAILURE);
            }
            break;
         case 'C':
            colors_when = NEVER;
            break;
         case 'h':
           fputs("Usage: nsuds [OPTIONS]...\n\
Nsuds: The Ncurses Sudoku System\n\
   -c --color[=WHEN] Control when to use colors. WHEN may be `never', `auto'\n\
                       or `always'. Defaults to `auto' \n\
   -C --no-color     Synonym for --color=never\n\
   -h --help         Show this help screen\n\
   -v --version      Print version info\n\
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

   /* Start the game */
   new_game();

   
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
            break;
         case KEY_RIGHT:
         case 'L':
         case 'l':
         case 'D':
         case 'd':
            movec(RIGHT);
            break;
         case KEY_UP:
         case 'K':
         case 'k':
         case 'W':
         case 'w':
            movec(UP);
            break;
         case KEY_DOWN:
         case 'J':
         case 'j':
         case 'S':
         case 's':
            movec(DOWN);
            break;
         case KEY_HOME:
            movec(HOME);
            break;
         case KEY_END:
            movec(END);
            break;
         case KEY_NPAGE:
            movec(BOTTOM);
            break;
         case KEY_PPAGE:
            movec(TOP);
            break;
         case 'Q':
         case 'q':
            if (launch_confirm("Really quit?")) {
               endwin();
               goto done;
            }
            break;

         /* Help */
         case '?':
            if (!paused)  {
               paused=1;
               draw_grid();
               doupdate();
               curs_set(!paused);
               movec(CUR);
            }
            launch_file(HELPDIR "main", "Help with nsuds");
            paused=0;
            curs_set(!paused);
            draw_all();
            break;
         case 'P':
         case 'p':
            paused=!paused;
            draw_grid();
            doupdate();
            curs_set(!paused);
            movec(CUR);
            break;
         case 'X':
         case 'x':
         case KEY_DC:
            gaddch('0');
            draw_grid();
            draw_stats();
            doupdate();
            movec(CUR);
            break;
         /* New game, in freeplay */
         case 'N':
         case 'n':
            if (launch_confirm("End current game and start a fresh?")) {
               game_over();
            }
            break;

         /* Marking Tools */
         case 'm':
            mark_square();
            break;
         case 'r':
            marks_show(ONE);
            break;
         case 'R':
            marks_show(MULTIPLE);
            break;
         case 'c':
            marks_clear(SINGLE);
            break;
         case 'C':
            marks_clear(ALL);
            break;

         /* Catch mouse events */
         case KEY_MOUSE:
            if (getmouse(&mouse_e) == OK) {
               /* Left click selects square */
               if (mouse_e.bstate & BUTTON1_CLICKED) {
                  movec_mouse(mouse_e.x, mouse_e.y);
               }
            }
            break;
         case KEY_RESIZE:
            getmaxyx(stdscr, row, col);
            fbar_time=0;
            delwin(fbar);
            fbar = newwin(1, col, row-1, 0);
            draw_all();
            break;
         default:
            /* Handle number input */
            if (c>='1' && c<='9') {
               if (!paused) {
                  gaddch(c);
                  draw_stats();
                  doupdate();
                  movec(CUR);
               }
            /* Key unknown, show function bar */
            } else if (!fbar_time) {
               draw_fbar();
               doupdate();
               movec(CUR);
               fbar_time = 5;
            }
            break;
      }
   }
done:
   exit(0);
}

