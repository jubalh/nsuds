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
#include <unistd.h>
#include <signal.h>
#include <ncurses.h>
#include "timer.h"

extern WINDOW *timer;

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
} countdown;

void start_timer(int mins, int secs)
{
   countdown.mins = mins;
   countdown.secs = secs;
   signal(SIGALRM, catch_alarm);
   draw_timer();
   alarm(1);
}

void catch_alarm(int sig)
{
   if (!countdown.secs && !countdown.mins) return;

   if (countdown.secs > 0) {
      countdown.secs--;
   } else {
      countdown.mins--;
      countdown.secs=59;
   }

   alarm(1);
   draw_timer();
}

#define time2strs(line) digits2str(line, 1), digits2str(line, 2), \
   digits2str(line, 3), digits2str(line, 4)

static char *digits2str(int line, int dig) {
   /* dig = 1..4 (MM:SS or 12:34) */
   if (dig==1) {
      if (countdown.mins / 10) 
         return timer_digits[line-1][countdown.mins / 10];
      else return empty;
   } else if (dig==2) {
      return timer_digits[line-1][countdown.mins % 10];
   } else if (dig==3) {
      return timer_digits[line-1][countdown.secs / 10];
   } else if (dig==4) {
      return timer_digits[line-1][countdown.secs % 10];
   } else {
      return empty;
   }
}


void draw_timer()
{
   int left;
   werase(timer);

   if (countdown.mins > 9) left = 1;
   else left = 1;
   box(timer, 0, 0);
   mvwaddstr(timer, 1, 5, "Time Remaining");
   mvwprintw(timer, 2, 1, "%s%s   %s%s", time2strs(1));
   mvwprintw(timer, 3, 1, "%s%s . %s%s", time2strs(2));
   mvwprintw(timer, 4, 1, "%s%s . %s%s", time2strs(3));

   wrefresh(timer);
   doupdate();
   refresh();
}



