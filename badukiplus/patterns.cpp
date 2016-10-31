/* Baduki - Computer Baduk(Go) Program 
   Copyright(c) 1998-1999 Lim Jaebum <artist@soback.kornet21.net>

   This program is free software; you can redistribute it and'or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
   USA */

/* $Id: patterns.c,v 1.13 1999/04/01 20:24:58 artist Exp $ */

#include <cstdlib>
#include <iostream>
#include <iomanip>
//#include <string>
#include <cstdio>
#include <cstring>

using namespace std;

#include "game.h"
#include "baduki.h"
#include "patterns.h"

extern game thegame;

extern stringRec sList[maxString];
extern int SMap[maxString];

extern int board_size;

// Difficult to maintain at this point (ADB)

extern struct pattern_type pattern[]; // array of patterns setup by ?

extern int total_patterns; // this is found at bottom of patternsval.cpp

struct pattern_type pat_conv(struct pattern_type pat, int mode)
{
  static struct pattern_type dpat;
  int i, j;

  memcpy(&dpat, &pat, sizeof(struct pattern_type));

  switch (mode) {
  case 0:
    break;
  case 1:
    dpat.x_size = pat.y_size;
    dpat.y_size = pat.x_size;
    dpat.x_pos = pat.y_pos;
    dpat.y_pos = pat.x_pos;
    for (i = 0; i < dpat.y_size; i++)
      for (j = 0; j < dpat.x_size; j++)
	dpat.pat[i * dpat.x_size + j] = pat.pat[j * pat.x_size + i];
    break;
  case 2:
    dpat.x_pos = pat.x_size - pat.x_pos - 1;
    for (i = 0; i < dpat.y_size; i++)
      for (j = 0; j < dpat.x_size; j++)
        dpat.pat[i * dpat.x_size + j] =
        	pat.pat[i * pat.x_size + (pat.x_size - j - 1)];
    break;
  case 3:
    dpat.x_size = pat.y_size;
    dpat.y_size = pat.x_size;
    dpat.x_pos = dpat.x_size - pat.y_pos - 1;
    dpat.y_pos = pat.x_pos;
    for (i = 0; i < dpat.y_size; i++)
      for (j = 0; j < dpat.x_size; j++)
	dpat.pat[i * dpat.x_size + j] =
		pat.pat[(pat.y_size - j - 1) * pat.x_size + i];
    break;
  case 4:
    dpat.y_pos = pat.y_size - pat.y_pos - 1;
    for (i = 0; i < dpat.y_size; i++)
      for (j = 0; j < dpat.x_size; j++)
        dpat.pat[i * dpat.x_size + j] =
        	pat.pat[(pat.y_size - i - 1) * pat.x_size + j];
    break;
  case 5:
    dpat.x_size = pat.y_size;
    dpat.y_size = pat.x_size;
    dpat.x_pos = pat.y_pos;
    dpat.y_pos = dpat.y_size - pat.x_pos - 1;
    for (i = 0; i < dpat.y_size; i++)
      for (j = 0; j < dpat.x_size; j++)
	dpat.pat[i * dpat.x_size + j] =
		pat.pat[j * pat.x_size + (pat.x_size - i - 1)];
    break;
  case 6:
    dpat.x_pos = pat.x_size - pat.x_pos - 1;
    dpat.y_pos = pat.y_size - pat.y_pos - 1;
    for (i = 0; i < dpat.y_size; i++)
      for (j = 0; j < dpat.x_size; j++)
        dpat.pat[i * dpat.x_size + j] =
        	pat.pat[(pat.y_size - i - 1) * pat.x_size + (pat.x_size - j - 1)];
    break;
  case 7:
    dpat.x_size = pat.y_size;
    dpat.y_size = pat.x_size;
    dpat.x_pos = dpat.x_size - pat.y_pos - 1;
    dpat.y_pos = dpat.y_size - pat.x_pos - 1;
    for (i = 0; i < dpat.y_size; i++)
      for (j = 0; j < dpat.x_size; j++)
	dpat.pat[i * dpat.x_size + j] =
		pat.pat[(pat.y_size - j - 1) * pat.x_size + (pat.x_size - i - 1)];
    break;
  }
  return dpat;
}

void print_pattern(struct pattern_type pat)
{
  int i, j;
  
  printf("id:%d value:%.1f type:%d\n", pat.id, pat.value, pat.type);
  printf("%d,%d - %d,%d\n", pat.x_size, pat.y_size, pat.x_pos, pat.y_pos);
  for (i = 0; i < pat.y_size; i++) {
    for (j = 0; j < pat.x_size; j++) {
      if (i == pat.y_pos && j == pat.x_pos)
        printf("@");
      else
        printf("%c", pat.pat[i*pat.x_size+j]);
    }
    printf("\n");
  }
}

int get_px(int x, int rx, int ry, struct pattern_type pat, int dir)
{
  switch (dir) {
  case 0:
    return x + rx;
    break;
  case 1:
    return x + ry;
    break;
  case 2:
    return x + pat.x_size - rx - 1;
    break;
  case 3:
    return x + pat.x_size - ry - 1;
    break;
  case 4:
    return x + rx;
    break;
  case 5:
    return x + ry;
    break;
  case 6:
    return x + pat.x_size - rx - 1;
    break;
  case 7:
    return x + pat.x_size - ry - 1;
    break;
  }
  return x;
}

int get_py(int y, int rx, int ry, struct pattern_type pat, int dir)
{
  switch (dir) {
  case 0:
    return y + ry;
    break;
  case 1:
    return y + rx;
    break;
  case 2:
    return y + ry;
    break;
  case 3:
    return y + rx;
    break;
  case 4:
    return y + pat.y_size - ry - 1;
    break;
  case 5:
    return y + pat.y_size - rx - 1;
    break;
  case 6:
    return y + pat.y_size - ry - 1;
    break;
  case 7:
    return y + pat.y_size - rx - 1;
    break;
  }
  return y;
}

int check_pattern(int x, int y, struct pattern_type pat, int dir)
{
  int i, j, px, py;
  float value;

  for (i = 0; i < pat.x_size; i++)
    for (j = 0; j < pat.y_size; j++) {
      switch (pat.pat[j * pat.x_size + i]) {
      case 'X':
	if (bord[i + x][j + y] != -1)
	  return FALSE;
	break;
      case 'O':
	if (bord[i + x][j + y] != 1)
	  return FALSE;
	break;
      case '.':
	if (bord[i + x][j + y] != 0)
	  return FALSE;
	break;
      case 'S':
	if (bord[i + x][j + y] != 0 || scoreBoard[i + x][j + y] < 1)
	  return FALSE;
	break;
      case 's':
	if (bord[i + x][j + y] != 0 || scoreBoard[i + x][j + y] > -1)
	  return FALSE;
	break;
      case 'T':
	if (bord[i + x][j + y] != 0 || claim[i + x][j + y] < 1)
	  return FALSE;
	break;
      case 't':
	if (bord[i + x][j + y] != 0 || claim[i + x][j + y] > -1)
	  return FALSE;
	break;
      case 'o':
	if (bord[i + x][j + y] == -1)
	  return FALSE;
	break;
      case 'x':
	if (bord[i + x][j + y] == 1)
	  return FALSE;
	break;
      case '%':
	if (bord[i + x][j + y] == 0)
	  return FALSE;
	break;
      case '*':
	break;
      default:
	fprintf(stderr, "Invallid Pattern!!!");
	exit(3);
	break;
      }
    }
    
  px = x + pat.x_pos;
  py = y + pat.y_pos;
  
  if ((value = pattern_value(x, y, pat, dir, &px, &py)) > 0)
    add_altmoves(px, py, value, pat.id);

  return TRUE;
}

int pattern_match(struct pattern_type pat)
{
  int i, j, d;
  struct pattern_type dpat;

  for (d = 0; d < 8; d++) {
    if (d && (pat.type & (1 << (d + P_IGNORE_D))))
      continue;
    dpat = pat_conv(pat, d);
    if (pat.type & PT_WHOLE) {
      for (i = 0; i <= board_size - dpat.x_size; i++) {
	for (j = 0; j <= board_size - dpat.y_size; j++) {
	  check_pattern(i, j, dpat, d);
	}
      }
    } else {
      if (pat.type & PT_CENTER) {
	for (i = 1; i <= maxPoint - dpat.x_size; i++) {
	  for (j = 1; j <= maxPoint - dpat.y_size; j++) {
	    check_pattern(i, j, dpat, d);
	  }
	}
      }
      if (pat.type & PT_SIDE) {
	if (d == 0 || d == 2)
	  for (i = 1; i <= maxPoint - dpat.x_size; i++) {
	    check_pattern(i, 0, dpat, d);
	  }
	if (d == 4 || d == 6)
	  for (i = 1; i <= maxPoint - dpat.x_size; i++) {
	    check_pattern(i, board_size - dpat.y_size, dpat, d);
	  }
	if (d == 1 || d == 5)
	  for (i = 1; i <= maxPoint - dpat.y_size; i++) {
	    check_pattern(0, i, dpat, d);
	  }
	if (d == 3 || d == 7)
	  for (i = 1; i <= maxPoint - dpat.y_size; i++) {
	    check_pattern(board_size - dpat.x_size, i, dpat, d);
	  }
      }
      if (pat.type & PT_CORNER) {
	if (d == 0 || d == 1)
	  check_pattern(0, 0, dpat, d);
	if (d == 4 || d == 5)
	  check_pattern(0, board_size - dpat.y_size, dpat, d);
	if (d == 6 || d == 7)
	  check_pattern(board_size - dpat.x_size, board_size - dpat.y_size, dpat, d);
	if (d == 2 || d == 3)
	  check_pattern(board_size - dpat.x_size, 0, dpat, d);
      }
    }
  }
  return TRUE;
}

Getmove(find_pattern)
{
  int i;

  for (i = 0; i < total_patterns; i++)
    pattern_match(pattern[i]);
}

int is_same_pattern(struct pattern_type pat1, struct pattern_type pat2)
{
  if (pat1.x_size != pat2.x_size)
    return FALSE;
  if (pat1.y_size != pat2.y_size)
    return FALSE;
  if (pat1.x_pos != pat2.x_pos)
    return FALSE;
  if (pat1.y_pos != pat2.y_pos)
    return FALSE;
  if (strcmp(pat1.pat, pat2.pat))
    return FALSE;
  return TRUE;
}

void init_pattern()
{
  int i, d, j;
  struct pattern_type dpat[8];

  for (i = 0; i < total_patterns; i++) {
    for (d = 0; d < 8; d++) {
      dpat[d] = pat_conv(pattern[i], d);
      for (j = 0; j < d; j++) {
        if (is_same_pattern(dpat[j], dpat[d])) {
          if (!(((dpat[j].type & PT_SIDE) && ((j == 0 && d != 2) ||
          	(j == 4 && d != 6) ||(j == 1 && d != 5) ||(j == 3 && d != 7))) ||
          	((dpat[j].type & PT_CORNER) && ((j == 0 && d != 1) ||
          	(j == 2 && d != 3) ||(j == 4 && d != 5) ||(j == 6 && d != 7)))))
          pattern[i].type |= (1 << (d + P_IGNORE_D));
        }
      }
#ifdef DEBUG
      printf("--- %d, %d ---\n", i, d);
      print_pattern(dpat[d]);
#endif
    }
#ifdef DEBUG
    printf("==========\n");
#endif
  }
}
