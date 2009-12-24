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
#define _XOPEN_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#ifdef HAVE_NCURSES_H
   #include <ncurses.h>
#else 
   #include <curses.h>
#endif
#include <errno.h>
#include <err.h>

#include "nsuds.h"
#include "timer.h"
#include "grid.h"
#include "util.h"


/* "Font" for numbers taken from htop, by Hisham Muhammad */
static char *timer_digits[3][10] = {
   { " __ ","    "," __ "," __ ","    "," __ "," __ "," __ "," __ "," __ "},
   { "|  |","   |"," __|"," __|","|__|","|__ ","|__ ","   |","|__|","|__|"},
   { "|__|","   |","|__ "," __|","   |"," __|","|__|","   |","|__|"," __|"}
};
static char *empty="   ";
struct ltimer cdown, ltime={0,0};
struct gtimer gtime={0,0, 0};

/* Start a new timer for a level,
 * cancels any old timers */
void start_timer(int mins, int secs)
{
   struct sigaction new;

   /* Setup countdown */
   cdown.mins = mins;
   cdown.secs = secs;
   /* Reset level timer */
   ltime.mins = ltime.secs = 0;

   /* Set up signal handler */
   new.sa_handler = catch_alarm;
   sigemptyset(&new.sa_mask);
   new.sa_flags = 0;
   
#ifdef SA_RESTART
   /* Restart interrupted system calls */
   new.sa_flags |= SA_RESTART;
#endif
   
   if (sigaction(SIGALRM, &new, NULL) < 0)
      err(errno, "Can't set up signal handler");

   /* Update timer */
   draw_timer();
   alarm(1);
}

/* Called every second when in game (unless paused).
 * Update game timers, and handle the fbar timeout */
void catch_alarm(unused int sig)
{
   alarm(1);

   /* If function bar is shown, countdown it's timer
    * and erase when it expires */
   if (fbar_time && !(--fbar_time)) {
      hide_fbar();
   }

   /* Don't countdown if paused */
   if (paused) return;

   /* Decrement level timer */
   if (cdown.secs > 0) {
      cdown.secs--;
   } else {
      cdown.mins--;
      cdown.secs=59;
   }

   /* Increment level time */
   if (ltime.secs < 59) {
      ltime.secs++;
   } else {
      ltime.mins++;
      ltime.secs=0;
   }

   /* Increment total game time */
   if (gtime.secs < 59) {
      gtime.secs++;
   } else if (gtime.mins < 59) {
      gtime.mins++;
      gtime.secs=0;
   } else {
      gtime.mins = gtime.secs = 0;
      gtime.hours++;
   }

   /* If timer reaches 0 */
   if (!cdown.secs && !cdown.mins) {
      alarm(0);
      game_over();
      return;
   }

   draw_timer();
   draw_stats();
   movec(CUR);
   doupdate();
}


/* Get each line of timer digits in ascii font */
#define time2strs(line) t2s(line-1, cdown.mins, cdown.secs)
/* Shorthand */
#define TD timer_digits
/* If time < 10 mins, first 0 digit isn't shown */
#define FTD(l, m) m>=10 ? TD[l][m/10] : empty
/* t2s(line, mins, seconds): Get ascii strings from time */
#define t2s(l,m,s) FTD(l, m), TD[l][m%10], TD[l][s/10], TD[l][s%10]

/* Draw timer window, with updated timers */
void draw_timer()
{
   int left;
   werase(timer);

   if (cdown.mins > 19) left = 2;
   else left = 1;
   box(timer, 0, 0);
   mvwaddstr(timer, 1, 5, "Time Remaining");
   mvwprintw(timer, 2, left, "%s%s   %s%s", time2strs(1));
   mvwprintw(timer, 3, left, "%s%s . %s%s", time2strs(2));
   mvwprintw(timer, 4, left, "%s%s . %s%s", time2strs(3));

   wnoutrefresh(timer);
}

