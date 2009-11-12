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
#define _XOPEN_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <ncurses.h>
#include <errno.h>
#include <err.h>

#include "nsuds.h"
#include "timer.h"


/* "Font" for numbers taken from htop, by Hisham Muhammad */
static char *timer_digits[3][10] = {
   { " __ ","    "," __ "," __ ","    "," __ "," __ "," __ "," __ "," __ "},
   { "|  |","   |"," __|"," __|","|__|","|__ ","|__ ","   |","|__|","|__|"},
   { "|__|","   |","|__ "," __|","   |"," __|","|__|","   |","|__|"," __|"}
};
static char *empty="   ";

static struct {
   int mins;
   int secs;
} cdown; /* cdown */

void start_timer(int mins, int secs)
{
   struct sigaction new;

   cdown.mins = mins;
   cdown.secs = secs;

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

void catch_alarm(int sig)
{
   alarm(1);
   if (paused || (!cdown.secs && !cdown.mins)) return;

   if (cdown.secs > 0) {
      cdown.secs--;
   } else {
      cdown.mins--;
      cdown.secs=59;
   }

   draw_timer();
}

#define time2strs(line) digits2str(line, 1), digits2str(line, 2), \
   digits2str(line, 3), digits2str(line, 4)

static char *digits2str(int line, int dig) {
   /* dig = 1..4 (MM:SS or 12:34) */
   switch (dig) {
      case 1:
         if (cdown.mins / 10) 
            return timer_digits[line-1][cdown.mins / 10];
         else return empty;
         break;
      case 2:
         return timer_digits[line-1][cdown.mins % 10];
      case 3:
         return timer_digits[line-1][cdown.secs / 10];
      case 4:
         return timer_digits[line-1][cdown.secs % 10];
      default:
         return empty;
   }
}


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

   wrefresh(timer);
   refresh();
}

