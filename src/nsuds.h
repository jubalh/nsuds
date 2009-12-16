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
#ifndef _NSUDS_NSUDS_H
#define _NSUDS_NSUDS_H

extern WINDOW *grid, *timer, *stats, *title, *fbar;
extern int paused, difficulty;
extern int score;
extern int fbar_time;
extern int row,col;
extern int use_colors;
extern void game_over(void);
extern void game_win(void);
extern void draw_stats(void);
extern void draw_grid(void);
extern void draw_all(void);
extern void hide_fbar(void);
extern void new_level(void);

enum {
   C_INPUT=1, C_KEY=1, C_DIALOG=2, 
   C_MARKS1=3, C_MARKS2=4, C_MARKS3=5,
   C_URGENT=6
};

#endif

