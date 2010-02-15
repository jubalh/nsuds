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
#include "scroller.h"

/* Headers */
static bool scroller_can_down(Scroller *s);
static void scroller_check_over(Scroller *s);
static void scroller_scroll(Scroller *s, int dir);
static void scroller_resize(Scroller *s, int height, int width, 
            int starty, int startx);
static void draw_scroller(Scroller *s);

/* Return a pointer to a new scroller, or scrollable text window */
Scroller *scroller_new(int height, int width, int starty, 
                int startx, char *title)
{
   Scroller *new = tmalloc(sizeof(Scroller));
   new->window = newwin(height, width, starty, startx);
   new->height = height;
   new->width = width;
   new->rfresh=1;
   new->smooth=0;
   new->size = 0;
   new->overview=0;
   new->tlines=0;
   new->cur_sl=0;
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
   int dlines=0;                 /* Number of lines displayed */

   int screen_space=s->height-2; /* Space for scrollbar to go */
   int sbar_height=0;            /* Height scrollbar should be */
   int sbar_start=0;             /* Where scrollbar should start */

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
         /* Print each character, adding formatting info */
         for (j=start; j < l->len && j < s->width-2 + start; j++) {
                     waddch(s->window, l->line[j] | l->fmask[j]);
         }
         if (dlines >= s->height-2) goto scrollbar;
      }
   }

scrollbar:

   /* Print a rather ugly scrollbar. Idealy it would take advantage of 256
    * color capabilities, but it requires ncurses to have the support
    * compiled in, and I'm not sure how widespread that is.. */
   if (s->tlines - 1 >= screen_space) {
      /* Start of scrollbar */
      sbar_start  = ((double)(s->cur_sl+1) / s->tlines) * screen_space;
      sbar_start = clamp(sbar_start, 0, screen_space-1);
      /* And it's height */
      sbar_height = (((double)screen_space / s->tlines) * screen_space);
      sbar_height = clamp(sbar_height, 1, screen_space - sbar_start + 1);

      /* If we're at the bottom, the scrollbar might not be due to a rounding
       * error. If it isn't, fix it. */
      if (!scroller_can_down(s) && sbar_start + sbar_height < screen_space)
         sbar_start++;

      for (i=0; i<sbar_height;i++) {
         mvwaddch(s->window, sbar_start + i + 1, s->width-1, 
            ACS_DIAMOND);
      }
   }

   /* And output it */
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
   return false;
}

/* Check if the scroller is scrolled too far due to a 
 * window resize. If so, fix it */
static void scroller_check_over(Scroller *s)
{
   struct scrl_line *a;
   int total= - s->overview-1;
   for (a=s->cur; a; a=TAILQ_NEXT(a, entries)) {
      total += a->lines;
      if (total >= s->height-2) return;
   }
   /* Scroll to the bottom to fix issue */
   if (total < s->height-1) scroller_scroll(s, SCROLL_BASE);
}


/* Resize and reposition a scroller window */
static void scroller_resize(Scroller *s, int height, int width, 
            int starty, int startx)
{
   struct scrl_line *l;
   s->tlines=0;
   /* Recalculate the number of overflowing lines */
   for (l=TAILQ_FIRST(&s->buffer); l; l=TAILQ_NEXT(l, entries)) {
      l->lines = ceil(l->len / (double)(width - 2));
      s->tlines += l->lines;
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
void scroller_write(Scroller *s, char *msg)
{
   struct scrl_line *nline;
   char *c;
   int len=0, i=0;
   bool in_ul=0, in_cyan=0, in_red=0; /* Formatting attributes */
   
   s->size++;
   /* Calculate the length minus formattting chars */
   for (c=msg; *c; c++) {
      switch (*c) {
         case '_':
         case '{':
         case '}':
         case '%':
            break;
         default:
            len++;
            break;
      }
   }

   /* Allocate memory */
   nline = tmalloc(sizeof(*nline));
   nline->line = tmalloc(len + 1);
   nline->fmask = tmalloc(sizeof(attr_t) * (len + 1));

   /* Parse out formatting characters, and copy over
    * all other characters with formatting applied */
   for (c=msg; *c; c++) {
         switch (*c) {
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
               nline->line[i]=*c;
               nline->fmask[i]=0;
               /* Red OR cyan */
               if (in_cyan) {
                  nline->fmask[i] |= COLOR_PAIR(C_KEY);
               } else if (in_red) {
                  nline->fmask[i] |= COLOR_PAIR(C_URGENT) | A_BOLD;
               }
               /* Can be combined with underline */
               if (in_ul) {
                  nline->fmask[i] |= A_UNDERLINE;
               } 
               i++;
               break;
         }
   }
   nline->line[i]='\0';
   nline->lines = ceil(len / (double)(s->width - 2));
   s->tlines += nline->lines;
   nline->len = len;
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
   if (!s->cur) return;

   switch (dir) {
      case SCROLL_DOWN:
         if (!scroller_can_down(s)) break;
         if (s->cur->lines > s->overview+1) {
            s->overview++;
         } else {
            s->cur = TAILQ_NEXT(s->cur, entries);
            s->overview=0;
         }
         s->cur_sl++;
         break;
      case SCROLL_UP:
         if (s->overview>0) {
            s->overview--;
            s->cur_sl--;
         } else if (TAILQ_PREV(s->cur, scrl_hn, entries)) {
            s->cur = TAILQ_PREV(s->cur, scrl_hn, entries);
            if (s->cur->lines>1) s->overview = s->cur->lines -1;
            s->cur_sl--;
         }
         break;
      /* Scroll to the top line */
      case SCROLL_TOP:
         s->cur = TAILQ_FIRST(&s->buffer);
         s->overview=0;
         s->cur_sl=0;
         break;
      /* Scroll to the bottom line */
      case SCROLL_BASE:
         {
            int total=s->height-2;
            s->cur = TAILQ_LAST(&s->buffer, scrl_hn);
            s->cur_sl = s->tlines;
            do {
               total-=s->cur->lines;
               s->cur_sl -= s->cur->lines;
               if (total <= 0) {
                  s->overview=-total;
                  return;
               }
               if (!TAILQ_PREV(s->cur, scrl_hn, entries)) break;
            } while ((s->cur=TAILQ_PREV(s->cur, scrl_hn, entries)));

            /* Not enough lines to fill the screen */
            scroller_scroll(s, SCROLL_TOP);
         }
         break;
   }
   if (s->rfresh || s->smooth) draw_scroller(s);
}


/* Free memory from a scroller */
void free_scroller(Scroller *s)
{
   while ((s->cur = TAILQ_FIRST(&s->buffer))) {
      free(s->cur->line);
      free(s->cur->fmask);
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
   Scroller *s;

   s = scroller_new(row * 0.9, col * 0.9, row * 0.05, col * 0.05, title);

   /* Don't draw until we're done adding lines */
   scroller_set(s, SCRL_RFRESH, 0);
   
   /* Read in the file */
   fd=fopen(fname, "r");
   if (fd==NULL) {
      /* Perhaps they're not installed properly */
      scroller_write(s, "Error: Can't access help files!");
      scroller_write(s, "Are you sure they are installed correctly?");
   } else {
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

/* Handle input on a scroller, so user can scroll and then close it. Handles
 * window resizes */
void scroller_input_loop(Scroller *s)
{
   int c, i;
   /* Handle input */
   while ((c=getkey())) {
      switch (c) {
         case KEY_RESIZE:
            getmaxyx(stdscr, row, col);
            scroller_resize(s, row * 0.9, col * 0.9, row * 0.05, col * 0.05);
            draw_all();
            draw_scroller(s);
            break;
         case KEY_UP:
         case CTRL('p'):
         case CTRL('y'):
         case 'k':
         case 'w':
            scroller_scroll(s, SCROLL_UP);
            break;
         case KEY_DOWN:
         case CTRL('n'):
         case CTRL('e'):
         case 'j':
         case 's':
         case 32: /* 32 = space */
            scroller_scroll(s, SCROLL_DOWN);
            break;
         case KEY_PPAGE:
         case CTRL('u'):
         case ALT('v'):
            scroller_set(s, SCRL_RFRESH, 0);
            for (i=0;i<(s->height/2);i++) scroller_scroll(s, SCROLL_UP);
            scroller_set(s, SCRL_RFRESH, 1);
            break;
         case KEY_NPAGE:
         case CTRL('d'):
         case CTRL('v'):
            scroller_set(s, SCRL_RFRESH, 0);
            for (i=0;i<(s->height/2);i++) scroller_scroll(s, SCROLL_DOWN);
            scroller_set(s, SCRL_RFRESH, 1);
            break;
         case KEY_HOME:
         case 'g':
            scroller_scroll(s, SCROLL_TOP);
            draw_scroller(s);
            break;
         case KEY_END:
         case 'G':
            scroller_scroll(s, SCROLL_BASE);
            draw_scroller(s);
            break;
         case 'Q':
         case 'q':
         case 27: /* Escape */
         case 10: /* Enter */
            hide_fbar();
            return;
         default:
            unknown_key();
            break;
      }
   }
}

/* Set properties of a scroller */
void scroller_set(Scroller *s, int flag, int val)
{
   switch (flag) {
      case SCRL_RFRESH:
         s->rfresh=val;
         if (val) draw_scroller(s);
         break;
      default:
         break;
   }
}


