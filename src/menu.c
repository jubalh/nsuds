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
#include <sys/queue.h>

#include "scroller.h"
#include "menu.h"
#include "highscores.h"
#include "nsuds.h"
#include "util.h"

/* Structs */
struct item {
   char *name;    /* Name of item */
   TAILQ_ENTRY(item) entries;
};

typedef struct {
   WINDOW *window;	   /* Ncurses Window */
   char *title;	      /* Window Title */
   int height;		      /* Height of Window */
   int width;		      /* Width of the window? */
   int offset;	         /* Item offset (Which is shown first) */
   int size;		      /* Number of items */
   int selected;        /* Which item is selected */
   struct item *cur;    /* Pointer to [offset]'th item  */
   TAILQ_HEAD(item_hn, item) items; /* List of items */
} Menu; 

/* Prototypes */
static void menu_resize(Menu *m, int height, int width, int starty, int startx);
static void draw_menu(Menu *m);
static Menu *menu_new(int height, int width, int starty, int startx, 
            char *title);
static void menu_scroll(Menu *m, int dir);
static void menu_add_item(Menu *m, char *name);
static void free_menu(Menu *m);

/* Return a pointer to a new initialized menu */
static Menu *menu_new(int height, int width, int starty, int startx, char *title)
{
   Menu *new = tmalloc(sizeof(Menu));
   new->window = newwin(height, width, starty, startx);
   new->height = height;
   new->width = width;
   new->selected=1;
   new->offset = 0;
   new->size = 0;
   new->cur = NULL;
   if (title) {
      new->title = tmalloc(strlen(title) + 1);
      strcpy(new->title, title);
   } else {
      new->title=NULL;
   }
   TAILQ_INIT(&new->items);
   return new;
}

/* Resize a menu, e.g after a SIGWINCH */
static void menu_resize(Menu *m, int height, int width, int starty, int startx)
{
   delwin(m->window);
   m->window = newwin(height, width, starty, startx);
   m->height = height;
   m->width = width;
}

/* Redraw menu, when another item is selected for instance */
static void draw_menu(Menu *m)
{
   /* Temps */
   struct item *it;
   int i,j,y=2;

   /* Erase and border */
   werase(m->window);
   box(m->window, 0, 0);

   /* Print title if applicable */
   if (m->title) {
      mvwprintw(m->window, 0, (m->width/2) - (strlen(m->title)/2), m->title);
   }

   /* Draw items */
   for (it=m->cur, i=m->offset; it && i < m->height-3+m->offset; 
    it=TAILQ_NEXT(it, entries), i++,y++) {
       int nx; /* Number of printed chars */
       if (m->selected == i + 1) wattron(m->window, A_REVERSE);

       /* Print item, storing how many chars were printed so
        * we can pad the right with spaces below */
       mvwprintw(m->window, y, 1, "%.22s%n", it->name, &nx);

       /* If selected, use padding to highlight the rest of the line */
       if (m->selected == i + 1) {
          for (j=0; j < m->width - 2 - nx; j++) {
             waddch(m->window, ' ');
          }
          wattroff(m->window, A_REVERSE);
       }
   }

   wrefresh(m->window);
}

/* Scroll a menu, i.e select the previous or
 * next items */
static void menu_scroll(Menu *m, int dir)
{
   switch (dir) {
      case SCROLL_UP:
         if (m->selected == 1) {
            m->selected = m->size;
            menu_scroll(m, SCROLL_BASE);
            return;
         } else { 
            m->selected--;
            if (m->selected < 2+m->offset && m->offset!=0) {
               m->offset--;
               m->cur = TAILQ_PREV(m->cur, item_hn, entries);
            }
         }
         break;
      case SCROLL_DOWN:
         if (m->selected == m->size)  {
            m->selected = 1;
            m->offset=0;
            m->cur= TAILQ_FIRST(&m->items);
         } else {
            m->selected++;
            if (m->selected >= m->height-3+m->offset) {
               m->offset++;
               m->cur = TAILQ_NEXT(m->cur, entries);
            }
         }
         break;
      /* Scroll to the top */
      case SCROLL_TOP:
         m->cur = TAILQ_FIRST(&m->items);
         m->offset=0;
         m->selected=1;
         break;
      /* Scroll to the bottom line, but don't udpated
       * the selected item */
      case SCROLL_BASE:
         {
            int total=m->height-3;

            /* Not enough lines to fill the screen */
            if (m->size < m->height-3) {
               m->cur = TAILQ_FIRST(&m->items);
               m->offset=0;
               goto draw;
            }

            m->cur = TAILQ_LAST(&m->items, item_hn);
            m->offset = m->size - 1;

            while (TAILQ_PREV(m->cur, item_hn, entries)) {
               if (total <= 1) {
                  goto draw;
               }
               m->cur = TAILQ_PREV(m->cur, item_hn, entries);
               total--;
               m->offset--;
            }
         }
         break;
   }
draw:
   draw_menu(m);
}

/* Add an item to the menu. */
static void menu_add_item(Menu *m, char *name)
{
   struct item *nitem;
   nitem = tmalloc(sizeof(struct item));
   nitem->name = tmalloc(strlen(name)+1);
   nitem->name[0]='\0';
   strcpy(nitem->name, name);
   TAILQ_INSERT_TAIL(&m->items, nitem, entries);
   m->size++;
   if (!m->cur) m->cur = TAILQ_FIRST(&m->items);
}

/* Free all the items in a menu */
static void free_menu(Menu *m)
{
   while ((m->cur=TAILQ_FIRST(&m->items))) {
      free(m->cur->name);
      TAILQ_REMOVE(&m->items, m->cur, entries);
      free(m->cur);
   }
   if (m->title) free(m->title);
   delwin(m->window);
   free(m);
}

/* Launch a menu and let the user select an item.  Takes an array of item
 * names terminated with a NULL name. Returns the index in the array of the
 * selected item.  If select is nonzero, select that item by default. */
int launch_menu(int height, int width, int starty, int startx,
    char *title, char *items[], int select)
{
   Menu *m;
   int i,c;
   int ret=0;

   m = menu_new(height, width, starty, startx, title);
   if (select) m->selected = select;

   /* Add items */
   for (i=0; items[i]; i++) {
      menu_add_item(m, items[i]);
   }
   overwrite(m->window, grid);
   draw_menu(m);
   while ((c = getkey())) {
      switch (c) {
         case KEY_RESIZE:
            getmaxyx(stdscr, row, col);
            menu_resize(m, height, width, starty, startx);
            draw_all();
            draw_menu(m);
            break;
         case KEY_UP:
         case CTRL('p'):
         case ALT('p'):
         case CTRL('y'):
         case ALT('y'):
         case 'k':
         case 'w':
            menu_scroll(m, SCROLL_UP);
            break;
         case KEY_DOWN:
         case CTRL('n'):
         case ALT('n'):
         case CTRL('e'):
         case ALT('e'):
         case 'j':
         case 's':
         case 32: /* 32 = space */
            menu_scroll(m, SCROLL_DOWN);
            break;
         case KEY_HOME:
         case 'g':
            menu_scroll(m, SCROLL_TOP);
            break;
         case KEY_END:
         case 'G':
            m->selected=m->size;
            menu_scroll(m, SCROLL_BASE);
            break;
         case 10:
            ret = m->selected;
            goto done;
         case '?':
            launch_file(HELPDIR "main", "Help with nsuds");
            ungetch(KEY_RESIZE);
            break;
         case 'H':
            display_scores();
            ungetch(KEY_RESIZE);
            break;
         case 'Q':
         case 'q':
         case 27: /* Escape */
            /* Just quit at this point, don't bother asking */
            endwin();
            exit(EXIT_SUCCESS);
            break;
         default:
            unknown_key();
            break;
      }
   }
done:
   delwin(m->window);
   free_menu(m);
   hide_fbar();
   return ret;
}

