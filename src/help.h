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
#ifndef _NSUDS_HELP_H
#define _NSUDS_HELP_H
#include <sys/queue.h>

enum {SCROLL_UP, SCROLL_DOWN, SCROLL_TOP, SCROLL_BASE};
enum {SCRL_RFRESH};

struct scrl_line {
   char *line;
   attr_t *fmask; /* Format mask */
   int lines;
   TAILQ_ENTRY(scrl_line) entries;
};

/* Scrolling window */
typedef struct {
   WINDOW *window;            /* Ncurses Window */
   char *title;               /* Optional title */
   int rfresh;                /* Refresh or not */
   int height;	               /* Height of window */
   int width;	               /* Width of window */
   int size;		            /* # of lines in buffer (pos) */
   int overview;              /* Overflow offset for this->cur */
   struct scrl_line *cur; /* First line shown in the scroller */
   TAILQ_HEAD(scrl_hn, scrl_line) buffer; /* List of lines */
} Scroller;

#define scroller_check_over(s) (void)scroller_can_down(s)
extern  void launch_file(char *fname, char *title);

#endif

