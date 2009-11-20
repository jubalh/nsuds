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

/* Random number generator */
#define MWC ((zr = 36969 * (zr & 65535) + (zr >> 16)) ^ (wr = 18000 * (wr & 65535)+(wr >> 16)))
static unsigned zr = 362436069, wr = 521288629;


static short Rows[325], Row[325][10], Col[730][5], Ur[730], Uc[325], V[325], W[325];
static short P[88], A[88], C[88], I[88];
static int w, m0, c1, c2, r1, m1, n=729, m=324;
static int seed, solutions, min, clues;

extern char grid_data[9][9];
static int solve();

/* Generate a puzzle, put the result in grid_data */
void do_generate(void)
{
   struct timeval tm;
   int i,j,k;
   int x,y,s;     /* Temps */

   /* Seed rand() */
   gettimeofday(&tm, NULL);
   srand(tm.tv_usec);

   /* Seed MWC rand-generator */
   seed = 0.11 /*rand()*/*4149024;
   zr ^= seed;
   wr += seed;

   i = 0;
   for (x = 1; x <= 9; x++) {
      for (y = 1; y <= 9; y++) {
         for (s = 1; s <= 9; s++) {
            i++;
            Col[i][1] = (x - 1) * 9 + y;
            Col[i][2] = (3*((x-1)/3)+(y-1)/3)*9+s+81;
            Col[i][3] = (x - 1) * 9 + s + 81 * 2;
            Col[i][4] = (y - 1) * 9 + s + 81 * 3;
         }
      }
   }

   for (x = 1; x <= m; x++)
      Rows[x] = 0;

   for (s = 1; s <= n; s++) {
      for (y = 1; y <= 4; y++) {
         x = Col[s][y];
         Rows[x]++;
         Row[x][Rows[x]] = s;
      }
   }

   /* Add random clues until the puzzle has a unique solution. */
   do {
      int valid; /* Validity of puzzle (# solutions) */
      for (i = 1; i <= 81; i++)
         A[i] = 0;
      do {
         /* Choose a random unfilled square */
         do {
            do {
               x = (MWC >> 8) & 127;
            } while (x > 80);
            x++;
         }
         while (A[x]);

         /* Fill with random number */
         do {
            s = (MWC >> 9) & 15;
         } while (s > 8);
         s++;
         A[x] = s;

         valid = solve();
         /* If it makes the puzzle unsolvible, 
          * remove the invalid clue and try again */
         if (!valid) A[x] = 0;
      } while (valid != 1);

      /* Keep adding until the solution is unique */
   } while (solve() != 1);


   /* Now we have a unique-solution sudoku, remove 
    * clues to make it minimal */
   for (i = 1; i <= 81; i++) {
      do
         x = (MWC >> 8) & 127;
      while (x >= i);
      x++;
      P[i] = P[x];
      P[x] = i;
   }
   for (x = 1; x <= 81; x++) {
      y = A[P[x]];
      if (!y)		/* don't solve if board is zero */
         continue;
      A[P[x]] = 0;
      if (solve() > 1)
         A[P[x]] = y;
   }

   /* Transfer grid to grid_data  */
   for (i = 0; i < 9; i++) {
      for (j = 0; j < 9; j++) {
         /* Generated numbers are stored negative */
         grid_data[i][j] = - A[i * 9 + j + 1];
      }
   }
}


/*returns 0 (no solution), 1 (unique sol.), 2 (more than one sol.) */
static int solve()			
{
   int t1,t2,t3;
   int i,j,k;
   for (i = 0; i <= n; i++)
      Ur[i] = 0;
   for (i = 0; i <= m; i++)
      Uc[i] = 0;
   clues = 0;
   for (i = 1; i <= 81; i++) {
      if (A[i]) {
         clues++;
         t3 = (i - 1) * 9 + A[i];
         for (j = 1; j <= 4; j++) {
            t1 = Col[t3][j];
            if (Uc[t1])
               return 0;
            Uc[t1]++;
            for (k = 1; k <= 9; k++) {
               Ur[Row[t1][k]]++;
            }
         }
      }
   }
   for (t2 = 1; t2 <= m; t2++) {
      V[t2] = 0;
      for (t3 = 1; t3 <= 9; t3++)
         if (Ur[Row[t2][t3]] == 0)
            V[t2]++;
   }

   i = clues;
   m0 = 0;
   m1 = 0;
   solutions = 0;

m2:
   i++;
   I[i] = 0;
   min = n + 1;
   if ((i <= 81) && !m0) {
      if (m1) {
         C[i] = m1;
      } else {
         w = 0;
         for (t2 = 1; t2 <= m; t2++) {
            if (!Uc[t2]) {
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
         do
            c2 = MWC & 254;
         while (c2 >= w);
         C[i] = W[c2 + 1];
      }
keepgoing:
      t2 = C[i];
      I[i]++;
      if (I[i] <= 9) {

         t3 = Row[t2][I[i]];
         if (Ur[t3])
            goto keepgoing;

         m0 = 0;
         m1 = 0;


         for (j = 1; j <= 4; j++) {
            c1 = Col[t3][j];
            Uc[c1]++;
         }
         for (j = 1; j <= 4; j++) {
            c1 = Col[t3][j];
            for (k = 1; k <= 9; k++) {
               r1 = Row[c1][k];
               Ur[r1]++;
               if (Ur[r1] == 1) {
                  for (t1 = 1; t1 <= 4; t1++) {
                     c2 = Col[r1][t1];
                     V[c2]--;
                     if ((Uc[c2] + V[c2]) < 1)
                        m0 = c2;
                     if ((Uc[c2] == 0) && (V[c2] < 2))
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
      Uc[c1]--;
      for (k = 1; k <= 9; k++) {
         r1 = Row[c1][k];
         Ur[r1]--;
         if (Ur[r1] == 0) {
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
