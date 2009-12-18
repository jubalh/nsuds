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
#include <stdarg.h>
#if STDC_HEADERS || HAVE_STRING_H
   #include <string.h>
#else /* Old system with only <strings.h> */
   #include <strings.h>
#endif
#ifdef HAVE_NCURSES_H
   #include <ncurses.h>
#else 
   #include <curses.h>
#endif

#include "score.h"
#include "nsuds.h"
#include "timer.h"
#include "util.h"
#include "grid.h"
#include "scroller.h"

int score=0;
int level=1;
static bool initialized=0;
static char *rasline;   /* For rasprintf */

static char *rasprintf(char *format, ...);
static void free_ras(void);

/* Allocate string and parse with vsprintf, then return the allocated string,
 * for use with scroller_write. Note that if the formatting characters
 * expand to 20 or more characters, it'll overflow! */
static char *rasprintf(char *format, ...)
{
   va_list ap;
   if (rasline) free(rasline);
   rasline = tmalloc(strlen(format) + 20);

   va_start(ap, format);
   vsprintf(rasline, format, ap);
   va_end(ap);

   return rasline;
}

/* Free data used by rasprintf, when it's
 * no longer going to be accessed. */
static void free_ras(void)
{
   if (rasline) {
      free(rasline);
      rasline=NULL;
   }
}

/* Display the level win screen. The game may end, or they
 * may be taken to a new level. */
void game_win(void)
{
   Scroller *s;
   struct level *curlev, *i;  /* Current level data (and temp) */
   int cscore=0;              /* Cumulative score */

   /* Make sure the tail queue is initialized */
   if (!initialized) {
      TAILQ_INIT(&level_data);
      initialized=1;
   }
   /* Pause */
   paused = 1;
   curs_set(0);

   s = scroller_new(row * 0.9, col * 0.9, row * 0.05, col * 0.05, 
     "Congratulations!");

   /* Record level data */
   curlev = tmalloc(sizeof(struct level));
   curlev->level = level;
   curlev->score = (cdown.mins * 60 + cdown.secs) * (1+difficulty/100);
   curlev->time.mins = (20*60 - (cdown.mins * 60 + cdown.secs)) / 60;
   curlev->time.secs = (20*60 - (cdown.mins * 60 + cdown.secs)) % 60;
   TAILQ_INSERT_TAIL(&level_data, curlev, entries);
   score += curlev->score;

   /* It's game over if we reach the last level */
   if (level==30) {
      game_over();
      return;
   }

   /* Don't draw until we're done adding lines */
   scroller_set(s, SCRL_RFRESH, 0);

   /* Add text of level win screen */
   scroller_write(s, "{Congratulations! You have won the level}");
   scroller_write(s, " ");
   scroller_write(s, rasprintf("Time for Level:  {_%d_} minutes and "
      "{_%d_} seconds", curlev->time.mins, curlev->time.secs));
   scroller_write(s, rasprintf("Total Game time: {_%d_} minutes and "
      "{_%d_} seconds", gtime.mins, gtime.secs));
   scroller_write(s, rasprintf("Score so far:    {%d}", score));
   scroller_write(s, " ");
   scroller_write(s, " ");
   scroller_write(s, "%Press Enter to continue to the next level%");
   scroller_write(s, " ");
   scroller_write(s, "_Score Breakdown_:");
   scroller_write(s, "Here is the breakdown of your current score.");
   scroller_write(s, " ");
   scroller_write(s, "           _Time_             _Score_  _Total_");
   TAILQ_FOREACH(i, &level_data, entries) {
      cscore += i->score;
      scroller_write(s, rasprintf("Level %2d: %2d mins %2d secs "
         "| %5d | %d", i->level, i->time.mins, i->time.secs,
         i->score, cscore));
   }
   scroller_write(s, "----------------------------------------");
   scroller_write(s, rasprintf("Total:    %2d hrs  %2d mins "
      "|   -   | %d", gtime.hours, gtime.mins, score));
   scroller_write(s, " ");
   scroller_write(s, "%Press Enter to continue to the next level%");
   free_ras(); /* Free memory from rasprintf */

   /* Allow draws again (and draw) */
   scroller_set(s, SCRL_RFRESH, 1);

   /* Place over everything */
   overwrite(s->window, grid);

   /* Handle user input */
   scroller_input_loop(s);

   /* User has closed the scroller, free it */
   free_scroller(s);

   level++;
   new_level();
}

/* Draw game over screen */
void game_over(void)
{
   Scroller *s;
   struct level *i;
   int cscore=0;     /* Cumulative score */

   /* Pause */
   paused=1;
   curs_set(0);

   s = scroller_new(row * 0.9, col * 0.9, row * 0.05, col * 0.05, "Game Over");

   /* Don't draw until we're done adding lines */
   scroller_set(s, SCRL_RFRESH, 0);

   /* Add text of game over screen */
   if (level >= 30) {
      scroller_write(s, "%Game completed! You have beaten nsuds!%");
   } else {
      scroller_write(s, "%Game Over! You failed to complete the "
         "puzzle in time!%");
   }
   scroller_write(s, " ");
   if (level <= 3) {
      scroller_write(s, "Keep practicing, to improve your skills. If you're "
         "stuck, you can get extensive help on how to play by pressing the "
         "{?} key while in a game.");
   } else if (level < 8) {
      scroller_write(s, "Your doing pretty good, but keep practicing, to "
         "improve your skills. If you're stuck, you can get extensive help "
         "by pressing the {?} key while in a game.");
   } else if (level>=30) {
      scroller_write(s, "You have beaten all 30 levels! Very impressive!");
   } else {
      scroller_write(s, "You played a good game. But keep practicing, as the "
         "faster you play, the higher you score!");
   }
   scroller_write(s, " ");
   scroller_write(s, "_Statistics_");
   scroller_write(s, rasprintf("Total score:   %d", score));
   scroller_write(s, rasprintf("Level reached: %d", level));
   scroller_write(s, rasprintf("High score:    %s", "?"));
   scroller_write(s, " ");
   scroller_write(s, " ");
   scroller_write(s, "{Press Enter to start a new game}");
   scroller_write(s, " ");
   scroller_write(s, "_Score Breakdown_:");
   scroller_write(s, "Here is the breakdown of your current score.");
   scroller_write(s, " ");
   scroller_write(s, "           _Time_             _Score_  _Total_");
   TAILQ_FOREACH(i, &level_data, entries) {
      cscore += i->score;
      scroller_write(s, rasprintf("Level %2d: %2d mins %2d secs "
         "| %5d | %d", i->level, i->time.mins, i->time.secs,
         i->score, cscore));
   }
   scroller_write(s, "----------------------------------------");
   scroller_write(s, rasprintf("Total:    %2d hrs  %2d mins "
      "|   -   | %d", gtime.hours, gtime.mins, score));
   scroller_write(s, " ");
   scroller_write(s, "{Press Enter to start a new game}");
   free_ras(); /* Free memory from rasprintf */

   /* Free level data */
   while ((i=TAILQ_FIRST(&level_data))) {
      TAILQ_REMOVE(&level_data, i, entries);
      free(i);
   }

   /* TODO: Show high scores after */

   /* Allow draws again (and draw) */
   scroller_set(s, SCRL_RFRESH, 1);

   /* Place over everything */
   overwrite(s->window, grid);

   /* Handle user input */
   scroller_input_loop(s);

   /* User has closed the scroller, free it */
   free_scroller(s);

   /* Reset score and timers */
   score = 0;
   level=1;
   gtime.hours = gtime.mins = 0;

   new_level();
}

