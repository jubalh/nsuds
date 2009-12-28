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
#ifndef _NSUDS_MENU_H
#define _NSUDS_MENU_H
#include <sys/queue.h>

enum {MENU_RFRESH};

struct item {
   char *name;    /* Name of item */
   TAILQ_ENTRY(item) entries;
};

typedef struct {
   WINDOW *window;	   /* Ncurses Window */
   char *title;	      /* Window Title */
   int height;		      /* Height of Window */
   int width;		      /* Width of the window? */
   bool rfresh;         /* Refresh after a write */
   int offset;	         /* Item offset (Which is shown first) */
   int size;		      /* Number of items */
   int selected;        /* Which item is selected */
   struct item *cur;    /* Pointer to [offset]'th item  */
   TAILQ_HEAD(item_hn, item) items; /* List of items */
} Menu; 

/* Prototypes */
extern Menu *menu_new(int height, int width, int starty, int startx, 
       char *title);
extern void menu_set(Menu *m, int flag, int val);
extern void menu_scroll(Menu *m, int dir);
extern void menu_add_item(Menu *m, char *name);
extern void free_menu(Menu *m);
extern int launch_menu(int height, int width, int starty, int startx,
       char *title, char *items[], int select);

#endif

