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

#define _XOPEN_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
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
#include <errno.h>
#include <err.h>

#include "nsuds.h"
#include "timer.h"
#include "grid.h"
#include "marks.h"
#include "score.h"
#include "scroller.h"
#include "menu.h"
#include "highscores.h"
#include "dialog.h"

static void init_ncurses(void);
static void init_windows(void);
static void draw_title(void);
static void draw_xs(void);
static void draw_fbar(void);
static void init_signals(void);
void catch_signal(int sig);

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
int scrl_open=0; /* Is a scroller open? */
char *difficulties[] = {"Easy", "Medium", "Hard", "Expert", "Insane",NULL};
char level_times[][2] = {{20,0}, 
                         {17,30},
                         {15,0},
                         {10,0},
                         {7, 30}};

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

/* Set up every window we might use. Doesn't include Menus or
 * Scrollers as they're managed automatically. */
static void init_windows(void)
{
   title = newwin(1, 64, 0, 1);
   grid=newwin(19, 37, 2, 28);
   timer = newwin(6, 25, 2, 1);
   stats = newwin(13, 25, 8, 1);
   fbar = newwin(1, col, row-1, 0);
   intro = newwin(19, 37, 2, 28);
}

/* Set up all the signal handlers */
static void init_signals(void)
{
   struct sigaction new;

   /* Set up signal handler */
   new.sa_handler = catch_signal;
   sigemptyset(&new.sa_mask);
   new.sa_flags = 0;
   
#ifdef SA_RESTART
   /* Restart interrupted system calls */
   new.sa_flags |= SA_RESTART;
#endif
   if (sigaction(SIGINT, &new, NULL) < 0  ||
       sigaction(SIGTERM, &new, NULL) < 0 || 
       sigaction(SIGQUIT, &new, NULL) < 0 || 
       sigaction(SIGILL, &new, NULL) < 0  || 
       sigaction(SIGSEGV, &new, NULL) < 0)
     err(errno, "Can't set up signal handlers!");
}


/* Handle all the signals */
void catch_signal(int sig)
{
   switch (sig) {
      case SIGINT:
         /* Interrupt acts similar to the 'q' key */
         if (!dmode || confirm("Really quit?")) {
            endwin();
            exit(EXIT_SUCCESS);
         }
         /* Redraw help if user cancelled quit */
         if (scrl_open) ungetch(KEY_RESIZE);
         break;
      case SIGILL:
      case SIGSEGV:
         /* Die nicely from a fatal error */
         endwin();
         errx(EXIT_FAILURE, "Segmentation fault!");
      case SIGQUIT:
      case SIGTERM:
         /* Exit nicely from a kill */
         endwin();
         exit(EXIT_FAILURE);
   }
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
   mvwprintw(stats, 1, 1, "Level:      %d/30", level);
   mvwprintw(stats, 2, 1, "Difficulty: %s", difficulties[difficulty-1]);
   mvwprintw(stats, 4, 1, "Numbers:    %2d/81", grid_filled());
   mvwprintw(stats, 5 ,1, "Remaining:  %2d left", 81-grid_filled());
   mvwprintw(stats, 6 ,1, "Percent:    %-2.1f%%", ((double)grid_filled()/81)*100);
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
   mvwaddstr(intro, 1, 1, "Press '?' at any time for help");
   wattrset(intro, 0);	

   mvwaddstr(intro, 3, 1, "Nsuds is a curses sudoku game that");
   mvwaddstr(intro, 4, 1, "generates single-solution puzzles ");
   mvwaddstr(intro, 5, 1, "of varying difficulties. It was");
   mvwaddstr(intro, 6, 1, "designed to be small, portable and");
   mvwaddstr(intro, 7, 1, "efficient, with few depedencies.");
   mvwaddstr(intro, 9, 1, "Currently in beta, nsuds features:");
   mvwaddstr(intro, 10, 1, " - Multiple levels");
   mvwaddstr(intro, 11, 1, " - Varying difficulties");
   mvwaddstr(intro, 12, 1, " - Full pencil-marking support");
   mvwaddstr(intro, 13, 1, " - 100% Free software");
   mvwaddstr(intro, 14, 1, " - Press 'H' to view high scores");
   mvwaddstr(intro, 17, 1, "   By Vincent Launchbury et. al. ");

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

   /* DEL/X remove number */
   waddstr(fbar, " Del:");
   waddhlstr(fbar, "DEL/x");

   /* Move */
   waddstr(fbar, " Move:");
   waddhlstr(fbar, "Arrows/Mouse/hjkl");

   /* Help */
   waddstr(fbar, " Help:");
   waddhlstr(fbar, "?");
   wnoutrefresh(fbar);

}

void hide_fbar(void)
{
   fbar_time=0;
   werase(fbar);
   if (row <= 30) mvwhline(fbar, 0, 0, ACS_CKBOARD, col);
   else mvwhline(fbar, 0, 0, ' ', col);
   wrefresh(fbar);
}

/* Higher level getch that converts ESC+key to the meta value */
int getkey(void)
{
   int c;
   while ((c=getch())) {
      switch (c) {
         /* Escape (meta sequence) */
         case 27: 
            while ((c=getch())) {
               switch (c) {
                  case 27: /* Escape */
                  case ERR:
                     continue;
                  default:
                     return ALT(c);
               }
            }
         /* Don't return for this! */
         case ERR:
            continue;
         /* Regular key, return */
         default:
            return c;
      }
   }
}


void unknown_key(void)
{
   if (!fbar_time) {
      draw_fbar();
      doupdate();
      movec(CUR);
      fbar_time = 5;
   }
}

static void draw_xs(void)
{
   int i;
   erase();
   for (i=0; i<30; i++)
      mvhline(i, 0, ACS_CKBOARD, 90);
   wnoutrefresh(stdscr);
}

void draw_all(void)
{
   switch (dmode) {
      case 0:
         delwin(intro);
         intro = newwin(19, 37, 2, 28);
         draw_xs();
         draw_title();
         draw_intro();
         movec(CUR);
         break;
      case 1:
         /* We have to do this because otherwise, if any of the windows are too
          * large for the screen, and then the screen is enlarged, ncurses
          * messes up the heights. */
         delwin(title);
         delwin(grid);
         delwin(timer);
         delwin(stats);
         delwin(fbar);
         title = newwin(1, 64, 0, 1);
         grid=newwin(19, 37, 2, 28);
         timer = newwin(6, 25, 2, 1);
         stats = newwin(13, 25, 8, 1);
         fbar = newwin(1, col, row-1, 0);
         draw_xs();
         draw_title();
         draw_timer();
         draw_grid();
         draw_stats();
         if (!scrl_open) doupdate();
         if (!scrl_open) movec(CUR);
         break;
   }
}

/* Draw menu to select difficulty */
void new_game(void)
{
   /* Set to main-menu */
   dmode=0;
   /* Pause */
   paused=1;
   curs_set(0);

   /* Start a blank timer so that the fbar works */
   start_timer(0,0);

   /* Draw windows */
   draw_all();
   /* Wait for user to select difficulty */
   difficulty = launch_menu(19, 25, 2, 1, 
              "Select difficulty", difficulties, difficulty);

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
   start_timer(level_times[difficulty-1][0], level_times[difficulty-1][1]);
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
   init_signals();

   /* Start the game */
   new_game();

   
   /* Main input loop */
   while ((c = getkey())) {
      switch (c) {
         case KEY_LEFT:
         case 'h':
         case 'a':
         case CTRL('b'):
            movec(LEFT);
            break;
         case KEY_RIGHT:
         case 'l':
         case 'd':
         case CTRL('f'):
            movec(RIGHT);
            break;
         case KEY_UP:
         case 'k':
         case 'w':
         case CTRL('u'):
         case CTRL('y'):
         case CTRL('p'):
            movec(UP);
            break;
         case KEY_DOWN:
         case 'j':
         case 's':
         case CTRL('d'):
         case CTRL('e'):
         case CTRL('n'):
            movec(DOWN);
            break;
         case KEY_HOME:
         case 'I':
         case '0':
         case ALT('a'):
            movec(HOME);
            break;
         case KEY_END:
         case 'A':
         case '$':
         case ALT('e'):
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
            if (confirm("Really quit?")) {
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
            scrl_open=1;
            launch_file(HELPDIR "main", "Help with nsuds");
            scrl_open=0;
            paused=0;
            curs_set(!paused);
            draw_all();
            break;
         /* High scores */
         case 'H':
            if (!paused)  {
               paused=1;
               draw_grid();
               doupdate();
               curs_set(!paused);
               movec(CUR);
            }
            scrl_open=1;
            display_scores();
            scrl_open=0;
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
         case 'x':
         case KEY_DC:
            gsetcur(0);
            draw_grid();
            draw_stats();
            doupdate();
            movec(CUR);
            break;
         /* New game, in freeplay */
         case 'n':
         case 'N':
            if (confirm("End current game and start a fresh?")) {
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
#ifdef DEBUG
         /* Very useful for debugging */
         case 'z':
            game_win();
            break;
         case 'Z':
            game_over();
            break;
         /* Testing string input */
         case 'B':
            getstring("Please enter your name:");
            break;
#endif

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
                  gsetcur(c-'0');
                  draw_stats();
                  doupdate();
                  movec(CUR);
               }
            /* Key unknown, show function bar */
            } else {
               unknown_key();
            }
            break;
      }
   }
done:
   exit(0);
}

