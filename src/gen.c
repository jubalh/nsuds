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

/* Generate a unique-solution sudoku puzzle 
 * ---------------------------------------
 * Based on public domain code by: Guenter Stertenbrink
 * With public domain modifications by: Patrick Hulin 
 *
 * Modified for nsuds by Vincent Launchbury. All such 
 * modifications are licensed under the GNU General Public
 * License version 2, or (at your option) any later version.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/time.h>

#include "grid.h"

/* Random number in the range [a,b] */
#define rrand(a,b) (int)(((double)((double)rand() /RAND_MAX) * (b-a) ) + a)

static short Rows[325], Row[325][10], Col[730][5], Urow[730], Ucol[325], V[325], W[325];
static short C[82], I[82];
static short grid[82];   /* The puzzle grid itself */
static int w, m0, c1, c2, r1, m1;
static int solutions, min, clues;

extern char grid_data[9][9];
static int solve();

/* Generate a puzzle, put the result in grid_data */
void do_generate(void)
{
   int i,j,x,y,s; /* Temps */
   struct timeval tm;
   short rorder[82]; /* The numbers 1-81 in random order */

   /* Seed rand() */
   gettimeofday(&tm, NULL);
   srand(tm.tv_usec);

   i = 1;
   for (x = 1; x <= 9; x++) {
      for (y = 1; y <= 9; y++) {
         for (s = 1; s <= 9; s++, i++) {
            Col[i][1] = (x - 1) * 9 + y;
            Col[i][2] = (3*((x-1)/3)+(y-1)/3)*9+s+81;
            Col[i][3] = (x - 1) * 9 + s + 81 * 2;
            Col[i][4] = (y - 1) * 9 + s + 81 * 3;
         }
      }
   }

   for (i = 1; i <= 324; i++)
      Rows[i] = 0;
   for (i = 1; i <= 729; i++) {
      for (j = 1; j <= 4; j++) {
         x = Col[i][j];
         Rows[x]++;
         Row[x][Rows[x]] = i;
      }
   }

   /* Add random clues until the puzzle has a unique solution. */
   do {
      int valid, square;
      for (i = 1; i <= 81; i++) grid[i] = 0;
      do {
         /* Choose a random unfilled square */
         do {
            square = rrand(1, 81);
         } while (grid[square]);

         /* Fill with random number */
         grid[square] = rrand(1, 9);

         valid = solve();
         /* If it makes the puzzle unsolvible, 
          * remove the invalid clue and try again */
         if (!valid) grid[square] = 0;
      } while (valid != 1);

      /* Keep adding until the solution is unique */
   } while (solve() != 1);


   /* Now we have a unique-solution sudoku, remove 
    * clues to make it minimal. First, set up a 
    * list of the numbers 1-81 in random order.
    * Otherwise, the majority of numbers will 
    * always be near the beginning. */
   for (i = 1; i <= 81; i++) {
      j = rrand(1, i);
      rorder[i] = rorder[j];
      rorder[j] = i;
   }

   /* Try, in above random order, to remove each 
    * number, so that the puzzle will become minimal.*/
   for (i = 1; i <= 81; i++) {
      int old = grid[rorder[i]];
      if (!old) continue; /* Number is already empty */

      /* Try blanking out the number */
      grid[rorder[i]] = 0;

      /* If it makes the puzzle invalid, restore */
      if (solve() != 1) grid[rorder[i]] = old;
   }

   /* Transfer grid to grid_data in grid.c */
   for (i = 0; i < 9; i++) {
      for (j = 0; j < 9; j++) {
         grid_data[i][j] = - grid[i * 9 + j + 1];
      }
   }
}


/*returns 0 (no solution), 1 (unique sol.), 2 (more than one sol.) */
static int solve()			
{
   int t1,t2,t3;
   int i,j,k;
   for (i = 0; i <= 729; i++)
      Urow[i] = 0;
   for (i = 0; i <= 324; i++)
      Ucol[i] = 0;
   clues = 0;
   for (i = 1; i <= 81; i++) {
      if (grid[i]) {
         clues++;
         t3 = (i - 1) * 9 + grid[i];
         for (j = 1; j <= 4; j++) {
            t1 = Col[t3][j];
            if (Ucol[t1])
               return 0;
            Ucol[t1]++;
            for (k = 1; k <= 9; k++) {
               Urow[Row[t1][k]]++;
            }
         }
      }
   }
   for (t2 = 1; t2 <= 324; t2++) {
      V[t2] = 0;
      for (t3 = 1; t3 <= 9; t3++)
         if (Urow[Row[t2][t3]] == 0)
            V[t2]++;
   }

   i = clues;
   m0 = 0;
   m1 = 0;
   solutions = 0;

m2:
   i++;
   I[i] = 0;
   min = 729 + 1;
   if ((i <= 81) && !m0) {
      if (m1) {
         C[i] = m1;
      } else {
         w = 0;
         for (t2 = 1; t2 <= 324; t2++) {
            if (!Ucol[t2]) {
               if (V[t2] < 2) {
                  C[i] = t2;
                  goto keepgoing;
               } else {
                  if (V[t2] <= min) {
                     w++;
                     W[w] = t2;
                  }
                  if (V[t2] < min) {
                     w = 1;
                     W[w] = t2;
                     min = V[t2];
                  }
               }
            }
         }
         c2 = rrand(1,w-1);
         C[i] = W[c2 + 1];
      }
keepgoing:
      t2 = C[i];
      I[i]++;
      if (I[i] <= 9) {

         t3 = Row[t2][I[i]];
         if (Urow[t3])
            goto keepgoing;

         m0 = 0;
         m1 = 0;


         for (j = 1; j <= 4; j++) {
            c1 = Col[t3][j];
            Ucol[c1]++;
         }
         for (j = 1; j <= 4; j++) {
            c1 = Col[t3][j];
            for (k = 1; k <= 9; k++) {
               r1 = Row[c1][k];
               Urow[r1]++;
               if (Urow[r1] == 1) {
                  for (t1 = 1; t1 <= 4; t1++) {
                     c2 = Col[r1][t1];
                     V[c2]--;
                     if ((Ucol[c2] + V[c2]) < 1)
                        m0 = c2;
                     if ((Ucol[c2] == 0) && (V[c2] < 2))
                        m1 = c2;
                  }
               }
            }
         }
         if (i == 81)
            solutions++;
         if (solutions > 1)
            return solutions;
         goto m2;
      }
   }

   i--;
   t2 = C[i];
   t3 = Row[t2][I[i]];
   if (i == clues)
      return solutions;
   for (j = 1; j <= 4; j++) {
      c1 = Col[t3][j];
      Ucol[c1]--;
      for (k = 1; k <= 9; k++) {
         r1 = Row[c1][k];
         Urow[r1]--;
         if (Urow[r1] == 0) {
            for (t1 = 1; t1 <= 4; t1++) {
               c2 = Col[r1][t1];
               V[c2]++;
            }
         }
      }
   }
   if (i > clues)
      goto keepgoing;
   return solutions;
}
