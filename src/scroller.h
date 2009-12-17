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
#ifndef _NSUDS_SCROLLER_H
#define _NSUDS_SCROLLER_H
#include <sys/queue.h>

enum {SCROLL_UP, SCROLL_DOWN, SCROLL_TOP, SCROLL_BASE};
enum {SCRL_RFRESH};

struct scrl_line {
   char *line;    /* Text of the line */
   attr_t *fmask; /* Format mask */
   int lines;     /* Number of screen lines each line will take up */
   short len;     /* Length of line */
   TAILQ_ENTRY(scrl_line) entries;
};

/* Scrolling window */
typedef struct {
   WINDOW *window;         /* Ncurses Window */
   char *title;            /* Optional title */
   bool rfresh;            /* Refresh or not */
   bool smooth;            /* Smooth scrolling */
   int height;	            /* Height of window */
   int width;	            /* Width of window */
   int size;		         /* # of lines in buffer (pos) */
   int tlines;             /* Total screen lines */
   int overview;           /* Overflow offset for this->cur */
   int cur_sl;             /* Current screen line shown */
   struct scrl_line *cur;  /* First line shown in the scroller */
   TAILQ_HEAD(scrl_hn, scrl_line) buffer; /* List of lines */
} Scroller;

extern void launch_file(char *fname, char *title);
extern Scroller *scroller_new(int height, int width, int starty, 
                int startx, char *title);
extern void scroller_set(Scroller *s, int flag, int val);
extern void scroller_write(Scroller *s, char *msg);
extern void scroller_input_loop(Scroller *s);
extern void free_scroller(Scroller *s);

#endif

