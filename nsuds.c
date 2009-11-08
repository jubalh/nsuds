#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>

static int colors=0;
static int row,col;
static WINDOW *grid;

static void init_ncurses(void)
{
   initscr(); /* Enter curses */
   if (has_colors()) {
      start_color(); 
   }

   init_pair(1, COLOR_BLUE, COLOR_BLACK);
   cbreak();      /* Disable line buffering */
   noecho();      /* Don't echo typed chars */
   curs_set(0);   /* Hide cursor */
   keypad(stdscr, TRUE); /* Catch special keys */
   getmaxyx(stdscr, row, col);
   refresh();
}

static void draw_grid(void) {
   int i, j;
   int left;

   left = 20;
   clear();
   refresh();
   grid=newwin(40, 40, 1, left);

   /* Top border */
   mvwaddch(grid, 0, 0, ACS_ULCORNER);
   mvwhline(grid, 0, 1, ACS_HLINE, 35);
   for (i=12; i<36; i+=12) 
      mvwaddch(grid, 0, i, ACS_TTEE);
   mvwaddch(grid, 0, 36, ACS_URCORNER);

   /* Horizontal insides */
   for (i=2; i<18; i+=2) {
      if (i%6==0) continue;
      mvwhline(grid, i, 1, '-', 35);
   }
   /* Vertical insides */
   for (i=4; i<36; i+=4) {
      if (i%12==0) continue;
      mvwvline(grid, 1, i, '|', 17);
   }
   /* Verticals */
   for (i=1; i<18; i+=1) {
      if (i%6==0) continue;
      for (j=0; j<=36;j+=12) {
         mvwaddch(grid, i, j, ACS_VLINE);
      }
   }

   /* Horizontal */
   for (i=6; i<18; i+=6) {
      mvwaddch(grid, i, 0, ACS_LTEE);
      mvwhline(grid, i, 1, ACS_HLINE, 36);
      for (j=12; j<=36; j+=12)
         mvwaddch(grid, i, j, ACS_PLUS);
      mvwaddch(grid, i, 36, ACS_RTEE);
   }

   /* Bottom border */
   mvwaddch(grid, 18, 0, ACS_LLCORNER);
   mvwhline(grid, 18, 1, ACS_HLINE, 35);
   for (i=12; i<36; i+=12) 
      mvwaddch(grid, 36, i, ACS_BTEE);
   mvwaddch(grid, 18, 36, ACS_LRCORNER);
      

#if 0
   /* Horizontal */
   for (i=1; i < 18; i+=2) {
      mvwaddch(grid, i, 0, ACS_LTEE);
      mvwhline(grid, i, 1, ACS_HLINE, 35);
      mvwaddch(grid, i, 36, ACS_RTEE);
   }
   /* Vertical */
   for (i=4; i < 36; i+=4) {
      mvwaddch(grid, 1, i, ACS_LTEE);
      mvwvline(grid, 1, i, ACS_VLINE, 18);
      mvwaddch(grid, 1, i, ACS_RTEE);
   }
#endif

   wrefresh(grid);
   doupdate();
   refresh();
}
   /*
   mvwaddch(grid, 0, 0, ACS_LTEE);
   whline(grid, ACS_HLINE, 35);
   waddch(grid, ACS_RTEE);
   */

int main(void) {
   char c;

   init_ncurses();
   draw_grid();
   while ((c = getch())) {
      switch (c) {
         case 'q':
            werase(grid);
            wnoutrefresh(grid);
            delwin(grid);
            endwin();
            goto done;
         default:
            break;
      }
   }
done:
   exit(0);
}


/* 
   +-----------------------------------+
   | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
   |---|---|---|---|---|---|---|---|---|
   | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
   |---|---|---|---|---|---|---|---|---|
   | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
   |---|---|---|---|---|---|---|---|---|
   | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
   |---|---|---|---|---|---|---|---|---|
   | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
   |---|---|---|---|---|---|---|---|---|
   | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
   |---|---|---|---|---|---|---|---|---|
   | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
   |---|---|---|---|---|---|---|---|---|
   | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
   |---|---|---|---|---|---|---|---|---|
   | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
   +-----------------------------------+
*/

