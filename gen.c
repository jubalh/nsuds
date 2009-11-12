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

#define MWC ((zr = 36969 * (zr & 65535) + (zr >> 16)) ^ (wr = 18000 * (wr & 65535)+(wr >> 16)))

static unsigned zr = 362436069, wr = 521288629;
static int Rows[325], Cols[730], Row[325][10], Col[730][5], Ur[730], Uc[325],
    V[325], W[325];
static int P[88], A[88], C[88], I[88], Two[888];
static int w, f, s1, m0, c1, c2, r1, l, i1, m1, m2, a, p, i, j, k, r, c, d, n =
729, m = 324, x, y, s, j;
static int q7, part, nt, rate, nodes, seed, solutions, min, clues;
static char L[11] = "0123456789";

extern char grid_data[9][9];
static int solve();

int do_generate(void)
{
   struct timeval tm;
   gettimeofday(&tm, NULL);
   srand(tm.tv_usec);

   seed = rand()*4149024;
   zr ^= seed;
   wr += seed;
   rate = 1;

   for (i = 0; i < 888; i++) {
      j = 1;
      while (j <= i)
         j *= 2;
      Two[i] = j - 1;
   }

   r = 0;
   for (x = 1; x <= 9; x++) {
      for (y = 1; y <= 9; y++) {
         for (s = 1; s <= 9; s++) {
            r++;
            Cols[r] = 4;
            Col[r][1] = (x - 1) * 9 + y;
            Col[r][2] = (3*((x-1)/3)+(y-1)/3)*9+s+81;
            Col[r][3] = (x - 1) * 9 + s + 81 * 2;
            Col[r][4] = (y - 1) * 9 + s + 81 * 3;
         }
      }
   }
   for (c = 1; c <= m; c++)
      Rows[c] = 0;

   for (r = 1; r <= n; r++) {
      for (c = 1; c <= Cols[r]; c++) {
         a = Col[r][c];
         Rows[a]++;
         Row[a][Rows[a]] = r;
      }
   }

   do {
      for (i = 1; i <= 81; i++)
         A[i] = 0;
      part = 0;
      q7 = 0;
      do {
         do {
            do
               i1 = (MWC >> 8) & 127;
            while (i1 > 80);
            i1++;
         }
         while (A[i1]);

         do
            s = (MWC >> 9) & 15;
         while (s > 8);
         s++;
         A[i1] = s;
         m2 = solve();
         q7++;
         /* add a random clue and solve it. No solution ==> remove it again.
          Not yet a unique solution ==> continue adding clues */
         if (m2 < 1)
            A[i1] = 0;
      }
      while (m2 != 1);

      /*now we have a unique-solution sudoku. Now remove clues to make it minimal*/
      part++;
   } while (solve() != 1);

   for (i = 1; i <= 81; i++) {
      do
         x = (MWC >> 8) & 127;
      while (x >= i);
      x++;
      P[i] = P[x];
      P[x] = i;
   }
   for (i1 = 1; i1 <= 81; i1++) {
      s1 = A[P[i1]];
      if (!s1)		/* don't solve if board is zero */
         continue;
      A[P[i1]] = 0;
      if (solve() > 1)
         A[P[i1]] = s1;
   }

   if (rate) {
      nt = 0;
      for (f = 0; f < 100; f++) {
         solve();
         nt += nodes;
      }
   }
   for (i = 0; i < 9; i++) {
      for (j = 0; j < 9; j++) {
         grid_data[i][j] = - (L[A[i * 9 + j + 1]]-'0');
      }
   }

   return nt;
}


/*returns 0 (no solution), 1 (unique sol.), 2 (more than one sol.) */
static int solve()			
{
   for (i = 0; i <= n; i++)
      Ur[i] = 0;
   for (i = 0; i <= m; i++)
      Uc[i] = 0;
   clues = 0;
   for (i = 1; i <= 81; i++) {
      if (A[i]) {
         clues++;
         r = (i - 1) * 9 + A[i];
         for (j = 1; j <= Cols[r]; j++) {
            d = Col[r][j];
            if (Uc[d])
               return 0;
            Uc[d]++;
            for (k = 1; k <= Rows[d]; k++) {
               Ur[Row[d][k]]++;
            }
         }
      }
   }
   for (c = 1; c <= m; c++) {
      V[c] = 0;
      for (r = 1; r <= Rows[c]; r++)
         if (Ur[Row[c][r]] == 0)
            V[c]++;
   }

   i = clues;
   m0 = 0;
   m1 = 0;
   solutions = 0;
   nodes = 0;

m2:
   i++;
   I[i] = 0;
   min = n + 1;
   if ((i <= 81) && !m0) {
      if (m1) {
         C[i] = m1;
      } else {
         w = 0;
         for (c = 1; c <= m; c++) {
            if (!Uc[c]) {
               if (V[c] < 2) {
                  C[i] = c;
                  goto keepgoing;
               } else {
                  if (V[c] <= min) {
                     w++;
                     W[w] = c;
                  }
                  if (V[c] < min) {
                     w = 1;
                     W[w] = c;
                     min = V[c];
                  }
               }
            }
         }
         do
            c2 = MWC & Two[w];
         while (c2 >= w);
         C[i] = W[c2 + 1];
      }
keepgoing:
      c = C[i];
      I[i]++;
      if (I[i] <= Rows[c]) {

         r = Row[c][I[i]];
         if (Ur[r])
            goto keepgoing;

         m0 = 0;
         m1 = 0;

         nodes++;

         for (j = 1; j <= Cols[r]; j++) {
            c1 = Col[r][j];
            Uc[c1]++;
         }
         for (j = 1; j <= Cols[r]; j++) {
            c1 = Col[r][j];
            for (k = 1; k <= Rows[c1]; k++) {
               r1 = Row[c1][k];
               Ur[r1]++;
               if (Ur[r1] == 1) {
                  for (l = 1; l <= Cols[r1]; l++) {
                     c2 = Col[r1][l];
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
   c = C[i];
   r = Row[c][I[i]];
   if (i == clues)
      return solutions;
   for (j = 1; j <= Cols[r]; j++) {
      c1 = Col[r][j];
      Uc[c1]--;
      for (k = 1; k <= Rows[c1]; k++) {
         r1 = Row[c1][k];
         Ur[r1]--;
         if (Ur[r1] == 0) {
            for (l = 1; l <= Cols[r1]; l++) {
               c2 = Col[r1][l];
               V[c2]++;
            }
         }
      }
   }
   if (i > clues)
      goto keepgoing;
   return solutions;
}
