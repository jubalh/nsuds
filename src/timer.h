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

#ifndef _NSUDS_TIMER_H
#define _NSUDS_TIMER_H

/* Level timer */
struct ltimer {
   short mins;
   short secs;
};

/* Game timer */
struct gtimer {
   short mins;
   short hours;
};

extern void draw_timer();
extern void catch_alarm(int sig);
extern void start_timer(int mins, int secs);
extern struct ltimer cdown, ltime;
extern struct gtimer gtime;

#endif

