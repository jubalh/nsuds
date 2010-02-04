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

#include <stdio.h>
#include <stdlib.h>
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
#include <sys/stat.h>

#include "highscores.h"
#include "nsuds.h"
#include "util.h"
#include "scroller.h"

void display_scores(void)
{
   FILE *fd;
   Scroller *s;

   s = scroller_new(row * 0.9, col * 0.9, row * 0.05, col * 0.05, 
     "High Scores");
   /* Don't draw until we're done adding lines */
   scroller_set(s, SCRL_RFRESH, 0);
   
   /* Read in the file */
   fd=fopen(SCOREDIR "high_scores", "r");
   if (fd==NULL) {
      scroller_write(s, "Error: Can't access high score file!");
      scroller_write(s, "Are you sure you installed nsuds correctly?");
   } else {
      struct stat f;
      if (stat(SCOREDIR "high_scores", &f) == -1) 
         scroller_write(s, "Error: Can't stat high score file");
      else if (f.st_size == 0) 
         scroller_write(s, "Error: High score file empty!");
      else {
         char *buffer, *record;
         int position=1;
         buffer=tmalloc(128);
         record=tmalloc(128);

         scroller_write(s, "{Top 15 Sudoku High Scores}");
         scroller_write(s, " ");
         scroller_write(s, "Here are the best of the skilled degenerates "
            "on your system:");
         scroller_write(s, " ");
         scroller_write(s, "%Press Enter to return to the game%");
         scroller_write(s, " ");
         scroller_write(s, "    _Date_       |  _Score_  |  _Diff_  |"
            " _Lv_ | _Time_   | _Name_");

         /* Get each line */
         while (fgets(buffer, 128, fd)) {
            struct highscore *h;
            char *m = (char *)memchr(buffer, '\n', 128);
            *m='\0';
            /* Comment */
            if (*buffer == '#') continue;
            h = tmalloc(sizeof(struct highscore));
            h->difficulty = tmalloc(12);
            h->version = tmalloc(8);
            h->name = tmalloc(15);
            /* Parse in the line */
            sscanf(buffer, "%d/%d/%d|%[^|]|%d|%[^|]|%d|%d:%d|%[^|]%*c",
               &h->year, &h->month, &h->day, h->version, &h->score,
               h->difficulty, &h->level, &h->hours, &h->mins, h->name);
            /* Display it */
            sprintf(record, "%2d) %4d/%.2d/%.2d | %7d | %6.6s | %2d "
               "| %1dh %2dm | %.10s",
               position, h->year, h->month, h->day, h->score, 
               h->difficulty,h->level, h->hours, h->mins, h->name);
            scroller_write(s, record);
            position++;
            free(h->difficulty);
            free(h->version);
            free(h->name);
            free(h);
         }
         scroller_write(s, " ");
         scroller_write(s, "%Press Enter to return to the game%");
         scroller_write(s, " ");
         scroller_write(s, " * - Denotes a default computer score ");
         scroller_write(s, " ** - The unshown 0th score is held by CowboyNeal");
         free(buffer);
         free(record);
      }
      fclose(fd);
   }

   /* Allow draws again (and draw) */
   scroller_set(s, SCRL_RFRESH, 1);

   /* Place over everything */
   overwrite(s->window, grid);

   /* Handle user input */
   scroller_input_loop(s);

   /* User has closed the scroller, free it */
   free_scroller(s);
}

