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
#if STDC_HEADERS || HAVE_STRING_H
   #include <string.h>
#else /* Old system with only <strings.h> */
   #include <strings.h>
#endif
#include <stdlib.h>
#ifdef HAVE_NCURSES_H
   #include <ncurses.h>
#else 
   #include <curses.h>
#endif
#include <math.h>
#include <sys/stat.h>
#include <sys/queue.h>
#include <errno.h>

#include "nsuds.h"
#include "util.h"
#include "help.h"

/* Headers */
static void draw_scroller(Scroller *s);
static Scroller *scroller_new(int height, int width, int starty, 
                int startx, char *title);
static void scroller_resize(Scroller *s, int height, int width, 
            int starty, int startx);
static void scroller_write(Scroller *s, char *msg);
static void scroller_scroll(Scroller *s, int dir);
static bool scroller_can_down(Scroller *s);
static void free_scroller(Scroller *s);
static void scroller_set(Scroller *s, int flag, int val);

/* Return a pointer to a new scroller, or scrollable text window */
static Scroller *scroller_new(int height, int width, int starty, 
                int startx, char *title)
{
   Scroller *new = tmalloc(sizeof(Scroller));
   new->window = newwin(height, width, starty, startx);
   new->height = height;
   new->width = width;
   new->rfresh=1;
   new->size = 0;
   new->overview=0;
   new->cur=NULL;
   if (title) {
      new->title = tmalloc(strlen(title) + 1);
      strcpy(new->title, title);
   } else {
      new->title = NULL;
   }
   TAILQ_INIT(&new->buffer);

   return new;
}

/* Redraw the window, after a scroll for instance */
static void draw_scroller(Scroller *s)
{
   int i, j; 
   struct scrl_line *l;
   int dlines=0;                     /* Number of lines displayed */
   int in_ul=0, in_cyan=0, in_red=0; /* States for character attributes */

   /* Erase and reborder */
   werase(s->window);
   box(s->window, 0, 0);

   /* Print title if applicable */
   if (s->title) {
      mvwaddstr(s->window, 0, s->width/2 - strlen(s->title)/2, s->title);
   }

   /* Loop through real lines in the buffer */
   for (l=s->cur; l; l=TAILQ_NEXT(l, entries)) {
      /* Loop through segments that can fit within the width of the scroller */
      for (i=0; i < l->lines; i++) {
         int start = i * (s->width-2); /* Start of segment */
         if (!dlines && i < s->overview) continue;  /* Skip overflowed lines */
         
         wmove(s->window, ++dlines, 1);
         /* Print each character, parsing style info */
         for (j=start; j < strlen(l->line)
           && j < s->width-2 + start; j++) {
            switch (l->line[j]) {
               /* _Underlined text_  normal */
               case '_':
                  in_ul=!in_ul;
                  break;
               /* { Cyan text } normal */
               case '{':
                  in_cyan=1;
                  break;
               case '}':
                  in_cyan=0;
                  break;
               /* %Red text% normal */
               case '%':
                  in_red = !in_red;
                  break;
               default:
                  if (in_ul) {
                     waddch(s->window, l->line[j] | A_UNDERLINE);
                  } else if (in_cyan) {
                     waddch(s->window, l->line[j] | COLOR_PAIR(1));
                  } else if (in_red) {
                     waddch(s->window, l->line[j] | COLOR_PAIR(8) | A_BOLD);
                  } else {
                     waddch(s->window, l->line[j]);
                  }
            }
         }
         if (dlines >= s->height-2) goto done;
      }
   }
done:

   /* Print scroll indicators */
   if (scroller_can_down(s)) mvwaddch(s->window, s->height-2, s->width-1, 'v'); 
   if (s->overview>0 || (s->cur && TAILQ_PREV(s->cur, scrl_hn, entries)))
      mvwaddch(s->window, 1, s->width-1, '^');

   wrefresh(s->window);
}

/* Is there enough lines to scroll down? */
static bool scroller_can_down(Scroller *s)
{
   struct scrl_line *a;
   int total= - s->overview-1;
   for (a=s->cur; a; a=TAILQ_NEXT(a, entries)) {
      total += a->lines;
      if (total >= s->height-2) return true;
   }
   /* Fix overflow incase invoked as macro scroller_check_over() */
   if (total < s->height-1) scroller_scroll(s, SCROLL_BASE);
   return false;
}


/* Resize and reposition a scroller window */
static void scroller_resize(Scroller *s, int height, int width, 
            int starty, int startx)
{
   struct scrl_line *l;
   /* Recalculate the number of overflowing lines */
   for (l=TAILQ_FIRST(&s->buffer); l; l=TAILQ_NEXT(l, entries)) {
      l->lines = ceil(strlen(l->line) / (double)(width - 2));
   }

   /* Create a new window */
   delwin(s->window);
   s->window = newwin(height, width, starty, startx);
   /* Update the size */
   s->height = height;
   s->width = width;
   /* If the windows enlarged, make sure as much as
    * possible is shown */
   scroller_check_over(s);
}

/* Add a line to a Scroller, and update the window accordingly. */
static void scroller_write(Scroller *s, char *msg)
{
   struct scrl_line *nline;
   
   /* FIXME: Perhaps store strlen(line) in scrl_line, which
    * would save a lot of wasted cpu time. 
    */

   /* Remove first item if buffer is full */
   if (s->size >= BUF_SIZE) {
      if (s->cur==TAILQ_FIRST(&s->buffer)) s->cur = TAILQ_NEXT(s->cur,entries);
      TAILQ_REMOVE(&s->buffer, TAILQ_FIRST(&s->buffer), entries); 
   } else s->size++;  /* If nothing is removed, the #lines is increased */

   nline = tmalloc(sizeof(*nline));
   nline->line = tmalloc(strlen(msg)+1);
   strcpy(nline->line, msg);
   nline->lines = ceil(strlen(msg) / (double)(s->width - 2));
   TAILQ_INSERT_TAIL(&s->buffer, nline, entries);

   if (!s->cur) s->cur=TAILQ_FIRST(&s->buffer);

   /* Scroll to bottom and refresh */
   if (s->rfresh) {
      scroller_scroll(s, SCROLL_BASE);
      draw_scroller(s);
   }
}


/* Scroll a scroller up/down or to the bottom/top */
static void scroller_scroll(Scroller *s, int dir)
{
   /* Buffer is empty */
   if (!s->cur) {
      s->cur = TAILQ_FIRST(&s->buffer);
      return;
   }

   switch (dir) {
      case SCROLL_DOWN:
         if (!scroller_can_down(s)) break;
         if (s->cur->lines > s->overview+1) {
            s->overview++;
         } else {
            s->cur = TAILQ_NEXT(s->cur, entries);
            s->overview=0;
         }
         break;
      case SCROLL_UP:
         if (s->overview>0) s->overview--;
         else if (TAILQ_PREV(s->cur, scrl_hn, entries)) {
            s->cur = TAILQ_PREV(s->cur, scrl_hn, entries);
            if (s->cur->lines>1) s->overview = s->cur->lines -1;
         }
         break;
      /* Scroll to the top line */
      case SCROLL_TOP:
         s->cur = TAILQ_FIRST(&s->buffer);
         s->overview=0;
         break;
      /* Scroll to the bottom line */
      case SCROLL_BASE:
         {
            int total=s->height-2;
            s->cur = TAILQ_LAST(&s->buffer, scrl_hn);
            do {
               total-=s->cur->lines;
               if (total <= 0) {
                  s->overview=-total;
                  return;
               }
               if (!TAILQ_PREV(s->cur, scrl_hn, entries)) break;
            } while ((s->cur=TAILQ_PREV(s->cur, scrl_hn, entries)));

            /* Not enough lines to fill the screen */
            s->cur = TAILQ_FIRST(&s->buffer);
            s->overview=0;
         }
   }
   draw_scroller(s);
}


/* Free memory from a scroller */
static void free_scroller(Scroller *s)
{
   while ((s->cur = TAILQ_FIRST(&s->buffer))) {
      free(s->cur->line);
      TAILQ_REMOVE(&s->buffer, s->cur, entries);
      free(s->cur);
   }
   if (s->title) free(s->title);
   delwin(s->window);
   free(s);
}


/* Load a text file into a scroller dialog */
void launch_file(char *fname, char *title)
{
   FILE *fd;
   int c, i;
   Scroller *s;

   s = scroller_new(row * 0.9, col * 0.9, row * 0.05, col * 0.05, title);

   /* Don't draw until we're done adding lines */
   scroller_set(s, SCRL_RFRESH, 0);
   
   /* Read in the file */
   fd=fopen(fname, "r");
   if (fd != NULL) {
      struct stat f;
      if (stat(fname, &f) == -1) 
         scroller_write(s, "Error: Can't stat helpfile");
      else if (f.st_size == 0) 
         scroller_write(s, "Error: Help file empty");
      else {
         char *buffer;
         buffer=tmalloc(4096);

         /* Get each line, max 4096 chars */
         while (fgets(buffer, 4096, fd)) {
            char *m = (char *)memchr(buffer, '\n', 4096);
            *m=' ';
            scroller_write(s, buffer);
         }
         free(buffer);
      }
      fclose(fd);
   } else {
      /* Perhaps they're not installed properly */
      scroller_write(s, "Error: Can't access help files");
      scroller_write(s, strerror(errno));
   }

   /* Allow draws again */
   scroller_set(s, SCRL_RFRESH, 1);
   scroller_scroll(s, SCROLL_TOP);
   draw_scroller(s);


   /* Place over everything */
   overwrite(s->window, grid);
   
   /* Handle input (10=Enter) */
   while ((c=getch()) != 10) {
      switch (c) {
         case KEY_RESIZE:
            getmaxyx(stdscr, row, col);
            scroller_resize(s, row * 0.9, col * 0.9, row * 0.05, col * 0.05);
            draw_all();
            draw_scroller(s);
            break;
         case KEY_UP:
         case 'K':
         case 'k':
         case 'W':
         case 'w':
            scroller_scroll(s, SCROLL_UP);
            break;
         case KEY_DOWN:
         case 'J':
         case 'j':
         case 'S':
         case 's':
         case 32: /* 32 = space */
            scroller_scroll(s, SCROLL_DOWN);
            break;
         case KEY_PPAGE:
            for (i=0;i<6;i++) scroller_scroll(s, SCROLL_UP);
            break;
         case KEY_NPAGE:
            for (i=0;i<6;i++) scroller_scroll(s, SCROLL_DOWN);
            break;
         case KEY_HOME:
            scroller_scroll(s, SCROLL_TOP);
            draw_scroller(s);
            break;
         case KEY_END:
            scroller_scroll(s, SCROLL_BASE);
            draw_scroller(s);
            break;
         case 'Q':
         case 'q':
         case 27: /* Escape */
            goto done;
      }
   }
done:
   free_scroller(s);
}

/* Set properties of a scroller */
static void scroller_set(Scroller *s, int flag, int val)
{
   switch (flag) {
      case SCRL_RFRESH:
         s->rfresh=val;
         break;
      default:
         break;
   }
}


