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
#ifndef _NSUDS_SCORE_H
#define _NSUDS_SCORE_H
#include <sys/queue.h>

/* Headers */
extern void game_win(void);
extern void game_over(void);

extern int score;
extern int level;

#if 0
/* Stats for each level */
struct level {
   short level;
   struct ltimer time;  
   short score;
   TAILQ_ENTRY(scrl_line) entries;
};

struct breakdown {
   struct level *cur;  /* First line shown in the scroller */
   TAILQ_HEAD(lhn, level) lbuffer; /* List of lines */
} scores;
#endif

#endif

