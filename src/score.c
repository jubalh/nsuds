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
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#if STDC_HEADERS || HAVE_STRING_H
   #include <string.h>
#else /* Old system with only <strings.h> */
   #include <strings.h>
#endif
#include <ncurses.h>

#include "score.h"
#include "nsuds.h"
#include "timer.h"
#include "util.h"
#include "grid.h"
#include "scroller.h"

int score=0;
int level=1;

static char *rasprintf(char *str, char *format, ...);

/* Allocate string and parse with vsprintf, then return the allocated string,
 * for use with scroller_write. Note that if the formatting characters
 * expand to 20 or more characters, it'll overflow! */
static char *rasprintf(char *str, char *format, ...) {
   va_list ap;
   if (str) free(str);
   str = tmalloc(strlen(format) + 20);

   va_start(ap, format);
   vsprintf(str, format, ap);
   va_end(ap);

   return str;
}

/* Display the level win screen. The game may end, or they
 * may be taken to a new level. */
void game_win(void)
{
   Scroller *s;
   char *line=NULL;
   paused = 1;
   curs_set(0);

   s = scroller_new(row * 0.9, col * 0.9, row * 0.05, col * 0.05, 
     "Congratulations!");
   score += (cdown.mins * 60 + cdown.secs) * (1+difficulty/100);

   /* Don't draw until we're done adding lines */
   scroller_set(s, SCRL_RFRESH, 0);

   /* Add lines */
   scroller_write(s, "{Congratulations! You have won the level}");
   scroller_write(s, " ");

   scroller_write(s, rasprintf(line, "Time for Level:  {_%d_} minutes and "
      "{_%d_} seconds", (20 * 60 - (cdown.mins * 60 + cdown.secs)) / 60,
      (20 * 60 - (cdown.mins * 60 + cdown.secs)) % 60));
   scroller_write(s, rasprintf(line, "Total Game time: {_%d_} minutes and "
      "{_%d_} seconds", gtime.mins, gtime.secs));
   scroller_write(s, rasprintf(line, "Score so far:    {%d}", score));
   scroller_write(s, " ");
   scroller_write(s, " ");
   scroller_write(s, "%Press Enter to continue to the next level%");
   scroller_write(s, " ");
   scroller_write(s, "_Score Breakdown_:");
   scroller_write(s, "Here is the breakdown of your current score.");
   scroller_write(s, " ");
   scroller_write(s, "         Time              Score   Total");
#if 0
   scroller_write(s, rasprintf(line, "Level 1: %2d mins %2d secs "
      "|   -   |  %d", cdown.mins, cdown.secs, score));
   scroller_write(s, rasprintf(line, "Level 2: %2d mins %2d secs "
      "|   -   |  %d", cdown.mins, cdown.secs, score));
#endif
   scroller_write(s, rasprintf(line, "Total:   %2d mins %2d secs "
      "|   -   |  %d", gtime.mins, gtime.secs, score));
   scroller_write(s, " ");
   scroller_write(s, "%Press Enter to continue to the next level%");
   free(line);


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
   char *line=NULL;
   paused=1;
   curs_set(0);

   s = scroller_new(row * 0.9, col * 0.9, row * 0.05, col * 0.05, "Game Over");

   /* Don't draw until we're done adding lines */
   scroller_set(s, SCRL_RFRESH, 0);

   /* Add lines */
   if (level >= 30) {
      scroller_write(s, "%Game completed! You have beaten nsuds%");
   } else {
      scroller_write(s, "%Game Over! You failed to complete the "
         "puzzle in time!%");
   }
   scroller_write(s, " ");
   if (level < 3) {
      scroller_write(s, "Keep practicing, to improve your skills. If you're "
         "stuck, you can get extensive help on how to play by pressing the "
         "{?} key while in a game.");
   } else if (level < 8) {
      scroller_write(s, "Your doing pretty good, but keep practicing, to "
         "improve your skills. If you're stuck, you can get extensive help "
         "by pressing the {?} key while in a game.");
   } else {
      scroller_write(s, "You played a good game. But keep practicing, as the "
         "faster you play, the higher you score!");
   }
   scroller_write(s, " ");
   scroller_write(s, "_Statistics_");
   scroller_write(s, rasprintf(line, "Total score:   %d", score));
   scroller_write(s, rasprintf(line, "Level reached: %d", level));
   scroller_write(s, rasprintf(line, "High score:    %s", "?"));
   scroller_write(s, " ");
   scroller_write(s, " ");
   scroller_write(s, "{Press Enter to start a new game}");
   scroller_write(s, " ");
   scroller_write(s, "_Score Breakdown_:");
   scroller_write(s, "Here is the breakdown of your final score.");
   scroller_write(s, " ");
   scroller_write(s, "         Time              Score   Total");
#if 0
   scroller_write(s, rasprintf(line, "Level 1: %2d mins %2d secs "
      "|   -   |  %d", cdown.mins, cdown.secs, score));
   scroller_write(s, rasprintf(line, "Level 2: %2d mins %2d secs "
      "|   -   |  %d", cdown.mins, cdown.secs, score));
#endif
   scroller_write(s, rasprintf(line, "Total:   %2d mins %2d secs "
      "|   -   |  %d", cdown.mins, cdown.secs, score));
   scroller_write(s, " ");
   scroller_write(s, "{Press Enter to start a new game}");
   free(line);

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

