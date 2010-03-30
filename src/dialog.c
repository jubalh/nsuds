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
#include <unistd.h>
#include <string.h>
#ifdef HAVE_NCURSES_H
   #include <ncurses.h>
#else 
   #include <curses.h>
#endif

#include "dialog.h"
#include "nsuds.h"
#include "timer.h"
#include "util.h"

/* Launch a dialog that asks OK/Cancel for a question,
 * pausing the game while it waits for input */
bool confirm(char *question)
{
   int c;
   bool status=false;
   WINDOW *confirm;

   /* Cancel alarm */
   alarm(0);
   /* Pause */
   paused=1;
   curs_set(!paused);
   /* Only redraw the grid if the help isn't open */
   if (!scrl_open) draw_grid();

redraw:
   confirm = newwin(row * 0.4, col * 0.7, row * 0.3, col * 0.15);

   /* Draw dialog */
   wbkgd(confirm, COLOR_PAIR(C_DIALOG));
   box(confirm, 0, 0);
   mvwaddstr(confirm, 0, col * 0.35 - (strlen("Confirm..") / 2), "Confirm");
   mvwaddstr(confirm, 2, col * 0.35 - (strlen(question) / 2), question);

   /* Draw the options */
   wattrset(confirm, A_REVERSE);	
   if (status) {
      mvwaddstr(confirm, (row * 0.4) -3, col *0.35 - 9, "   OK   ");
      wattroff(confirm, A_REVERSE);
      mvwaddstr(confirm, (row * 0.4) -3, col *0.35, " Cancel ");
   } else {
      mvwaddstr(confirm, (row * 0.4) -3, col *0.35, " Cancel ");
      wattroff(confirm, A_REVERSE);
      mvwaddstr(confirm, (row * 0.4) -3, col *0.35 - 9, "   OK   ");
   }

   /* Draw over top of everything */
   overwrite(confirm, stats);
   wrefresh(confirm);

   /* Handle input */
   while ((c = getkey())) {
      switch (c) {
         case KEY_RESIZE:
            getmaxyx(stdscr, row, col);
            draw_all();
            goto redraw;
         case KEY_LEFT:
         case KEY_RIGHT:
         case 'h':
         case 'l':
         case 'a':
         case 'd':
         case CTRL('f'):
         case CTRL('b'):
            wattrset(confirm, A_REVERSE);	
            if (!status) {
               mvwaddstr(confirm, (row * 0.4) -3, col *0.35 - 9, "   OK   ");
               wattroff(confirm, A_REVERSE);
               mvwaddstr(confirm, (row * 0.4) -3, col *0.35, " Cancel ");
            } else {
               mvwaddstr(confirm, (row * 0.4) -3, col *0.35, " Cancel ");
               wattroff(confirm, A_REVERSE);
               mvwaddstr(confirm, (row * 0.4) -3, col *0.35 - 9, "   OK   ");
            }
            status = !status;
            wrefresh(confirm);
            break;				
         /* Enter pressed */
         case 10:
            werase(confirm);
            delwin(confirm);
            if (!scrl_open) {
               paused=0;
               curs_set(!paused);
               draw_all();
            }
            catch_alarm(0);
            fbar_time=0;
            return status;
         default:
            break;
      }
   }
   return 0;
}

/* Launch a dialog that asks user for a string (e.g name) */
char *getstring(char *question)
{
   int c, width, apos=0;
   WINDOW *dialog;
   char *answer;

   /* Cancel alarm */
   alarm(0);
   /* Pause */
   paused=1;
   draw_grid();
   scrl_open=1;

   answer = malloc(21);

redraw:
   width = clamp(col*0.7, col*0.7, 60);
   dialog = newwin(8, width, row * 0.3, col*0.5 - width/2);

   /* Draw dialog */
   wbkgd(dialog, COLOR_PAIR(C_DIALOG));
   box(dialog, 0, 0);
   mvwaddstr(dialog, 2, width/2 - (strlen(question) / 2), question);

   /* Draw input box */
   mvwhline(dialog, 4, 3, ACS_HLINE, width-7);
   mvwhline(dialog, 6, 3, ACS_HLINE, width-7);
   mvwaddch(dialog, 4, 3, ACS_ULCORNER);
   mvwaddch(dialog, 6, 3, ACS_LLCORNER);
   mvwaddch(dialog, 5, 3, ACS_VLINE);
   mvwaddch(dialog, 5, width-4, ACS_VLINE);
   mvwaddch(dialog, 4, width-4, ACS_URCORNER);
   mvwaddch(dialog, 6, width-4, ACS_LRCORNER);

   if (apos > 0) mvwaddstr(dialog, 5, 4, answer);
   wmove(dialog, 5, 4+apos);


   /* Draw over top of everything */
   overwrite(dialog, stats);
   wrefresh(dialog);

   /* Handle input */
   while ((c = getkey())) {
      switch (c) {
         case KEY_RESIZE:
            getmaxyx(stdscr, row, col);
            draw_all();
            curs_set(1);
            goto redraw;
         /* Enter pressed */
         case 10:
            werase(dialog);
            delwin(dialog);
            paused=0;
            curs_set(!paused);
            draw_all();
            catch_alarm(0);
            fbar_time=0;
            scrl_open=0;
            return answer;
         case KEY_BACKSPACE:
         case 127: /* Katch the Konsole backspace */
         case 12:
            if (apos > 0) {
               mvwprintw(dialog, 5, 3+apos, " ");
               answer[--apos] = '\0';
               wmove(dialog, 5, 4+apos);
               wrefresh(dialog);
            }
            break;
         default:
            if ((c >= 'A' && c<= 'Z') 
                  || (c>='a' && c<= 'z')
                  || c==32 || c=='_' 
                  || (c>='1' && c<='9')) {
               if (apos>=20) break;
               answer[apos]=c;
               answer[++apos]='\0';
               mvwaddch(dialog, 5, 3+apos, c);
               wmove(dialog, 5, 4+apos);
               wrefresh(dialog);
            }
            break;
      }
   }
   scrl_open=0;
   return NULL;
}

