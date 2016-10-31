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

/* $Id: think.c,v 1.29 1999/04/12 16:29:28 artist Exp artist $ */

/* The go player */
/* Ported from Pascal to C by Todd R. Johnson 4/17/88 */
/* From the original pascal file:
   Go Move Generator
   Copyright (c) 1983 by Three Rivers Computer Corp.
   Written: January 17, 1983 by Stoney Ballard */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <cstdlib>
#include <cmath>
#include <iostream>
#include <string>
#include <algorithm>

using namespace std;

#include "game.h"
#include "baduki.h"

extern game thegame;

extern struct bRec goboard[19][19];
extern int ko, koX, koY;

extern int SMap[maxString];
extern groupRec gList[MAX_GROUP], wgList[MAX_GROUP];

extern int maxStringID;
extern int treeLibLim;
extern int killFlag;
extern int depthLimit;
extern int showTrees;
extern int utilPlayLevel;
extern stringRec sList[maxString];
extern int sGlist[maxString + 1];
extern pointList pList;
extern pointList pList1;
extern pointList plist2;
extern pointList plist3;
extern sType mySType;
extern int show_altmoves;
extern int sgf_mode;
extern int gmp_mode;
extern int current_move;
extern int ignore_takecorner;
extern int board_size;

extern struct dmode_type dmode[];
extern int display_mode;

pointList dapList1, dapList2, dapList3;
char *playReason;
int maxPlayLevel = 7;
int playLevel = 7;

intList aList;

void genBord(enum bVal color)
{
  int x, y, nomoves = TRUE;
/*  char mv[8]; */

  maxPlayLevel = 7;
  utilPlayLevel = playLevel;
  mySType = color;
/*  if (playLevel < 2)
    treeLibLim = 2;
  else
    treeLibLim = 5; */
  treeLibLim = playLevel - 3;
  if (treeLibLim < 2)
    treeLibLim = 2;
  else if (treeLibLim > 9)
    treeLibLim = 9;
  depthLimit = 10;
  for (y = 0; y <= 18; y++)
    for (x = 0; x <= 18; x++)
      if (goboard[x][y].Val == color) {
	bord[x][y] = 1;
	legal[x][y] = FALSE;
	nomoves = FALSE;
      } else if (goboard[x][y].Val == EMPTY) {
	bord[x][y] = 0;
	if (Suicide(color, x, y))
	  legal[x][y] = FALSE;
	else
	  legal[x][y] = TRUE;
      } else {
	bord[x][y] = -1;
	legal[x][y] = FALSE;
	nomoves = FALSE;
      }
  if (ko) {
    legal[koX][koY] = FALSE;
  }
  if (!nomoves)
    genState();
  else
    initGPUtils();
}

#define ALT_MOVES	300
struct altmovestype altmoves[ALT_MOVES];

struct moveseqtype move_seq[] = {
  {BB_TAKECORNER, takeCorner, ("TakeCorner"), 0},
  {BB_FINDPAT, find_pattern, ("Pattern"), 0},
  {BB_JUMP1, thk_jump1, ("Jump1"), 0},
  {BB_SAVE, lookForSave, ("Save"), 0},
/*  {BB_SAVEN, lookForSaveN, ("SaveN"), 0}, */
  {BB_EXTEND, extend, ("extend"), 0},
  {BB_KILL, lookForKill, ("Kill"), 0},
  {BB_DOUBLE, doubleAtari, ("2Atari"), 0},
  {BB_CUTNKILL, cutNkill, ("cutNkill"), 0},
/*  {BB_ATTACK, lookForAttack, ("Attack"), 0}, */
/*  {threaten, ("threaten"), 0}, */
  {BB_EXTEND2, extend2, ("extend2"), 0},
  {BB_CONNECT, connectCut, ("conCut"), 0},
  {BB_BLOCK, blockCut, ("blockCut"), 0},
  {BB_CUT, cutHim, ("cutHim"), 0},
  {BB_WALL, extendWall, ("extWall"), 0},
  {BB_WALL2, extendWall2, ("extWall2"), 0},
  {BB_ATTACK2, findAttack2, ("Attack2"), 0},
  {BB_PUSH, pushWall, ("pushWall"), 0},
  {BB_EDGE, dropToEdge, ("ToEdge"), 0},
  {BB_UNDER, underCut, ("underCut"), 0},
  {BB_BLOCK1, BlockUnderCut, ("BlkCut1"), 0},
  {BB_ATARI, atariAnyway, ("atari"), 0},
  {BB_REDUCE, reduceHisLiberties, ("rHisLib."), 0},
/*  {BB_EDGE2, dropToEdge2, ("ToEdge2"), 0}, */
/*  {BB_SAVE2, lookForSave2, ("Save2"), 0}, */
  {BB_DAME, fill_dame, ("Dame"), 0},
  {0, NULL, "", 0}
};

int last_altmove, max_altvalue;
void clear_altmoves(void)
{
  int i;
  
  for (i = 0; i < ALT_MOVES; i++) {
    altmoves[i].px = 0;
  }
  last_altmove = 0;
  max_altvalue = 0;
/*  if (show_altmoves)
    printf("-------\n"); */
}

void add_altmoves(int x, int y, float fvalue, int target)
{
  int i;
  int value = (int) floor( fvalue + .5 ); // round to nearest integer
  static int why;
//  char buf[10];
  
  if (x == -1 && y == -1) {
    why = value;
    return;
  }
  
  if (last_altmove >= ALT_MOVES) {
//    fprintf(stderr, "!!!! Too many alt moves!!!!\n");
    return;
  }
  if (!legal[x][y]) {
/*    fprintf(stderr, "!!! Illegal Move !!!\n"); */
    return;
  }

  altmoves[last_altmove].px = x + 1;
  altmoves[last_altmove].py = y + 1;
  altmoves[last_altmove].value = value;
  altmoves[last_altmove].target = target;
  altmoves[last_altmove].why = why;
  
  last_altmove++;
  if (value > max_altvalue)
    max_altvalue = value;
    
/*  if (show_altmoves && sgf_mode)
    printf("%c %2d : %3.1f %s\n", x < 8 ? x + 'A' : x + 'B',
  	y + 1, fvalue, move_seq[why].why);
 */
  i = last_altmove - 1;
  
  if (valueBoard[altmoves[i].px-1][altmoves[i].py-1] < altmoves[i].value * 10)
    valueBoard[altmoves[i].px-1][altmoves[i].py-1] = (int) floor( (altmoves[i].value * 10) + .5 );
/*  
  if (!gmp_mode && show_altmoves && dmode[display_mode].mode == DMODE_PLAY)
    if (valueBoard[altmoves[i].px-1][altmoves[i].py-1] < altmoves[i].value * 10) {
      sprintf(buf, "%.0f", altmoves[i].value);
      sprintf(buf, "%d", (valueBoard[altmoves[i].px-1][altmoves[i].py-1] + 5) / 10);
      gtk_go_board_set_label(go_board, altmoves[i].px, altmoves[i].py, buf);
    }
 */

  if (valueBoard[altmoves[i].px-1][altmoves[i].py-1] < altmoves[i].value * 10)
    valueBoard[altmoves[i].px-1][altmoves[i].py-1] = (int) floor( (altmoves[i].value * 10) + .5 );

//  while (!gmp_mode && gtk_events_pending())
//	gtk_main_iteration();
}

#define ACT_KILL	-1
#define ALT_ID(i)	move_seq[altmoves[(i)].why].id
/* 현재 수에 추가 점수 주기 : 예술인 (98. 8. 20) */

void add_altpower(int id, int target, float fvalue)
{
  int i;
  int value = (int) floor( fvalue + .5 ); // round to nearest integer
  
  for (i = 0; altmoves[i].px; i++) {
    if (target == altmoves[i].target &&
    	(id == ALT_ID(i) || (id == ACT_KILL &&
    	(ALT_ID(i) == BB_KILL || ALT_ID(i) == BB_ATTACK)))) {
      altmoves[i].value += value;
      if (valueBoard[altmoves[i].px-1][altmoves[i].py-1] < altmoves[i].value * 10)
        valueBoard[altmoves[i].px-1][altmoves[i].py-1] = (int) floor( (altmoves[i].value * 10) + .5 );
    }
  }
}

void add_altpower2(int id, int x, int y, float fvalue)
{
  int i;
  int value = (int) floor( fvalue + .5 ); // round to nearest integer
  
  if (valueBoard[x][y] == 0)
    return;
    
  for (i = 0; altmoves[i].px; i++) {
    if (x == altmoves[i].px - 1&& y == altmoves[i].py - 1 &&
    	(id == ALT_ID(i) || (id == ACT_KILL &&
    	(ALT_ID(i) == BB_KILL || ALT_ID(i) == BB_ATTACK)))) {
      altmoves[i].value += value;
      if (valueBoard[altmoves[i].px-1][altmoves[i].py-1] < altmoves[i].value * 10)
        valueBoard[altmoves[i].px-1][altmoves[i].py-1] = (int) floor( (altmoves[i].value * 10) + .5 );
    }
  }
}

int find_altmoves(int id, int x, int y)
{
  int i;

  for (i = 0; altmoves[i].px; i++) {
    if (x == altmoves[i].px - 1 && y == altmoves[i].py - 1 &&
    	(id == ALT_ID(i) || (id == ACT_KILL &&
    	(ALT_ID(i) == BB_KILL || ALT_ID(i) == BB_ATTACK)))) {
      return (int) altmoves[i].value;
    }
  }
  return FALSE;
}

void dd_pause()
{
}

int getMove(int *x, int *y)
{
  int i, j, dx, dy, max_i = 0, ix, iy, jx, jy, libc, found, tgid;
  float max_value = 0, value;
//  char buf[80];
  
  clear_altmoves();
  for (i = 0; move_seq[i].get; i++) {
    add_altmoves(-1, -1, i, 0);
    move_seq[i].get();
  }
  
  restoreState();
  
  /* 추가 기능이 있는지 찾아서 점수 더하기 */
  for (i = 0; i < last_altmove; i++) {
    if (altmoves[i].value > max_value) {
      max_value = altmoves[i].value;
      max_i = i;
    }
    switch(ALT_ID(i)) {
      case BB_SAVE:
      case BB_KILL:
/*      case BB_CONNECT:
      case BB_BLOCK:
      case BB_JUMP1: */
        ix = sList[altmoves[i].target].lx;
        iy = sList[altmoves[i].target].ly;
        for (j = 0; j < last_altmove; j++) {
          value = 0;
          jx = sList[altmoves[j].target].lx;
          jy = sList[altmoves[j].target].ly;
          if (j != i && ALT_ID(j) == BB_SAVE) {
	    tryPlay(altmoves[i].px - 1, altmoves[i].py -1, 1);
            if (!killable(jx, jy, &dx, &dy)) {
              if (ALT_ID(i) == BB_KILL) {
                value = (float)altmoves[i].value + (float)altmoves[j].value * 0.9;
              } else
                value = (float)altmoves[i].value + (altmoves[j].value > 2 ?
              		(float)altmoves[j].value - 2 : 0.1);
	    }
	    restoreState();
          } else if (ALT_ID(j) == BB_KILL && 
          	ALT_ID(i) == BB_SAVE) {
/* 돌을 살릴때 잡으려는 돌이 자동으로 죽는 경우도 고려 */
            tryPlay(altmoves[i].px - 1, altmoves[i].py -1, 1);
            /* saveable에 들어가면 상태가 변하므로 미리 값을 구해놓는다 */
            libc = sList[SMap[stringIDs[ix][iy]]].libC;
            if (!saveable(jx, jy, &dx, &dy)) {
              value = (float)altmoves[i].value + (float) altmoves[j].value * 0.9;
              if (libc <= 2) {
                value -= 0.3;
              }
            }
            restoreState();
          }
          if (valueBoard[altmoves[i].px-1][altmoves[i].py-1] < value * 10) {
            valueBoard[altmoves[i].px-1][altmoves[i].py-1] = (int) floor((value * 10) + .5 );
	    if (value > max_value) {
	      max_value = value;
	      max_i = i;
	    }
          }
        }
        break;
    }
  }

  if (max_value == 0)
    return FALSE;
  
  /* 다음에 놓을 수가 안전한지를 검사해서 안전하지 않으면 무시한다. */
  found = FALSE;
  while(!found) {
    if (safeMove(altmoves[max_i].px - 1, altmoves[max_i].py - 1)) {
    /* 이수를 둘때 다른 안전한 돌이 위험해지는지 검사한다 */
/* 드물지만 공배메우기등을 하면서 인접한돌이 촉촉수에 걸리게
   되는 경우가 생기는 것을 방지한다.
   이를 해결하기 위해서는 공배메우기 즈음에 촉촉수에 걸려서
   결국 다 이어야 할것들을 찾아서 단수 맞기전에 미리 메우도록 해야한다. */
      tryPlay(altmoves[max_i].px - 1, altmoves[max_i].py - 1, 1);
      found = TRUE;
      for (i = 1; i <= maxStringID; i++) {
        if (bord[sList[i].lx][sList[i].ly] != 1 || !sList[i].isSafe)
          continue;
          /* killable()이 같은 상황에서 다른 결과를 내보내는 경우때문에
          markDead에서 안전하다고 했던돌이 이번에는 잡히는 돌로 처리하는
          경우가 생기고 이렇게 되면 중반에 갑자기 계속 착수를 포기하는
          일이 생긴다. */
        if (claim[altmoves[max_i].px - 1][altmoves[max_i].py - 1] > 0)
          tgid = wGIDMap[altmoves[max_i].px - 1][altmoves[max_i].py - 1];
        else if (altmoves[max_i].px > 1 &&
        	bord[altmoves[max_i].px - 2][altmoves[max_i].py - 1] == 1)
          tgid = wGIDMap[altmoves[max_i].px - 2][altmoves[max_i].py - 1];
        else if (altmoves[max_i].py > 1 &&
        	bord[altmoves[max_i].px - 1][altmoves[max_i].py - 2] == 1)
          tgid = wGIDMap[altmoves[max_i].px - 1][altmoves[max_i].py - 2];
        else if (altmoves[max_i].px < board_size &&
        	bord[altmoves[max_i].px][altmoves[max_i].py - 1] == 1)
          tgid = wGIDMap[altmoves[max_i].px][altmoves[max_i].py - 1];
        else if (altmoves[max_i].py < board_size &&
        	bord[altmoves[max_i].px - 1][altmoves[max_i].py] == 1)
          tgid = wGIDMap[altmoves[max_i].px - 1][altmoves[max_i].py];
        else
          tgid = 0;
        if (tgid && wGIDMap[sList[i].lx][sList[i].ly] != tgid)
          continue;
        if (killable(sList[i].lx, sList[i].ly, &dx, &dy) &&
        	max_value < sList[i].size * 20) {
/*          fprintf(stderr, "### %d (%d,%d) value:%.1f ID:%d size:%d ###\n",
          	current_move + 1, sList[i].lx, sList[i].ly,
          	max_value / 10, i, sList[i].size); */
          found = FALSE;
          break;
        }
      }
      restoreState();
    }
    if (found)
      break;
    valueBoard[altmoves[max_i].px-1][altmoves[max_i].py-1] =
    	-valueBoard[altmoves[max_i].px-1][altmoves[max_i].py-1];

/*
    if (!gmp_mode && show_altmoves && dmode[display_mode].mode == DMODE_PLAY) {
      sprintf(buf, "%d", (valueBoard[altmoves[max_i].px-1][altmoves[max_i].py-1] + 5) / 10);
      gtk_go_board_set_label(go_board, altmoves[max_i].px, altmoves[max_i].py, buf);
    }
    while (!gmp_mode && gtk_events_pending())
    gtk_main_iteration();
 */
    max_value = 0;
    for (i = 0; i < last_altmove; i++) {
      if (valueBoard[altmoves[i].px-1][altmoves[i].py-1] > max_value) {
        max_i = i;
        max_value = valueBoard[altmoves[i].px-1][altmoves[i].py-1];
      }
    }
    if (max_value == 0)
      return FALSE;
  }
  *x = altmoves[max_i].px - 1;
  *y = altmoves[max_i].py - 1;
  playReason = move_seq[altmoves[max_i].why].why;
  if (showTrees) {
//    reportReason();
  }

  return (int) max_value;
}

int genMove(enum bVal color, int *x, int *y)
{
  genBord(color);
  if (getMove(x, y))
    return TRUE;
  return FALSE;
}

int checkPos(int x, int y, int field)
{
  int ok;
  ok = (((field == 0) && (claim[x][y] == 0)) ||
      ((field > 0) &&
	  (claim[x][y] >= 0) && (claim[x][y] <= field)) ||
      ((field < 0) &&
	  (claim[x][y] <= 0) && (claim[x][y] >= field))) &&
      (bord[x - 1][y] == 0) &&
      (bord[x + 1][y] == 0) &&
      (bord[x][y - 1] == 0) &&
      (bord[x][y + 1] == 0);
  if (ok)
    return TRUE;
  else
    return FALSE;
}

/* int takeCorner(x, y)
     int *x, *y; */
Getmove(old_takeCorner)
{
  int field = -1, i, value = 5;
  i = 18 - 3;

  while (field != -4) {
    if (field == -1)
      field = 0;
    else if (field == 0)
      field = 4;
    else
      field = -4;
    if (checkPos(2, 3, field)) {
      add_altmoves(2, 3, value, 0);
    }
    if (checkPos(3, 2, field)) {
      add_altmoves(3, 2, value, 0);
    }
    if (checkPos(2, i, field)) {
      add_altmoves(2, i, value, 0);
    }
    if (checkPos(3, i + 1, field)) {
      add_altmoves(3, i + 1, value, 0);
    }
    if (checkPos(i, i + 1, field)) {
      add_altmoves(i, i + 1, value, 0);
    }
    if (checkPos(i + 1, i, field)) {
      add_altmoves(i + 1, i, value, 0);
    }
    if (checkPos(i, 2, field)) {
      add_altmoves(i, 2, value, 0);
    }
    if (checkPos(i + 1, 3, field)) {
      add_altmoves(i + 1, 3, value, 0);
    }
    if (checkPos(2, 4, field)) {
      add_altmoves(2, 4, value, 0);
    }
    if (checkPos(4, 2, field)) {
      add_altmoves(4, 2, value, 0);
    }
    if (checkPos(2, i - 1, field)) {
      add_altmoves(2, i - 1, value, 0);
    }
    if (checkPos(4, i + 1, field)) {
      add_altmoves(4, i + 1, value, 0);
    }
    if (checkPos(i - 1, i + 1, field)) {
      add_altmoves(i - 1, i + 1, value, 0);
    }
    if (checkPos(i + 1, i - 1, field)) {
      add_altmoves(i + 1, i - 1, value, 0);
    }
    if (checkPos(i + 1, 4, field)) {
      add_altmoves(i + 1, 4, value, 0);
    }
    if (checkPos(i - 1, 2, field)) {
      add_altmoves(i - 1, 2, value, 0);
    }
  }
  return;
}

/* 빈 귀 차지 */
/* 빈귀중 하나를 선택하고 화점, 소목, 삼삼, 외목, 고목을
  4:3:1:1:1의 비율로 선택한다. */
Getmove(takeCorner)
{
  int x, y, d, dx, dy, vdc = 0;
  int cmine[4] = {0,0,0,0}, chis[4] = {0,0,0,0}, vd[4];
  float value = 5.5;
  
  if (ignore_takecorner)
    return;

  for (d = 0; d < 4; d++) {
    for (x = 0; x < 7; x++)
      for (y = 0; y < 7; y++) {
        get_cxy(d, x, y, &dx, &dy);
        if (bord[dx][dy] == 1)
          cmine[d]++;
        else if (bord[dx][dy] == -1)
          chis[d]++;
      }
    if ((cmine[d] + chis[d]) < 1) {
      vd[vdc] = d;
      vdc++;
    }
  }
  
  if (vdc == 0) {
    ignore_takecorner = TRUE;
    return;
  }
  
  d = random() % vdc;
  switch(random() % 20) {
  case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7:
    x = 3; y = 3;	/* 화점 */
    break;
  case 8: case 9: case 10:
    x = 3; y = 2;
    break;
  case 11: case 12: case 13:
    x = 2; y = 3;	/* 소목 */
    break;
  case 14: case 15:
    x = 2; y = 2;	/* 삼삼 */
    break;
  case 16:
    x = 4; y = 3;	/* 고목 */
    break;
  case 17:
    y = 3; x = 4;
    break;
  case 18:
    x = 4; y = 2;	/* 외목 */
    break;
  case 19:
    x = 2; y = 4;
    break;
  }
  get_cxy(vd[d], x, y, &dx, &dy);

  add_altmoves(dx, dy, value, vd[d]);
}

/*
void printBoard(intBoard brd, char *name)
{
  int x, y;
  printf("%s\n", name);
  for (y = 0; y <= 18; y++) {
    for (x = 0; x <= 18; x++)
      printf("%2d ", brd[x][y]);
    printf("\n");
  }
}
 */

int noNbrs(int x, int y)
{
  if (x > 0 && bord[x - 1][y] != 0)
    return FALSE;
  if (x < 18 && bord[x + 1][y] != 0)
    return FALSE;
  if (y > 0 && bord[x][y - 1] != 0)
    return FALSE;
  if (y < 18 && bord[x][y + 1] != 0)
    return FALSE;
  return TRUE;
}


/* 벌림 */
Getmove(extend)
{
  int i;
  float value = 4.0;

  for (i = 2; i <= 18 - 2; i++)
    if (claim[2][i] == 0 && noNbrs(2, i)) {
      if (bord[2][i-1] == 1 && safeConnect[4][i-1] > 0 && libBoard[4][i-1] == 4)
	add_altmoves(4, i - 1, value - 0.5, 0);
      else if (bord[2][i+1] == 1 && safeConnect[4][i+1] > 0 && libBoard[4][i-1] == 4)
	add_altmoves(4, i + 1, value - 0.5, 0);
      else if (bord[2][i-1] == 0 && bord[2][i+1] == 0) {
        add_altmoves(2, i, value + (float) powerBoard2[2][i] / 25, 0);
/*        break; */
      }
    }
  for (i = 2; i <= 18 - 2; i++)
    if (claim[i][18 - 2] == 0 && noNbrs(i, 18 - 2)) {
      if (bord[i-1][18-2] == 1 && safeConnect[i-1][18 - 4] > 0 &&
      		libBoard[i-1][18 - 4] == 4)
	add_altmoves(i-1, 18 - 4, value - 0.5, 0);
      else if (bord[i+1][18-2] == 1 && safeConnect[i+1][18 - 4] > 0 &&
      		libBoard[i-1][18 - 4] == 4)
	add_altmoves(i+1, 18 - 4, value - 0.5, 0);
      else if (bord[i-1][18-2] == 0 && bord[i+1][18-2] == 0) {
        add_altmoves(i, 18 - 2, value + (float) powerBoard2[i][18-2] / 25, 0);
/*        break; */
      }
    }
  for (i = 18 - 2; i >= 2; i--)
    if (claim[18 - 2][i] == 0 && noNbrs(18 - 2, i)) {
      if (bord[18-2][i-1] == 1 && safeConnect[18-4][i-1] > 0 &&
      		libBoard[18-4][i-1] == 4)
	add_altmoves(18-4, i-1, value - 0.5, 0);
      else if (bord[18-2][i+1] == 1 && safeConnect[18-4][i+1] > 0 &&
      		libBoard[18-4][i-1] == 4)
	add_altmoves(18-4, i+1, value - 0.5, 0);
      else if (bord[18-2][i-1] == 0 && bord[18-2][i+1] == 0) {
        add_altmoves(18 - 2, i, value + (float) powerBoard2[18 - 2][i] / 25, 0);
/*        break; */
      }
    }
  for (i = 18 - 2; i >= 2; i--)
    if (claim[i][2] == 0 && noNbrs(i, 2)) {
      if (bord[i-1][2] == 1 && safeConnect[i-1][4] > 0 &&
      		libBoard[i-1][4] == 4)
	add_altmoves(i-1, 4, value - 0.5, 0);
      else if (bord[i+1][2] == 1 && safeConnect[i+1][4] > 0 &&
      		libBoard[i-1][4] == 4)
	add_altmoves(i+1, 4, value - 0.5, 0);
      else if (bord[i-1][2] == 0 && bord[i+1][2] == 0) {
        add_altmoves(i, 2, value + (float) powerBoard2[i][2] / 25, 0);
/*        break; */
      }
    }
  return;
}

Getmove(extend2)
{
  int i, lowest = 999, value, x = -1, y = -1;

  for (i = 3; i <= 18 - 3; i++)
    if (legal[2][i]) {		/* if there is nobody there */
      value = claim[2][i];	/* get influence */
      if ((value < 7) &&	/* a reasonable hole in my wall */
	  (value > -5) &&	/* or a reasonable gap in his */
	  (bord[2][i + 1] == 0) &&	/* not in contact with any stones */
	  (bord[2][i - 1] == 0))
	if (value < lowest) {
	  lowest = value;	/* lowest gets the smallest value */
	  x = 2;		/* that was seen along all the 3-lines */
	  y = i;		/* x and y save that location */
	}
    }

  for (i = 3; i <= 18 - 3; i++)
    if (legal[i][2]) {
      value = claim[i][2];
      if ((value < 7) &&
	  (value > -5) &&
	  (bord[i + 1][2] == 0) &&
	  (bord[i - 1][2] == 0))
	if (value < lowest) {
	  lowest = value;
	  x = i;
	  y = 2;
	}
    }
  for (i = 18 - 3; i >= 3; i--)
    if (legal[18 - 2][i]) {
      value = claim[18 - 2][i];
      if ((value < 7) &&
	  (value > -5) &&
	  (bord[18 - 2][i + 1] == 0) &&
	  (bord[18 - 2][i - 1] == 0))
	if (value < lowest) {
	  lowest = value;
	  x = 18 - 2;
	  y = i;
	}
    }
  for (i = 3; i <= 18 - 3; i++)
    if (legal[i][18 - 2]) {
      value = claim[i][18 - 2];
      if ((value < 7) &&
	  (value > -5) &&
	  (bord[i + 1][18 - 2] == 0) &&
	  (bord[i - 1][18 - 2] == 0))
	if (value < lowest) {
	  lowest = value;
	  x = i;
	  y = 18 - 2;
	}
    }
  if (lowest == 999)
    return;

  if (x != -1 && y != -1 && powerBoard[x][y] > 5) {
    float value;
    value = powerBoard[x][y] / 3;
    if (value > 4)
      value = 4.1;
    add_altmoves(x, y, value, 0);
  }
  return;
}

/* 한칸 뜀 : 예술인 (98. 9. 22) */
Getmove(thk_jump1)
{
  int i, j, k, bdir, edir, max_value, dx, dy;
  float value = 5.5;
  
  struct vpoint {
    int x;
    int y;
    int value;
  } dp[4];
  
  for (i = 2; i <= maxPoint - 2; i++)
    for (j = 2; j <= maxPoint - 2; j++) {
      if (ndbord[i][j] != 0) {
        k = 0;
	dp[k].x = i - 2;
	dp[k].y = j;
	dp[k].value = (i == 2) ? -999 : 
		ndbord[i][j] == 1 ? claim[i-2][j] : -claim[i-2][j];
	k++;
	dp[k].x = i + 2;
	dp[k].y = j;
	dp[k].value = (i == maxPoint -2) ? -999 :
		ndbord[i][j] == 1 ? claim[i+2][j] : -claim[i+2][j];
	k++;
	dp[k].x = i;
	dp[k].y = j - 2;
	dp[k].value = (j == 2) ? -999 : 
		ndbord[i][j] == 1 ? claim[i][j-2] : -claim[i][j-2];
	k++;
	dp[k].x = i;
	dp[k].y = j + 2;
	dp[k].value = (j == maxPoint -2) ? -999 : 
		ndbord[i][j] == 1 ? claim[i][j+2] : claim[i][j+2];
	for (k = 0, bdir = 0; k < 4; k++) {
	  if (dp[k].value <= 0)
	    bdir++;
	}
	if (bdir >= 3) {
	  for (k = 0, max_value = -5, edir = -1; k < 4; k++) {
	    value = ndbord[i][j] == 1 ? 2.5 : 3.1;
	    if (libBoard[dp[k].x][dp[k].y] == 4 &&
	    	libBoard[(i+dp[k].x) / 2][(j+dp[k].y) / 2] == 3) {

		/* 3선에 있는 두칸벌림이 가능할 경우 */
	      if (dp[k].x == i && (i == 2 || i == maxPoint - 2)) {
	        dy = j + ((dp[k].y - j) / 2) * 3;
	        if (libBoard[i][dy] == 4) {
	          if (bord[i][j] == 1)
	            dp[k].y = dy;
	          dp[k].value += 10;
	          value += 1;
	        }
	      } else if (dp[k].y == j && (j == 2 || j == maxPoint - 2)) {
	        dx = i + ((dp[k].x - i) / 2) * 3;
	        if (libBoard[dx][j] == 4) {
	          if (bord[i][j] == 1)
	            dp[k].x = dx;
	          dp[k].value += 10;
	          value += 1;
	        }
	      }
	      if (dp[k].value > max_value) {
	        edir = k;
	        max_value = dp[k].value;
	      }
	      value += (float) powerBoard[dp[k].x][dp[k].y] / 7;
	      value -= (float) wgList[wGIDMap[i][j]].eyes / 2;
	      value -= (float) wgList[wGIDMap[i][j]].exts / 2;
	      if (value > 0)
	        add_altmoves(dp[k].x, dp[k].y, value, stringIDs[i][j]);
	    }
	  }
/*	  if (edir != -1 && wgList[wGIDMap[i][j]].eyes < 4 &&
	  	((ndbord[i][j] == 1 && scoreBoard[dp[edir].x][dp[edir].y] < 1) ||
	  	(ndbord[i][j] == -1 && powerBoard1[dp[edir].x][dp[edir].y] > 10)) &&
	    	max_value > -5 && height_cmp(i, j, dp[edir].x, dp[edir].y) <= 0) {
	      add_altmoves(dp[edir].x, dp[edir].y, value, stringIDs[i][j]);
	  } */
	}
      }
    }
}

/* 단수 된 돌 살리기 */
/* 살릴 수 있는돌 살리기 */
Getmove(lookForSave)
{
  int i, j, safe;
  float value;

  for (i = 1; i <= maxStringID; i++)	/* scan the string list */
    if ((ndbord[sList[i].lx][sList[i].ly] == 1) &&
    	!sList[i].isSafe && !sList[i].isDead) {

        listAdjacents(sList[i].lx, sList[i].ly, &aList);
        safe = FALSE;
        for (j = 1; j <= aList.indx; j++) {
          if (sList[aList.v[j]].isDead) {
            safe = TRUE;
          }
        }
        
        if (safe)
          continue;

        value = sList[i].size * 2;
        if (sList[i].libC == 1 &&
        	myStones[sList[i].sx][sList[i].sy] == 4) {
          value--;
          if (sList[i].size > 1)
            value--;
        } else {
          value += libBoard[sList[i].sx][sList[i].sy];
          value += powerBoard1[sList[i].sx][sList[i].sy] / 2.0;
        }
	add_altmoves(sList[i].sx, sList[i].sy, value, i);
	ignoreSafe[sList[i].sx][sList[i].sy] = TRUE;
      }
  return;
}

/* 잡힐 수 있는 돌 살리기 */
/* 현재는 사용하지 않음 */
Getmove(lookForSaveN)
{
  int i, value;
  
    for (i = 1; i <= maxStringID; i++)	/* scan the string list */
      if ( /* (sList[i].libC > 1) &&
	  (sList[i].libC <= treeLibLim) && */
	  (ndbord[sList[i].lx][sList[i].ly] == 1) &&
	  !sList[i].isSafe && !sList[i].isDead) {
	    value = sList[i].size * 2 + (powerBoard1[sList[i].sx][sList[i].sy]) / 2;
	    add_altmoves(sList[i].sx, sList[i].sy, value, i);
	  }
  return;
}

void get_higher(pointList pl, int *x, int *y)
{
  int i, dx, dy, h, mh = -1;
  
  for (i = 1; i <= pl.indx; i++) {
    dx = 9 - abs(pl.p[i].px - 9);
    dy = 9 - abs(pl.p[i].py - 9);
    h = dx > dy ? dy : dx;
    if (h > mh) {
      mh = h;
      *x = pl.p[i].px;
      *y = pl.p[i].py;
    } else if (h == mh && 
    	powerBoard[pl.p[i].px][pl.p[i].py] > powerBoard[*x][*y]) {
      mh = h;
      *x = pl.p[i].px;
      *y = pl.p[i].py;
    }
  }
}

/* 단수 될 수 있는돌 미리 피하기 */
Getmove(lookForSave2)
{
  int i, x1, y1, x2, y2;
  
    for (i = 1; i <= maxStringID; i++)	/* scan the string list */
      if ((sList[i].libC == 2) && 
      		ndbord[sList[i].lx][sList[i].ly] == 1 && sList[i].isSafe) {
        spanString(sList[i].lx, sList[i].ly, &pList);
        x1 = pList.p[1].px;
        y1 = pList.p[1].py;
        x2 = pList.p[2].px;
        y2 = pList.p[2].py;
        if (powerBoard1[x1][y1] > 2 && libBoard[x1][y1] > 1 &&
        	scoreBoard[x1][y1] < 2 && !protPoints[x1][y1])
	  add_altmoves(x1, y1, (powerBoard1[x1][y1] + 1) / 4 +
	  	valueBoard[x1][y1] / 20, i);
        if (powerBoard1[x2][y2] > 2 && libBoard[x2][y2] > 1 &&
        	scoreBoard[x2][y2] < 2 && !protPoints[x2][y2])
	  add_altmoves(x2, y2, (powerBoard1[x2][y2] + 1) / 4 +
	  	valueBoard[x2][y2] / 20, i);
      }
  return;
}


/* 단수 된 돌 잡기 */
Getmove(lookForKill1)
{
  int i, value;

  for (i = 1; i <= maxStringID; i++)	/* scan the string list */
    if ((sList[i].libC == 1) &&
	(ndbord[sList[i].lx][sList[i].ly] == -1)) {	/* we found a live enemy string with one liberty */
      /* find the liberty */
      spanString(sList[i].lx, sList[i].ly, &pList);
      if (legal[pList.p[1].px][pList.p[1].py]) {
        value = sList[i].size * 2;
/*        if (libBoard[pList.p[1].px][pList.p[1].py] ||
        	safeMove(pList.p[1].px, pList.p[1].py)) */
        tryPlay(pList.p[1].px, pList.p[1].py, 1);
        if (sList[SMap[stringIDs[pList.p[1].px][pList.p[1].py]]].libC > 1)
          value += powerBoard[pList.p[1].px][pList.p[1].py] / 2;
        else {
          value--;
          if (value > 1)
            value--;
        }
        restoreState();
        /* 할일: 주위 돌중 단수된돌 크기 더하기 */
        add_altmoves(pList.p[1].px, pList.p[1].py, value, i);
      }
    }
  return;
}

/* 양단수 */
Getmove(doubleAtari)
{				/* doubleAtari */
  int i, j, v;
  for (i = 1; i <= maxStringID - 1; i++)
    if ((sList[i].libC == 2) &&
	(ndbord[sList[i].lx][sList[i].ly] == -1)) {	/* found an atariable string of his */
      spanString(sList[i].lx, sList[i].ly, &dapList1);
      for (j = i + 1; j <= maxStringID; j++)
	if ((sList[j].libC == 2) &&
	    (ndbord[sList[j].lx][sList[j].ly] == -1)) {
	  spanString(sList[j].lx, sList[j].ly, &dapList2);
	  intersectPlist(&dapList1, &dapList2, &dapList3);
	  if (dapList3.indx > 0)
	    if (legal[dapList3.p[1].px][dapList3.p[1].py]) {
	      tryPlay(dapList3.p[1].px, dapList3.p[1].py, 1);
	      if (sList[stringIDs[dapList3.p[1].px][dapList3.p[1].py]].libC > 1) {
		v = min(sList[i].size, sList[j].size) * 2 - 1 + 
			powerBoard[dapList3.p[1].px][dapList3.p[1].py];
		v += scoreBoard[dapList3.p[1].px][dapList3.p[1].py] < 0 ?
			-scoreBoard[dapList3.p[1].px][dapList3.p[1].py] : 0;
		add_altmoves(dapList3.p[1].px, dapList3.p[1].py, v, i);
	      }
	      restoreState();
	    }
	}
    }
  /* 양단수 피하기 */
  for (i = 1; i <= maxStringID - 1; i++)
    if ((sList[i].libC == 2) &&
	(ndbord[sList[i].lx][sList[i].ly] == 1)) {
      spanString(sList[i].lx, sList[i].ly, &dapList1);
      for (j = i + 1; j <= maxStringID; j++)
	if ((sList[j].libC == 2) &&
	    (ndbord[sList[j].lx][sList[j].ly] == 1)) {
	  spanString(sList[j].lx, sList[j].ly, &dapList2);
	  intersectPlist(&dapList1, &dapList2, &dapList3);
	  if (dapList3.indx > 0)
	    if (legal[dapList3.p[1].px][dapList3.p[1].py]) {
	      tryPlay(dapList3.p[1].px, dapList3.p[1].py, -1);
	      if (sList[stringIDs[dapList3.p[1].px][dapList3.p[1].py]].libC > 1
/*	       && safeMove(dapList3.p[1].px, dapList3.p[1].py) */ ) {
		v = (int) floor((min(sList[i].size, sList[j].size) * 2 +
			powerBoard1[dapList3.p[1].px][dapList3.p[1].py] / 2.0) + .5);
		add_altmoves(dapList3.p[1].px, dapList3.p[1].py, v, i);
	      }
	      restoreState();
	    }
	}
    }
  return;
}				/* doubleAtari */

/* 끊어서 둘중 하나 잡기,, 그리고 그에 대비 */
Getmove(cutNkill)
{
  int i, j, gx, gy;
  int me, him;
  float v;
  
  for (i = 1; i <= maxStringID - 1; i++)
    if (sList[i].libC == 2 && !sList[i].isDead) {

      him = bord[sList[i].lx][sList[i].ly];
      me = -him;

      spanString(sList[i].lx, sList[i].ly, &dapList1);
      for (j = 1; j <= maxStringID; j++)
	if ((sList[j].libC == 3) && sList[j].isSafe &&
	    (ndbord[sList[j].lx][sList[j].ly] == him)) {
	  spanString(sList[j].lx, sList[j].ly, &dapList2);
	  intersectPlist(&dapList1, &dapList2, &dapList3);
	  if (dapList3.indx > 0)
	    if (legal[dapList3.p[1].px][dapList3.p[1].py]) {
	      tryPlay(dapList3.p[1].px, dapList3.p[1].py, me);
	      if (sList[stringIDs[dapList3.p[1].px][dapList3.p[1].py]].libC > 1) {

	      if (dapList3.p[1].px == dapList1.p[1].px &&
	      		dapList3.p[1].py == dapList1.p[1].py)
	        tryPlay(dapList1.p[2].px, dapList1.p[2].py, him);
	      else
	        tryPlay(dapList1.p[1].px, dapList1.p[1].py, him);
	      if (killable(sList[j].lx, sList[j].ly, &gx, &gy)) {
		v = min(sList[i].size, sList[j].size) * 2 + 0.5 +
			powerBoard[dapList3.p[1].px][dapList3.p[1].py];
		if (him == -1)
		  v += scoreBoard[dapList3.p[1].px][dapList3.p[1].py] < 0 ?
			-scoreBoard[dapList3.p[1].px][dapList3.p[1].py] : 0;
		add_altmoves(dapList3.p[1].px, dapList3.p[1].py, v, i);
	      }
	      }
	      restoreState();
	    }
	}
    }
}

/* 잡을 수 있는 돌 잡기 */
Getmove(lookForKill)
{				/* lookForAttack */
  int i, j, d, dx, dy;
  float value;

  for (i = 1; i <= maxStringID; i++)	/* scan the string list */
    if ((bord[sList[i].lx][sList[i].ly] == -1) && !sList[i].isSafe) {
      value = 0;
      if (!sList[i].isDead) {
	value = sList[i].size * 2;
	if (sList[i].libC == 1 && hisStones[sList[i].kx][sList[i].ky] == 
		Maxlibs(sList[i].kx,sList[i].ky)) {
	  value--;
	  if (sList[i].size > 1)
	    value--;
	} else {
	  value += libBoard[sList[i].kx][sList[i].ky];
	  value += powerBoard[sList[i].kx][sList[i].ky] / 2.0;
	}
      } else {
	/* 죽은 돌이 죽지 않은 돌과 연결 할 수 있으면 잡는다. */
	spanString(sList[i].lx, sList[i].ly, &pList);
	for (j = 1; j <= pList.indx; j++)
	  for (d = 0; d < 4; d++) {
	    if (get_dxy(d, pList.p[j].px, pList.p[j].py, 1, &dx, &dy) &&
		bord[dx][dy] == -1 &&
		!sList[stringIDs[dx][dy]].isDead)
	      value = (float)sList[i].size / 2;
	  }
      }
      if (value > 0) {
	add_altmoves(sList[i].kx, sList[i].ky, value, i);
	ignoreSafe[sList[i].kx][sList[i].ky] = TRUE;
      }
    }
  return;
}				/* lookForAttack */

  /*
     Plays a move that requires a response on the opponent's part
   */
Getmove(threaten)
{				/* threaten */
  int i, j, gx, gy, tNum;

  initArray(threatBord);
  for (i = 1; i <= maxStringID; i++)
    if ((!sList[i].isLive) &&
	(ndbord[sList[i].lx][sList[i].ly] == -1)) {
      spanString(sList[i].lx, sList[i].ly, &pList);
      for (j = 1; j <= pList.indx; j++)
	if (legal[pList.p[j].px][pList.p[j].py]) {
	  tryPlay(pList.p[j].px, pList.p[j].py, 1);
	  if (sList[stringIDs[pList.p[j].px][pList.p[j].py]].libC > 1)
	    if (killable(sList[i].lx, sList[i].ly, &gx, &gy))
	      threatBord[pList.p[j].px][pList.p[j].py] += 1;
	  restoreState();
	}
    }
  tNum = 0;
  for (i = 0; i <= maxPoint; i++)
    for (j = 0; j <= maxPoint; j++)
      if ((threatBord[i][j] > tNum) &&
	  ((threatBord[i][j] > 1) ||
	      (connectMap[i][j] > 0))) {
	tNum = threatBord[i][j];
	if (tNum > 0)
	  add_altmoves(i, j, 1, 0);
      }

  return;
}				/* threaten */

  /*
     connects against enemy cuts
   */
/* 연결 */
Getmove(connectCut)
{				/* connectCut */
  int i, j, nap, gid, gid2, gID[4];

  for (i = 0; i <= maxPoint; i++)
    for (j = 0; j <= maxPoint; j++)
      if (legal[i][j] &&
	  (protPoints[i][j] == 0)) {	/* not a protected point */
	nap = 0;		/* how many of my stones am I adjacent to? */
	if ((i > 0) && (bord[i - 1][j] == 1)) {
	  gID[nap] = GIDMap[i-1][j];
	  nap = nap + 1;
	  pList.p[nap].px = i - 1;
	  pList.p[nap].py = j;
	}
	if ((j > 0) && (bord[i][j - 1] == 1)) {
	  gID[nap] = GIDMap[i][j-1];
	  nap = nap + 1;
	  pList.p[nap].px = i;
	  pList.p[nap].py = j - 1;
	}
	if ((i < maxPoint) && (bord[i + 1][j] == 1)) {
	  gID[nap] = GIDMap[i+1][j];
	  nap = nap + 1;
	  pList.p[nap].px = i + 1;
	  pList.p[nap].py = j;
	}
	if ((j < maxPoint) && (bord[i][j + 1] == 1)) {
	  gID[nap] = GIDMap[i][j+1];
	  nap = nap + 1;
	  pList.p[nap].px = i;
	  pList.p[nap].py = j + 1;
	}
	if (nap == 1) {		/* possible knight's || 2-point extention */
	  gid = stringIDs[pList.p[1].px][pList.p[1].py];
	  if ((i > 0) && (i < maxPoint) &&
	      (ndbord[i - 1][j] == 1) &&
	      (ndbord[i + 1][j] == 0)) {	/* contact on left */
	    if (((j > 0) && (ndbord[i][j - 1] == -1) &&
		    (ndbord[i + 1][j - 1] == 1) &&
		    (gid != (gid2 = stringIDs[i + 1][j - 1]))) ||
		((j < maxPoint) && (ndbord[i][j + 1] == -1) &&
		    (ndbord[i + 1][j + 1] == 1) &&
		    (gid != (gid2 = stringIDs[i + 1][j + 1]))) ||
		((((j > 0) && (ndbord[i][j - 1] == -1)) ||
			((j < maxPoint) && (ndbord[i][j + 1] == -1))) &&
		    (i < (maxPoint - 1)) &&
		    (ndbord[i + 2][j] == 1) &&
		    (gid != (gid2 = stringIDs[i + 2][j])))) {
	      if (sList[gid].libC > 1 && sList[gid2].libC > 1/* && safeMove(i, j) */)
	        add_altmoves(i, j, 4, gid);
	    }
	  } else if ((i < maxPoint) && (i > 0) &&
		(ndbord[i + 1][j] == 1) &&
	      (ndbord[i - 1][j] == 0)) {	/* r */
	    if (((j > 0) && (ndbord[i][j - 1] == -1) &&
		    (ndbord[i - 1][j - 1] == 1) &&
		    (gid != (gid2 = stringIDs[i - 1][j - 1]))) ||
		((j < maxPoint) && (ndbord[i][j + 1] == -1) &&
		    (ndbord[i - 1][j + 1] == 1) &&
		    (gid != (gid2 = stringIDs[i - 1][j + 1]))) ||
		((((j > 0) && (ndbord[i][j - 1] == -1)) ||
			((j < maxPoint) && (ndbord[i][j + 1] == -1))) &&
		    (i > 1) &&
		    (ndbord[i - 2][j] == 1) &&
		    (gid != (gid2 = stringIDs[i - 2][j])))) {
	      if (sList[gid].libC > 1 && sList[gid2].libC > 1/* && safeMove(i, j) */)
	        add_altmoves(i, j, 4, gid);
	    }
	  } else if ((j > 0) && (j < maxPoint) &&
		(ndbord[i][j - 1] == 1) &&
	      (ndbord[i][j + 1] == 0)) {	/* top */
	    if (((i > 0) && (ndbord[i - 1][j] == -1) &&
		    (ndbord[i - 1][j + 1] == 1) &&
		    (gid != (gid2 = stringIDs[i - 1][j + 1]))) ||
		((i < maxPoint) && (ndbord[i + 1][j] == -1) &&
		    (ndbord[i + 1][j + 1] == 1) &&
		    (gid != (gid2 = stringIDs[i + 1][j + 1]))) ||
		((((i > 0) && (ndbord[i - 1][j] == -1)) ||
			((i < maxPoint) && (ndbord[i + 1][j] == -1))) &&
		    (j < (maxPoint - 1)) &&
		    (ndbord[i][j + 2] == 1) &&
		    (gid != (gid2 = stringIDs[i][j + 2])))) {
	      if (sList[gid].libC > 1 && sList[gid2].libC > 1/* && safeMove(i, j) */)
	        add_altmoves(i, j, 4, gid);
	    }
	  } else if ((j > 0) && (j < maxPoint) &&
		(ndbord[i][j + 1] == 1) &&
	      (ndbord[i][j - 1] == 0)) {	/* bottom */
	    if (((i > 0) && (ndbord[i - 1][j] == -1) &&
		    (ndbord[i - 1][j - 1] == 1) &&
		    (gid != (gid2 = stringIDs[i - 1][j - 1]))) ||
		((i < maxPoint) && (ndbord[i + 1][j] == -1) &&
		    (ndbord[i + 1][j - 1] == 1) &&
		    (gid != (gid2 = stringIDs[i + 1][j - 1]))) ||
		((((i > 0) && (ndbord[i - 1][j] == -1)) ||
			((i < maxPoint) && (ndbord[i + 1][j] == -1))) &&
		    (j > 1) &&
		    (ndbord[i][j - 2] == 1) &&
		    (gid != (gid2 = stringIDs[i][j - 2])))) {
	      if (sList[gid].libC > 1 && sList[gid2].libC > 1/* && safeMove(i, j) */)
	        add_altmoves(i, j, 4, gid);
	    }
	  }
	} else if (nap == 2) {	/* diagonal or 1-point extention */
	  if (stringIDs[pList.p[1].px][pList.p[1].py] !=
	      stringIDs[pList.p[2].px][pList.p[2].py]) {
	    if ((pList.p[1].px != pList.p[2].px) &&
		(pList.p[1].py != pList.p[2].py)) {	/* diag */
	      spanString(pList.p[1].px,
		  pList.p[1].py, &pList1);
	      spanString(pList.p[2].px,
		  pList.p[2].py, &plist2);
	      intersectPlist(&pList1, &plist2, &plist3);
	      if (plist3.indx == 1) {
	        pList.p[3].px = pList.p[1].px != i ?
	        	pList.p[1].px : pList.p[2].px;
	        pList.p[3].py = pList.p[1].py != j ?
	        	pList.p[1].py : pList.p[2].py;
	        if (ndbord[pList.p[3].px][pList.p[3].py] == -1) {
	        /* 마늘모 연결 절단 대비 */
	        /* 할일: 반대쪽 돌을 잡을 수 있으면 반대쪽 돌을 잡는다. */
		if (((i > 0) && (ndbord[i - 1][j] == -1)) ||
		    ((i < maxPoint) && (ndbord[i + 1][j] == -1)) ||
		    ((j > 0) && (ndbord[i][j - 1] == -1)) ||
		    ((j < maxPoint) && (ndbord[i][j + 1] == -1))) 	/* must make direct connection */
		  if (heCanCut(i, j))
/*		    if (safeMove(i, j)) */
		      add_altmoves(i, j, 5, stringIDs[pList.p[1].px][pList.p[1].py]);
		}
#if 0
		 else if (heCanCut(i, j)) {	/* protect point if possible */
		  infl = 1000;
		  if ((i > 0) && legal[i - 1][j] &&
		      ((i == 1) || (ndbord[i - 2][j] == 0)) &&
		      ((j == 0) || (ndbord[i - 1][j - 1] == 0)) &&
		      ((j == maxPoint) ||
			  (ndbord[i - 1][j + 1] == 0)))
/*		    if (safeMove(i - 1, j)) */
		      if (claim[i - 1][j] < infl) {
			x = i - 1;
			y = j;
			infl = claim[i - 1][j];
		      }
		  if ((j > 0) && legal[i][j - 1] &&
		      ((j == 1) || (ndbord[i][j - 2] == 0)) &&
		      ((i == 0) || (ndbord[i - 1][j - 1] == 0)) &&
		      ((i == maxPoint) ||
			  (ndbord[i + 1][j - 1] == 0)))
/*		    if (safeMove(i, j - 1)) */
		      if (claim[i][j - 1] < infl) {
			x = i;
			y = j - 1;
			infl = claim[i][j - 1];
		      }
		  if ((i < maxPoint) && legal[i + 1][j] &&
		      ((i == (maxPoint - 1)) ||
			  (ndbord[i + 2][j] == 0)) &&
		      ((j == 0) || (ndbord[i + 1][j - 1] == 0)) &&
		      ((j == maxPoint) ||
			  (ndbord[i + 1][j + 1] == 0)))
/*		    if (safeMove(i + 1, j)) */
		      if (claim[i + 1][j] < infl) {
			x = i + 1;
			y = j;
			infl = claim[i + 1][j];
		      }
		  if ((j < maxPoint) && legal[i][j + 1] &&
		      ((j == (maxPoint - 1)) ||
			  (ndbord[i][j + 2] == 0)) &&
		      ((i == 0) || (ndbord[i - 1][j + 1] == 0)) &&
		      ((i == maxPoint) ||
			  (ndbord[i + 1][j + 1] == 0)))
/*		    if (safeMove(i, j + 1)) */
		      if (claim[i][j + 1] < infl) {
			x = i;
			y = j + 1;
			infl = claim[i][j + 1];
		      }
		  if (infl < 1000)
		    add_altmoves(i, j, 3, infl);
		  /* direct connection */
		  else /*if (safeMove(i, j)) */
		    add_altmoves(i, j, 2, infl);
		}
#endif
	      }
	    } else {		/* 1-point extension, only protect if threatened */
	      if (((i > 0) && (ndbord[i - 1][j] == -1)) ||
		  ((j > 0) && (ndbord[i][j - 1] == -1)) ||
		  ((i < maxPoint) && (ndbord[i + 1][j] == -1)) ||
		  ((j < maxPoint) && (ndbord[i][j + 1] == -1))) {
		if (heCanCut(i, j)) {
/*		  if (safeMove(i, j)) */
		  if (gID[0] != gID[1])
		    add_altmoves(i, j, 6, 0);
		  else
		    add_altmoves(i, j, 1.8, 0);
		}
	      }
	    }
	  }
	} else if (nap == 3) {	/* unprotected, but me on 3 sides */
	  if ((stringIDs[pList.p[1].px][pList.p[1].py] !=
		  stringIDs[pList.p[2].px][pList.p[2].py]) ||
	      (stringIDs[pList.p[1].px][pList.p[1].py] !=
		  stringIDs[pList.p[3].px][pList.p[3].py]) ||
	      (stringIDs[pList.p[3].px][pList.p[3].py] !=
		  stringIDs[pList.p[2].px][pList.p[2].py])) {
	    spanString(pList.p[1].px, pList.p[1].py, &pList1);
	    spanString(pList.p[2].px, pList.p[2].py, &plist2);
	    intersectPlist(&pList1, &plist2, &plist3);
	    spanString(pList.p[3].px, pList.p[3].py, &plist2);
	    intersectPlist(&plist2, &plist3, &pList1);
	    if (pList1.indx == 1)	/* a common connect point */
	      if (heCanCut(i, j)) {
		if ((bord[i+1][j] == -1 &&
		    (safeboard[i-1][j-1] == -1 || safeboard[i-1][j+1] == -1)) ||
		    (safeboard[i-1][j] == -1 &&                        
		    (safeboard[i+1][j-1] == -1 || safeboard[i+1][j+1] == -1)) ||
		    (safeboard[i][j+1] == -1 &&                        
		    (safeboard[i-1][j-1] == -1 || safeboard[i+1][j-1] == -1)) ||
		    (safeboard[i][j-1] == -1 &&                        
		    (safeboard[i-1][j+1] == -1 || safeboard[i+1][j+1] == -1)))
		  add_altmoves(i, j, 5, 0);
		else
		  add_altmoves(i, j, 2, 0);
	      }
	  }
	}
      }
  return;
}				/* connectCut */

int heCanCut(int x, int y)
{				/* heCanCut */
  int gx, gy, result;
  if (playLevel > 3) {
    tryPlay(x, y, -1);		/* try his cut */
    result = !killable(x, y, &gx, &gy);
    restoreState();
    return result;
  } else
    return FALSE;
}				/* heCanCut */

  /*
     Checks out a move.
     If my stone is not killable then true.
   */
int safeMove(int x, int y)
{				/* safeMove */
  int gbx, gby, result;
#if 0
  if (killFlag)			/* I shouldn't kill if lookForKill didn't */
    result = FALSE;
  else if (sList[stringIDs[x][y]].libC < 2) {	/* if it is in atari or dead */
    result = FALSE;		/* reject it */
  } else 
#endif  
  if (ignoreSafe[x][y])
    return TRUE;
  if (!legal[x][y])
    return FALSE;
  tryPlay(x, y, 1);		/* try playing at point */
  if (sList[stringIDs[x][y]].libC <= treeLibLim)	/* see if killable */
    if (playLevel > 0)
      result = !killable(x, y, &gbx, &gby);
    else
      result = TRUE;
  else
    result = TRUE;
  restoreState();
  return result;
}				/* safeMove */


Getmove(extendWall)
{				/* extendWall */
  int infl, i, j, x, y;

  x = iNil;
  y = iNil;
  infl = 0;
  for (i = 2; i <= maxPoint - 2; i++)
    for (j = 2; j <= maxPoint - 2; j++)
      if (legal[i][j])
	if (safeConnect[i][j] > 0)
	  if ((powerBoard[i][j] > infl) &&
	      (ndbord[i - 1][j] == 0) &&
	      (ndbord[i + 1][j] == 0) &&
	      (ndbord[i][j - 1] == 0) &&
	      (ndbord[i][j + 1] == 0) &&
	      ((claim[i - 1][j] < 0) ||
		  (claim[i + 1][j] < 0) ||
		  (claim[i][j - 1] < 0) ||
		  (claim[i][j + 1] < 0)))
	      infl = powerBoard[i][j];

  for (i = 2; i <= maxPoint - 2; i++)
    for (j = 2; j <= maxPoint - 2; j++)
      if (legal[i][j])
	if (safeConnect[i][j] > 0)
	  if ((powerBoard[i][j] >= infl) &&
	      (ndbord[i - 1][j] == 0) &&
	      (ndbord[i + 1][j] == 0) &&
	      (ndbord[i][j - 1] == 0) &&
	      (ndbord[i][j + 1] == 0) &&
	      ((claim[i - 1][j] < 0) ||
		  (claim[i + 1][j] < 0) ||
		  (claim[i][j - 1] < 0) ||
		  (claim[i][j + 1] < 0)))
/*	    if (safeMove(i, j))*/ {
	      infl = powerBoard[i][j];
	      if (infl > 5)
	        add_altmoves(i, j, 2 + (float) infl / 10, infl);
	      else
	        add_altmoves(i, j, (float) infl / 5, infl);
	    }
}				/* extendWall */

Getmove(extendWall2)
{				/* extendWall */
  int infl, i, j, x, y;

  x = iNil;
  y = iNil;
  infl = 5;
  for (i = 2; i <= maxPoint - 2; i++)
    for (j = 2; j <= maxPoint - 2; j++)
      if (legal[i][j])
	if (safeConnect[i][j] > 0)
	  if ((powerBoard2[i][j] > infl) &&
	      (ndbord[i - 1][j] == 0) &&
	      (ndbord[i + 1][j] == 0) &&
	      (ndbord[i][j - 1] == 0) &&
	      (ndbord[i][j + 1] == 0) &&
	      ((claim[i - 1][j] < 0) ||
		  (claim[i + 1][j] < 0) ||
		  (claim[i][j - 1] < 0) ||
		  (claim[i][j + 1] < 0)))
	      infl = powerBoard2[i][j];

  for (i = 2; i <= maxPoint - 2; i++)
    for (j = 2; j <= maxPoint - 2; j++)
      if (legal[i][j])
	if (safeConnect[i][j] > 0)
	  if ((powerBoard2[i][j] >= infl) &&
	      (ndbord[i - 1][j] == 0) &&
	      (ndbord[i + 1][j] == 0) &&
	      (ndbord[i][j - 1] == 0) &&
	      (ndbord[i][j + 1] == 0) &&
	      ((claim[i - 1][j] < 0) ||
		  (claim[i + 1][j] < 0) ||
		  (claim[i][j - 1] < 0) ||
		  (claim[i][j + 1] < 0)))
/*	    if (safeMove(i, j))*/ {
	      infl = powerBoard2[i][j];
	      add_altmoves(i, j, 2 + (float) infl / 10, infl);
	    }
}				/* extendWall */

  /*
     check to see if I can attack one of his strings
     uses limited depth search so that it can work on larger lib counts
   */
Getmove(findAttack2)
{				/* findAttack2 */
  int tx, ty, i, otll, odl;
  if (playLevel < 7)
    return;

  odl = depthLimit;
  otll = treeLibLim;
  treeLibLim += 3;
  depthLimit = treeLibLim + 3;
  for (i = 1; i <= maxStringID; i++)	/* scan the string list */
    if ((!sList[i].isLive) &&
	(ndbord[sList[i].lx][sList[i].ly] == -1) &&
	(sList[i].libC > 1) && sList[i].isSafe) {
      if (killable(sList[i].lx, sList[i].ly, &tx, &ty)) {	/* can we kill it? */
	  add_altmoves(tx, ty, 5, i);
      }
    }
  treeLibLim = otll;
  depthLimit = odl;
  return;
}				/* findAttack2 */


  /*
     blocks enemy cuts thru 1-point extensions
   */
/* 막기 */
Getmove(blockCut)
{				/* blockCut */
  int i, j, v;

  for (i = 0; i <= maxPoint; i++)
    for (j = 0; j <= maxPoint; j++)
      if (legal[i][j]) {
	if ((i > 0) && (j > 0) && (j < maxPoint)) {
	  if ((ndbord[i - 1][j] == -1) &&
	      (ndbord[i - 1][j - 1] == 1) &&
	      (ndbord[i - 1][j + 1] == 1) &&
	      (stringIDs[i - 1][j - 1] != stringIDs[i - 1][j + 1]) &&
	      sList[stringIDs[i - 1][j - 1]].libC > 1 &&
	      sList[stringIDs[i - 1][j + 1]].libC > 1) {
	    if (sList[stringIDs[i][j]].isSafe && heCanCut(i, j)) {
	        v = 4 - myStones[i][j] + hisStones[i][j];
	        if (claim[i][j] < 0)
	          v -= 1;
	        else if (claim[i][j] > 0)
	          v += 2;
	        add_altmoves(i, j, v, stringIDs[i - 1][j - 1]);
	      }
	  }
	}
	if ((i < maxPoint) && (j > 0) && (j < maxPoint)) {
	  if ((ndbord[i + 1][j] == -1) &&
	      (ndbord[i + 1][j - 1] == 1) &&
	      (ndbord[i + 1][j + 1] == 1) &&
	      (stringIDs[i + 1][j - 1] != stringIDs[i + 1][j + 1]) &&
	      sList[stringIDs[i + 1][j - 1]].libC > 1 &&
	      sList[stringIDs[i + 1][j + 1]].libC > 1) {
	    if (sList[stringIDs[i][j]].isSafe && heCanCut(i, j)) {
	        v = 4 - myStones[i][j] + hisStones[i][j];
	        if (claim[i][j] < 0)
	          v -= 1;
	        else if (claim[i][j] > 0)
	          v += 2;
	        add_altmoves(i, j, v, stringIDs[i + 1][j - 1]);
	      }
	  }
	}
	if ((j > 0) && (i > 0) && (i < maxPoint)) {
	  if ((ndbord[i][j - 1] == -1) &&
	      (ndbord[i - 1][j - 1] == 1) &&
	      (ndbord[i + 1][j - 1] == 1) &&
	      (stringIDs[i - 1][j - 1] != stringIDs[i + 1][j - 1]) &&
	      sList[stringIDs[i - 1][j - 1]].libC > 1 &&
	      sList[stringIDs[i + 1][j - 1]].libC > 1) {
	    if (sList[stringIDs[i][j]].isSafe && heCanCut(i, j)) {
	        v = 4 - myStones[i][j] + hisStones[i][j];
	        if (claim[i][j] < 0)
	          v -= 1;
	        else if (claim[i][j] > 0)
	          v += 2;
	        add_altmoves(i, j, v, stringIDs[i - 1][j - 1]);
	      }
	  }
	}
	if ((j < maxPoint) && (i > 0) && (i < maxPoint)) {
	  if ((ndbord[i][j + 1] == -1) &&
	      (ndbord[i - 1][j + 1] == 1) &&
	      (ndbord[i + 1][j + 1] == 1) &&
	      (stringIDs[i - 1][j + 1] != stringIDs[i + 1][j + 1]) &&
	      sList[stringIDs[i - 1][j + 1]].libC > 1 &&
	      sList[stringIDs[i + 1][j + 1]].libC > 1) {
	    if (sList[stringIDs[i][j]].isSafe && heCanCut(i, j)) {
	        v = 4 - myStones[i][j] + hisStones[i][j];
	        if (claim[i][j] < 0)
	          v -= 1;
	        else if (claim[i][j] > 0)
	          v += 2;
	        add_altmoves(i, j, v, stringIDs[i - 1][j + 1]);
	      }
	  }
	}
      }
  return;
}				/* blockCut */


  /*
     cuts the enemy
   */
Getmove(cutHim)
{				/* cutHim */
  int i, j, nap, gid, gID[4], sID[4];
  float v;

  for (i = 0; i <= maxPoint; i++)
    for (j = 0; j <= maxPoint; j++)
      if (legal[i][j]) {
	nap = 0;		/* how many of his stones am I adjacent to? */
	if ((i > 0) && (ndbord[i - 1][j] == -1)) {
	  gID[nap] = GIDMap[i-1][j];
	  sID[nap] = stringIDs[i-1][j];
	  nap = nap + 1;
	  pList.p[nap].px = i - 1;
	  pList.p[nap].py = j;
	}
	if ((j > 0) && (ndbord[i][j - 1] == -1)) {
	  gID[nap] = GIDMap[i][j-1];
	  sID[nap] = stringIDs[i][j-1];
	  nap = nap + 1;
	  pList.p[nap].px = i;
	  pList.p[nap].py = j - 1;
	}
	if ((i < maxPoint) && (ndbord[i + 1][j] == -1)) {
	  gID[nap] = GIDMap[i+1][j];
	  sID[nap] = stringIDs[i+1][j];
	  nap = nap + 1;
	  pList.p[nap].px = i + 1;
	  pList.p[nap].py = j;
	}
	if ((j < maxPoint) && (ndbord[i][j + 1] == -1)) {
	  gID[nap] = GIDMap[i][j+1];
	  sID[nap] = stringIDs[i][j+1];
	  nap = nap + 1;
	  pList.p[nap].px = i;
	  pList.p[nap].py = j + 1;
	}
	if (nap == 1) {		/* possible knight's or 2-point extention */
	  gid = stringIDs[pList.p[1].px][pList.p[1].py];
	  if ((i > 0) && (i < maxPoint) &&
	      (ndbord[i - 1][j] == -1) &&
	      (connectMap[i][j] > 0)) {		/* contact on left */
	    if ((j > 0) &&
		    (ndbord[i + 1][j - 1] == -1) &&
		    (gid != stringIDs[i + 1][j - 1])) {
	      if (sList[stringIDs[i + 1][j - 1]].isSafe) {
/*		if (safeMove(i, j)) */
		  add_altmoves(i, j, 2, gid);
	      } else {
	        add_altpower(ACT_KILL, stringIDs[i + 1][j - 1], 1);
	      }
	    }
	    if ((j < maxPoint) &&
		    (ndbord[i + 1][j + 1] == -1) &&
		    (gid != stringIDs[i + 1][j + 1])) {
	      if (sList[stringIDs[i + 1][j + 1]].isSafe) {
/*		if (safeMove(i, j)) */
		  add_altmoves(i, j, 2, gid);
	      } else {
	        add_altpower(ACT_KILL, stringIDs[i + 1][j + 1], 1);
	      }
	    }
	    if ((i < (maxPoint - 1)) &&
		    (ndbord[i + 1][j] == 0) &&
		    (ndbord[i + 2][j] == -1) &&
		    (gid != stringIDs[i + 2][j])) {
	      if (sList[stringIDs[i + 2][j]].isSafe) {
/*		if (safeMove(i, j)) */
		  add_altmoves(i, j, 2, gid);
	      } else {
	        add_altpower(ACT_KILL, stringIDs[i + 2][j], 1);
	      }
	    }
	  } else if ((i < maxPoint) && (i > 0) &&
		(ndbord[i + 1][j] == -1) &&
	      (connectMap[i][j] > 0)) {		/* r */
	    if ((j > 0) &&
		    (ndbord[i - 1][j - 1] == -1) &&
		    (gid != stringIDs[i - 1][j - 1])) {
	      if (sList[stringIDs[i - 1][j - 1]].isSafe) {
/*		if (safeMove(i, j)) */
		  add_altmoves(i, j, 2, gid);
	      } else {
	        add_altpower(ACT_KILL, stringIDs[i - 1][j - 1], 1);
	      }
	    }
	    if ((j < maxPoint) &&
		    (ndbord[i - 1][j + 1] == -1) &&
		    (gid != stringIDs[i - 1][j + 1])) {
	      if (sList[stringIDs[i - 1][j + 1]].isSafe) {
/*		if (safeMove(i, j)) */
		  add_altmoves(i, j, 2, gid);
	      } else {
	        add_altpower(ACT_KILL, stringIDs[i - 1][j + 1], 1);
	      }
	    }
	    if ((i > 1) &&
		    (ndbord[i - 1][j] == 0) &&
		    (ndbord[i - 2][j] == -1) &&
		    (gid != stringIDs[i - 2][j])) {
	      if (sList[stringIDs[i - 2][j]].isSafe) {
/*		if (safeMove(i, j)) */
		  add_altmoves(i, j, 2, gid);
	      } else {
	        add_altpower(ACT_KILL, stringIDs[i - 2][j], 1);
	      }
	    }
	  } else if ((j > 0) && (j < maxPoint) &&
		(ndbord[i][j - 1] == -1) &&
	      (connectMap[i][j] > 0)) {		/* top */
	    if ((i > 0) &&
		    (ndbord[i - 1][j + 1] == -1) &&
		    (gid != stringIDs[i - 1][j + 1])) {
	      if (sList[stringIDs[i - 1][j + 1]].isSafe) {
/*		if (safeMove(i, j)) */
		  add_altmoves(i, j, 2, gid);
	      } else {
	        add_altpower(ACT_KILL, stringIDs[i - 1][j + 1], 1);
	      }
	    }
	    if ((i < maxPoint) &&
		    (ndbord[i + 1][j + 1] == -1) &&
		    (gid != stringIDs[i + 1][j + 1])) {
	      if (sList[stringIDs[i + 1][j + 1]].isSafe) {
/*		if (safeMove(i, j)) */
		  add_altmoves(i, j, 2, gid);
	      } else {
	        add_altpower(ACT_KILL, stringIDs[i + 1][j + 1], 1);
	      }
	    }
	    if ((j < (maxPoint - 1)) &&
		    (ndbord[i][j + 1] == 0) &&
		    (ndbord[i][j + 2] == -1) &&
		    (gid != stringIDs[i][j + 2])) {
	      if (sList[stringIDs[i][j + 2]].isSafe) {
/*		if (safeMove(i, j)) */
		  add_altmoves(i, j, 2, gid);
	      } else {
	        add_altpower(ACT_KILL, stringIDs[i][j + 2], 1);
	      }
	    }
	  } else if ((j > 0) && (j < maxPoint) &&
		(ndbord[i][j + 1] == -1) &&
	      (connectMap[i][j] > 0)) {		/* bottom */
	    if ((i > 0) &&
		    (ndbord[i - 1][j - 1] == -1) &&
		    (gid != stringIDs[i - 1][j - 1])) {
	      if (sList[stringIDs[i - 1][j - 1]].isSafe) {
/*		if (safeMove(i, j)) */
		  add_altmoves(i, j, 2, gid);
	      } else {
	        add_altpower(ACT_KILL, stringIDs[i - 1][j - 1], 1);
	      }
	    }
	    if ((i < maxPoint) &&
		    (ndbord[i + 1][j - 1] == -1) &&
		    (gid != stringIDs[i + 1][j - 1])) {
	      if (sList[stringIDs[i + 1][j - 1]].isSafe) {
/*		if (safeMove(i, j)) */
		  add_altmoves(i, j, 2, gid);
	      } else {
	        add_altpower(ACT_KILL, stringIDs[i + 1][j - 1], 1);
	      }
	    }
	    if ((j > 1) &&
		    (ndbord[i][j - 1] == 0) &&
		    (ndbord[i][j - 2] == -1) &&
		    (gid != stringIDs[i][j - 2])) {
	      if (sList[stringIDs[i][j - 2]].isSafe) {
/*		if (safeMove(i, j)) */
		  add_altmoves(i, j, 2, gid);
	      } else {
	        add_altpower(ACT_KILL, stringIDs[i][j - 2], 1);
	      }
	    }
	  }
	} else if (nap == 2) {	/* diagonal or 1-point extention */
	  if (stringIDs[pList.p[1].px][pList.p[1].py] !=
	      stringIDs[pList.p[2].px][pList.p[2].py]) {
	    if ((pList.p[1].px != pList.p[2].px) &&
		(pList.p[1].py != pList.p[2].py)) {	/* diag */
	      /* 반대쪽 돌이 안전할때만 끊는다 : 예술인 (98. 8. 17) */
	      int dx, dy;
	      if (stringIDs[pList.p[1].px][pList.p[2].py]) {
	        dx = pList.p[1].px;
	        dy = pList.p[2].py;
	      } else {
	        dx = pList.p[2].px;
	        dy = pList.p[1].py;
	      }
	      if (sList[stringIDs[dx][dy]].isSafe) {
		spanString(pList.p[1].px, pList.p[1].py, &pList1);
		spanString(pList.p[2].px, pList.p[2].py, &plist2);
		intersectPlist(&pList1, &plist2, &plist3);
		v = 4;
		if (GET_LIBC(pList.p[1].px, pList.p[1].py) == 2)
		  v++;
		if (GET_LIBC(pList.p[2].px, pList.p[2].py) == 2)
		  v++;
		if (plist3.indx == 1) {
		    add_altmoves(i, j, v, stringIDs[dx][dy]);
	        }
	      }
	    } else {		/* 1-point extension, only cut if connected */
	      if (connectMap[i][j] > 0) {
/*		if (safeMove(i, j)) { */
		  if ((i > 0 && ndbord[i-1][j] == 1) ||
	            (j > 0 && ndbord[i][j-1] == 1) ||
	            (i < maxPoint && ndbord[i+1][j] == 1) ||
	            (j < maxPoint && ndbord[i][j+1] == 1)) {
		    if (GIDMap[pList.p[1].px][pList.p[1].py] !=
			GIDMap[pList.p[2].px][pList.p[2].py])
		      v = 6;
		    else 
		      v = 1.9;
	          } else {
	              v = 2;
	          }
	          if (sList[sID[0]].libC == 2)
	            v += 2;
	          if (sList[sID[1]].libC == 2)
	            v += 2;
	            //printf("... %d,%d %d,%d\n", gID[0], sList[gID[0]].libC ,
	            //	gID[1], sList[gID[1]].libC );
	          add_altmoves(i, j, v, 0);
/*	        } */
	      }
	    }
	  }
	} else if (nap == 3) {	/* unprotected, but him on 3 sides */
	  if ((stringIDs[pList.p[1].px][pList.p[1].py] !=
		  stringIDs[pList.p[2].px][pList.p[2].py]) ||
	      (stringIDs[pList.p[1].px][pList.p[1].py] !=
		  stringIDs[pList.p[3].px][pList.p[3].py]) ||
	      (stringIDs[pList.p[3].px][pList.p[3].py] !=
		  stringIDs[pList.p[2].px][pList.p[2].py])) {
	    spanString(pList.p[1].px, pList.p[1].py, &pList1);
	    spanString(pList.p[2].px, pList.p[2].py, &plist2);
	    intersectPlist(&pList1, &plist2, &plist3);
	    spanString(pList.p[3].px, pList.p[3].py, &plist2);
	    intersectPlist(&plist2, &plist3, &pList1);
	    if (pList1.indx == 1) {	/* a common connect point */
/*	      if (safeMove(i, j)) */
	      if (gID[0] == gID[1] && gID[1] == gID[2])
		add_altmoves(i, j, 1.3, 0);
	      else
		add_altmoves(i, j, 5.5, 0);
	    }
	  }
	}
      }
  return;
}				/* cutHim */


  /*
     ataris a string just for the hell of it
   */
Getmove(atariAnyway)
{				/* atariAnyway */
  int i;

  if (altmoves[0].px)
    return;
  for (i = 1; i <= maxStringID; i++)	/* scan the string list */
    if ((sList[i].libC == 2) &&
	(ndbord[sList[i].lx][sList[i].ly] == -1)) {
      spanString(sList[i].lx, sList[i].ly, &pList);
      if (legal[pList.p[1].px][pList.p[1].py] &&
	  ((connectMap[pList.p[1].px][pList.p[1].py] > 0) ||
	      ((pList.p[1].px > 0) &&
		  (connectMap[pList.p[1].px - 1][pList.p[1].py] > 0)) ||
	      ((pList.p[1].px < maxPoint) &&
		  (connectMap[pList.p[1].px + 1][pList.p[1].py] > 0)) ||
	      ((pList.p[1].py > 0) &&
		  (connectMap[pList.p[1].px][pList.p[1].py - 1] > 0)) ||
	      ((pList.p[1].py < maxPoint) &&
		  (connectMap[pList.p[1].px][pList.p[1].py + 1] > 0))))
/*	if (safeMove(pList.p[1].px, pList.p[1].py)) */{
	  add_altmoves(pList.p[1].px, pList.p[1].py, 1, i);
	}
      if (legal[pList.p[2].px][pList.p[2].py] &&
	  ((connectMap[pList.p[2].px][pList.p[2].py] > 0) ||
	      ((pList.p[2].px > 0) &&
		  (connectMap[pList.p[2].px - 1][pList.p[2].py] > 0)) ||
	      ((pList.p[2].px < maxPoint) &&
		  (connectMap[pList.p[2].px + 1][pList.p[2].py] > 0)) ||
	      ((pList.p[2].py > 0) &&
		  (connectMap[pList.p[2].px][pList.p[2].py - 1] > 0)) ||
	      ((pList.p[2].py < maxPoint) &&
		  (connectMap[pList.p[2].px][pList.p[2].py + 1] > 0))))
/*	if (safeMove(pList.p[2].px, pList.p[2].py)) */{
	  add_altmoves(pList.p[1].px, pList.p[1].py, 1, i);
	}
    }
  return;
}				/* atariAnyway */


  /*
     undercuts his strings
   */
/* 1선 끝내기 */
Getmove(underCut)
{				/* underCut */
  int i;
  float evalue = 1, v;

  for (i = 1; i <= maxPoint - 1; i++) {
    if (legal[0][i]) {
      if (ndbord[1][i] == -1 || (scoreBoard[0][i] < 0 && (safeConnect[0][i] || safeConnect[0][i+1] || safeConnect[0][i-1])))
        if (safeConnect[0][i+1] && find_altmoves(BB_EDGE, 1, i+1))
          add_altpower2(BB_EDGE, 1, i+1, evalue);
        else if (safeConnect[0][i-1] && find_altmoves(BB_EDGE, 1, i-1))
          add_altpower2(BB_EDGE, 1, i-1, evalue);
        else /* if (safeMove(0, i)) */{
	  if (scoreBoard[0][i] < 0 && (safeConnect[0][i] || (safeConnect[0][i+1] && bord[1][i+1] > -1) 
	  	|| (safeConnect[0][i-1] && bord[1][i-1] > -1)))
	    add_altmoves(0, i, 2 + powerBoard[0][i] / 2, 0);
	  else if (ndbord[1][i] == -1) {
	    v = sList[stringIDs[1][i]].libC == 2 ? 4 : 2;
	    add_altmoves(0, i, v, 0);
	  }
	}
    }
    if (legal[maxPoint][i]) {
      if (ndbord[maxPoint - 1][i] == -1 || 
	(scoreBoard[maxPoint][i] < 0 &&
	(safeConnect[maxPoint][i] || safeConnect[maxPoint][i-1] || safeConnect[maxPoint][i+1])))
	if (safeConnect[maxPoint][i-1] && find_altmoves(BB_EDGE, maxPoint -1, i - 1))
	  add_altpower2(BB_EDGE, maxPoint -1, i - 1, evalue);
	else if (safeConnect[maxPoint][i+1] && find_altmoves(BB_EDGE, maxPoint -1, i + 1))
	  add_altpower2(BB_EDGE, maxPoint -1, i + 1, evalue);
	else /*if (safeMove(maxPoint, i)) */{
	if (scoreBoard[maxPoint][i] < 0 &&
	(safeConnect[maxPoint][i] || (safeConnect[maxPoint][i-1] && bord[maxPoint-1][i-1] > -1) 
		|| (safeConnect[maxPoint][i+1] && bord[maxPoint-1][i+1] > -1)))
	  add_altmoves(maxPoint, i, 2 + powerBoard[maxPoint][i] / 2, 0);
	  else if (ndbord[maxPoint - 1][i] == -1) {
	    v = sList[stringIDs[maxPoint - 1][i]].libC == 2 ? 4 : 2;
	    add_altmoves(maxPoint, i, v, 0);
	  }
	}
    }
    if (legal[i][0]) {
      if (ndbord[i][1] == -1 || (scoreBoard[i][0] < 0 &&
		(safeConnect[i][0] || safeConnect[i+1][0] || safeConnect[i-1][0])))
	if (safeConnect[i+1][0] && find_altmoves(BB_EDGE, i + 1, 1))
	  add_altpower2(BB_EDGE, i + 1, 1, evalue);
	else if (safeConnect[i-1][0] && find_altmoves(BB_EDGE, i - 1, 1))
	  add_altpower2(BB_EDGE, i - 1, 1, evalue);
	else /*if (safeMove(i, 0)) */{
	if (scoreBoard[i][0] < 0 &&
		(safeConnect[i][0] || (safeConnect[i+1][0] && bord[i+1][1] > -1) 
			|| (safeConnect[i-1][0] && bord[i-1][1] > -1)))
	  add_altmoves(i, 0, 2 + powerBoard[i][0] / 2, 0);
	  else if (ndbord[i][1] == -1) {
	    v = sList[stringIDs[i][1]].libC == 2 ? 4 : 2;
	    add_altmoves(i, 0, v, 0);
	  }
	}
    }
    if (legal[i][maxPoint]) {
      if (ndbord[i][maxPoint - 1] == -1 || (scoreBoard[i][maxPoint] < 0 &&
		(safeConnect[i][maxPoint] ||safeConnect[i+1][maxPoint] || safeConnect[i-1][maxPoint])))
	if (safeConnect[i+1][maxPoint] && find_altmoves(BB_EDGE, i+1, maxPoint -1))
	  add_altpower2(BB_EDGE, i+1, maxPoint -1, evalue);
	else if (safeConnect[i-1][maxPoint] && find_altmoves(BB_EDGE, i-1, maxPoint -1))
	  add_altpower2(BB_EDGE, i-1, maxPoint -1, evalue);
	else /*if (safeMove(i, maxPoint)) */{
	if (scoreBoard[i][maxPoint] < 0 &&
		(safeConnect[i][maxPoint] || (safeConnect[i+1][maxPoint] && bord[i+1][maxPoint - 1] > -1) 
			|| (safeConnect[i-1][maxPoint] && bord[i-1][maxPoint - 1] > -1)))
	  add_altmoves(i, maxPoint, 2 + powerBoard[i][maxPoint] / 2, 0);
	  else if (ndbord[i][maxPoint - 1] == -1) {
	    v = sList[stringIDs[i][maxPoint - 1]].libC == 2 ? 4 : 2;
	    add_altmoves(i, maxPoint, 2, 0);
	  }
	}
    }
  }
  return;
}				/* underCut */


/* 1선 끝내기 막기 */
Getmove(BlockUnderCut)
{
  int i, v1 = 2, v2 = 5;
  
  for (i = 1; i <= maxPoint - 1; i++) {
    if (ndbord[0][i] == - 1) {
      if (sList[stringIDs[0][i]].libC == 2) {
        if (legal[0][i+1] && claim[1][i+1] > -1/* && safeMove(0, i+1) */)
          add_altmoves(0, i+1, myStones[0][i+1] == 2 ? v1 : v2, 0);
        else if (legal[0][i-1] && claim[1][i-1] > -1/* && safeMove(0, i-1) */)
          add_altmoves(0, i-1, myStones[0][i-1] == 2 ? v1 : v2, 0);
      }
    }
    if (ndbord[i][0] == - 1) {
      if (sList[stringIDs[i][0]].libC == 2) {
        if (legal[i+1][0] && claim[i+1][1] > -1/* && safeMove(i+1, 0)*/)
          add_altmoves(i+1, 0, myStones[i+1][0] == 2 ? v1 : v2, 0);
        else if (legal[i-1][0] && claim[i-1][1] > -1/* && safeMove(i-1, 0)*/)
          add_altmoves(i-1, 0, myStones[i-1][0] == 2 ? v1 : v2, 0);
      }
    }
    if (ndbord[maxPoint][i] == - 1) {
      if (sList[stringIDs[maxPoint][i]].libC == 2) {
        if (legal[maxPoint][i+1] && claim[maxPoint-1][i+1] > -1/* && safeMove(maxPoint, i+1)*/)
          add_altmoves(maxPoint, i+1, myStones[maxPoint][i+1] == 2 ? v1 : v2, 0);
        else if (legal[maxPoint][i-1] && claim[maxPoint-1][i-1] > -1/* && safeMove(maxPoint, i-1)*/)
          add_altmoves(maxPoint, i-1, myStones[maxPoint][i-1] == 2 ? v1 : v2, 0);
      }
    }
    if (ndbord[i][maxPoint] == - 1) {
      if (sList[stringIDs[i][maxPoint]].libC == 2) {
        if (legal[i+1][maxPoint] && claim[i+1][maxPoint-1] > -1/* && safeMove(i+1, maxPoint)*/)
          add_altmoves(i+1, maxPoint, myStones[i+1][maxPoint] == 2 ? v1 : v2, 0);
        else if (legal[i-1][maxPoint] && claim[i-1][maxPoint-1] > -1/* && safeMove(i-1, maxPoint)*/)
          add_altmoves(i-1, maxPoint, myStones[i-1][maxPoint] == 2 ? v1 : v2, 0);
      }
    }
 }
}
  /*
     drops to the edge of the board if threatened
   */
/* 끝내기 */
Getmove(dropToEdge)
{				/* dropToEdge */
  int i, v1 = 4, v2 = 5;

  for (i = 1; i <= maxPoint - 1; i++) {
    if (legal[1][i]) {
      if ((ndbord[2][i] == 1) && (ndbord[0][i] == 0) &&
	  (ndbord[1][i - 1] < 1) && (ndbord[1][i + 1] < 1)) {
	if ((ndbord[1][i - 1] == -1) || (ndbord[1][i + 1] == -1)) {
/*	  if (safeMove(1, i)) */
	    add_altmoves(1, i, v2, 0);
	} else if ((ndbord[2][i - 1] == -1) || (ndbord[2][i + 1] == -1)) {
/*	  if (safeMove(1, i)) { */
	    if (sList[stringIDs[2][i]].libC == 2)
	      add_altmoves(1, i, v1 + 3, 0);
	    else if (hisStones[1][i] - myStones[1][i] < 2)
	      add_altmoves(1, i, v1 - hisStones[maxPoint - 1][i], 0);
/*	  } */
	}
      }
      if (claim[1][i] < 0 && (safeConnect[1][i] ||
      		(bord[2][i] == 0 && ndbord[2][i-1] && ndbord[2][i+1] &&
		ndbord[2][i-1] != ndbord[2][i+1])) &&
		hisStones[1][i] - myStones[1][i] < 2) {
/*        if (safeMove(1,i)) */
          add_altmoves(1, i, 3 - hisStones[1][i], 0);
      }
    }
    if (legal[maxPoint - 1][i]) {
      if ((ndbord[maxPoint - 2][i] == 1) && (ndbord[maxPoint][i] == 0) &&
	  (ndbord[maxPoint - 1][i - 1] < 1) &&
	  (ndbord[maxPoint - 1][i + 1] < 1)) {
	if ((ndbord[maxPoint - 1][i - 1] == -1) ||
	    (ndbord[maxPoint - 1][i + 1] == -1)) {
/*	  if (safeMove(maxPoint - 1, i)) */
	    add_altmoves(maxPoint - 1, i, v2, 0);
	} else if ((ndbord[maxPoint - 2][i - 1] == -1) ||
	    (ndbord[maxPoint - 2][i + 1] == -1)) {
/*	  if (safeMove(maxPoint - 1, i)) { */
	    if (sList[stringIDs[maxPoint - 2][i]].libC == 2)
	      add_altmoves(maxPoint - 1, i, v1 + 3, 0);
	    else if (hisStones[maxPoint - 1][i] - myStones[maxPoint - 1][i] < 2)
	      add_altmoves(maxPoint - 1, i, v1 - hisStones[maxPoint - 1][i], 0);
/*	  } */
	}
      }
      if (claim[maxPoint - 1][i] < 0 && (safeConnect[maxPoint - 1][i] ||
      		(bord[maxPoint -2][i] == 0 && ndbord[maxPoint -2][i-1] && ndbord[maxPoint -2][i+1] &&
		ndbord[maxPoint -2][i-1] != ndbord[maxPoint -2][i+1])) &&
		hisStones[maxPoint - 1][i] - myStones[maxPoint - 1][i] < 2) {
/*        if (safeMove(maxPoint -1,i)) */
          add_altmoves(maxPoint -1, i, 3 - hisStones[maxPoint - 1][i], 0);
      }
    }
    if (legal[i][1]) {
      if ((ndbord[i][2] == 1) && (ndbord[i][0] == 0) &&
	  (ndbord[i - 1][1] < 1) && (ndbord[i + 1][1] < 1)) {
	if ((ndbord[i - 1][1] == -1) || (ndbord[i + 1][1] == -1)) {
/*	  if (safeMove(i, 1)) */
	    add_altmoves(i, 1, v2, 0);
	} else if ((ndbord[i - 1][2] == -1) || (ndbord[i + 1][2] == -1)) {
/*	  if (safeMove(i, 1)) { */
	    if (sList[stringIDs[i][2]].libC == 2)
	      add_altmoves(i, 1, v1 + 3, 0);
	    else if (hisStones[i][1] - myStones[i][1] < 2)
	      add_altmoves(i, 1, v1 - hisStones[i][1], 0);
/*	  } */
	}
      }
      if (claim[i][1] < 0 && (safeConnect[i][1] ||
      	(bord[i][2] == 0 && ndbord[i-1][2] && ndbord[i+1][2] &&
		ndbord[i-1][2] != ndbord[i+1][2])) &&
		hisStones[i][1] - myStones[i][1] < 2) {
/*        if (safeMove(i, 1)) */
          add_altmoves(i, 1, 3 - hisStones[i][1], 0);
      }
    }
    if (legal[i][maxPoint - 1]) {
      if ((ndbord[i][maxPoint - 2] == 1) &&
	  (ndbord[i][maxPoint] == 0) &&
	  (ndbord[i - 1][maxPoint - 1] < 1) &&
	  (ndbord[i + 1][maxPoint - 1] < 1)) {
	if ((ndbord[i - 1][maxPoint - 1] == -1) ||
	    (ndbord[i + 1][maxPoint - 1] == -1)) {
/*	  if (safeMove(i, maxPoint - 1)) */
	    add_altmoves(i, maxPoint - 1, v2, 0);
	} else if ((ndbord[i - 1][maxPoint - 2] == -1) ||
	    (ndbord[i + 1][maxPoint - 2] == -1)) {
/*	  if (safeMove(i, maxPoint - 1)) { */
	    if (sList[stringIDs[i][maxPoint - 2]].libC == 2)
	      add_altmoves(i, maxPoint - 1, v1 + 3, 0);
	    else if (hisStones[i][maxPoint - 1] - myStones[i][maxPoint - 1] < 2)
	      add_altmoves(i, maxPoint - 1, v1 - hisStones[i][maxPoint - 1], 0);
/*	  } */
	}
      }
      if (claim[i][maxPoint - 1] < 0 && (safeConnect[i][maxPoint - 1] ||
      	(bord[i][maxPoint -2] == 0 && ndbord[i-1][maxPoint -2] && ndbord[i+1][maxPoint -2] &&
		ndbord[i-1][maxPoint -2] != ndbord[i+1][maxPoint -2])) &&
		hisStones[i][maxPoint - 1] - myStones[i][maxPoint - 1] < 2) {
/*        if (safeMove(i, maxPoint -1)) */
          add_altmoves(i, maxPoint -1, 3 - hisStones[i][maxPoint - 1], 0);
      }
    }
    
    if (legal[0][i])
      if ((ndbord[1][i] == 1) &&
	  (ndbord[0][i - 1] < 1) &&
	  (ndbord[0][i + 1] < 1) &&
	  (((ndbord[1][i - 1] == -1) &&
		  (ndbord[1][i + 1] == -1)) ||
	      (ndbord[0][i - 1] == -1) ||
	      (ndbord[0][i + 1] == -1))) {
/*	if (safeMove(0, i)) */
	  add_altmoves(0, i, 3, 0);
      }
    if (legal[maxPoint][i])
      if ((ndbord[maxPoint - 1][i] == 1) &&
	  (ndbord[maxPoint][i - 1] < 1) &&
	  (ndbord[maxPoint][i + 1] < 1) &&
	  (((ndbord[maxPoint - 1][i - 1] == -1) &&
		  (ndbord[maxPoint - 1][i + 1] == -1)) ||
	      (ndbord[maxPoint][i - 1] == -1) ||
	      (ndbord[maxPoint][i + 1] == -1))) {
/*	if (safeMove(maxPoint, i)) */
	  add_altmoves(maxPoint, i, 3, 0);
      }
    if (legal[i][0])
      if ((ndbord[i][1] == 1) &&
	  (ndbord[i - 1][0] < 1) &&
	  (ndbord[i + 1][0] < 1) &&
	  (((ndbord[i - 1][1] == -1) &&
		  (ndbord[i + 1][1] == -1)) ||
	      (ndbord[i - 1][0] == -1) ||
	      (ndbord[i + 1][0] == -1))) {
/*	if (safeMove(i, 0)) */
	  add_altmoves(i, 0, 3, 0);
      }
    if (legal[i][maxPoint])
      if ((ndbord[i][maxPoint - 1] == 1) &&
	  (ndbord[i - 1][maxPoint] < 1) &&
	  (ndbord[i + 1][maxPoint] < 1) &&
	  (((ndbord[i - 1][maxPoint - 1] == -1) &&
		  (ndbord[i + 1][maxPoint - 1] == -1)) ||
	      (ndbord[i - 1][maxPoint] == -1) ||
	      (ndbord[i + 1][maxPoint] == -1))) {
/*	if (safeMove(i, maxPoint)) */
	  add_altmoves(i, maxPoint, 3, 0);
      }
  }
  return;
}				/* dropToEdge */

  /*
     Pushes walls in a tightly connected fashion.
     Finds the lowest influence (mine) point that is connected to one
     of my strings.
   */
Getmove(pushWall)
{				/* pushWall */
  int infl, i, j, na, x, y;

  x = iNil;
  y = iNil;
  infl = 11;
  for (i = 0; i <= maxPoint; i++)
    for (j = 0; j <= maxPoint; j++)
      if (legal[i][j])
	if (connectMap[i][j] > 0)
	  if ((claim[i][j] < infl) &&
	      (((i > 0) && (ndbord[i - 1][j] == 1)) ||
		  ((i < maxPoint) && (ndbord[i + 1][j] == 1)) ||
		  ((j > 0) && (ndbord[i][j - 1] == 1)) ||
		  ((j < maxPoint) && (ndbord[i][j + 1] == 1)) ||
		  ((i > 0) && (j > 0) && (ndbord[i - 1][j - 1] == 1)) ||
		  ((i < maxPoint) && (j > 0) && (ndbord[i + 1][j - 1] == 1)) ||
		  ((i > 0) && (j < maxPoint) && (ndbord[i - 1][j + 1] == 1)) ||
		  ((i < maxPoint) && (j < maxPoint) &&
		      (ndbord[i + 1][j + 1] == 1))) &&
	      (((i > 0) && (claim[i - 1][j] < 0)) ||
		  ((i < maxPoint) && (claim[i + 1][j] < 0)) ||
		  ((j > 0) && (claim[i][j - 1] < 0)) ||
		  ((j < maxPoint) && (claim[i][j + 1] < 0)))) {
	    na = 0;
	    if ((i > 0) && (ndbord[i - 1][j] != 0))
	      na = na + 1;
	    if ((i < maxPoint) && (ndbord[i + 1][j] != 0))
	      na = na + 1;
	    if ((j > 0) && (ndbord[i][j - 1] != 0))
	      na = na + 1;
	    if ((j < maxPoint) && (ndbord[i][j + 1] != 0))
	      na = na + 1;
	    if (na < 3)
/*	      if (safeMove(i, j)) */ {
		infl = claim[i][j];
		x = i;
		y = j;
	      }
	  }
  if (x != iNil)
    add_altmoves(x, y, 0.1, 0);

  return;
}				/* pushWall */


  /*
     reduces the liberty count of one of his strings
   */
Getmove(reduceHisLiberties)
{				/* reduceHisLiberties */
  int i, j;

  if (altmoves[0].px)
    return;
  sortLibs();
  for (i = 1; i <= maxStringID; i++)
    if ((!sList[sGlist[i]].isLive) &&
	(sList[sGlist[i]].libC > 2) &&
	(ndbord[sList[sGlist[i]].lx][sList[sGlist[i]].ly] == -1)) {
      spanString(sList[sGlist[i]].lx, sList[sGlist[i]].ly, &pList);
      for (j = 1; j <= pList.indx; j++)
	if (legal[pList.p[j].px][pList.p[j].py] &&
	    (connectMap[pList.p[j].px][pList.p[j].py] > 0))
/*	  if (safeMove(pList.p[j].px, pList.p[j].py)) */{
	    add_altmoves(pList.p[j].px, pList.p[j].py, 1, i);
	  }
    }
  return;
}				/* reduceHisLiberties */


  /*
     connects a string to the edge
   */
/* 끝내기2 */
Getmove(dropToEdge2)
{				/* dropToEdge2 */
  int i;

  for (i = 1; i <= maxPoint - 1; i++) {
    if (legal[i][0]) {
      if ((ndbord[i][1] == 1) &&
	  ((ndbord[i - 1][0] < 1) ||
	      (stringIDs[i - 1][0] != stringIDs[i][1])) &&
	  ((ndbord[i + 1][0] < 1) ||
	      (stringIDs[i + 1][0] != stringIDs[i][1])) &&
	  ((ndbord[i - 1][1] == -1) ||
	      (ndbord[i + 1][1] == -1))) {
/*	if (safeMove(i, 0)) */
	  add_altmoves(i, 0, 0.1, 0);
      }
    }
    if (legal[0][i]) {
      if ((ndbord[1][i] == 1) &&
	  ((ndbord[0][i - 1] < 1) ||
	      (stringIDs[0][i - 1] != stringIDs[1][i])) &&
	  ((ndbord[0][i + 1] < 1) ||
	      (stringIDs[0][i + 1] != stringIDs[1][i])) &&
	  ((ndbord[1][i - 1] == -1) ||
	      (ndbord[1][i + 1] == -1))) {
/*	if (safeMove(0, i)) */
	  add_altmoves(0, i, 0.1, 0);
      }
    }
    if (legal[i][maxPoint]) {
      if ((ndbord[i][maxPoint - 1] == 1) &&
	  ((ndbord[i - 1][maxPoint] < 1) ||
	      (stringIDs[i - 1][maxPoint] != stringIDs[i][maxPoint - 1])) &&
	  ((ndbord[i + 1][maxPoint] < 1) ||
	      (stringIDs[i + 1][maxPoint] != stringIDs[i][maxPoint - 1])) &&
	  ((ndbord[i - 1][maxPoint - 1] == -1) ||
	      (ndbord[i + 1][maxPoint - 1] == -1))) {
/*	if (safeMove(i, maxPoint)) */
	  add_altmoves(i, maxPoint, 0.1, 0);
      }
    }
    if (legal[maxPoint][i]) {
      if ((ndbord[maxPoint - 1][i] == 1) &&
	  ((ndbord[maxPoint][i - 1] < 1) ||
	      (stringIDs[maxPoint][i - 1] != stringIDs[maxPoint - 1][i])) &&
	  ((ndbord[maxPoint][i + 1] < 1) ||
	      (stringIDs[maxPoint][i + 1] != stringIDs[maxPoint - 1][i])) &&
	  ((ndbord[maxPoint - 1][i - 1] == -1) ||
	      (ndbord[maxPoint - 1][i + 1] == -1))) {
/*	if (safeMove(maxPoint, i)) */
	  add_altmoves(maxPoint, i, 0.1, 0);
      }
    }
  }
  return;
}				/* dropToEdge2 */

/* 공배 메우기 */
Getmove(fill_dame)
{
  int i, j, d, dx, dy;
  float value = 0;
  
  if (current_move < 100)
    return;

  for (i = 0; i <= maxPoint; i++)
    for (j = 0; j <= maxPoint; j++) {
      if (scoreBoard[i][j] == 0 && legal[i][j]) {
        value = 0.1;
        for (d = 0; d < 4; d++)
          if (get_dxy(d, i, j, 1, &dx, &dy)) {
            if (bord[dx][dy] != 0) {
              if (sList[stringIDs[dx][dy]].libC <= 2)
                value += bord[dx][dy] == 1 ? 0.5 : 1;
            } else if (claim[dx][dy] != 0) {
              value += 0.4;
            }
          }

/*        if (i > 0 && bord[i-1][j] != 0 && sList[stringIDs[i-1][j]].libC <= 2)
          value = 2;
        if (i < maxPoint && bord[i+1][j] != 0 && sList[stringIDs[i+1][j]].libC <= 2)
          value = 2;
        if (j > 0 && bord[i][j-1] != 0 && sList[stringIDs[i][j-1]].libC <= 2)
          value = 2;
        if (j < maxPoint && bord[i][j+1] != 0 && sList[stringIDs[i][j+1]].libC <= 2)
          value = 2;
        if (value < 2) {
          if (i > 0 && bord[i-1][j] == 0 && claim[i-1][j] > 0)
            value = 1;
          if (i < maxPoint && bord[i+1][j] == 0 && claim[i+1][j] > 0)
            value = 1;
          if (j > 0 && bord[i][j-1] == 0 && claim[i][j-1] > 0)
            value = 1;
          if (j < maxPoint && bord[i][j+1] == 0 && claim[i][j+1] > 0)
            value = 1;
        } */

        if (value > 0) {
          value += (float) powerBoard[i][j] / 10;
          add_altmoves(i, j, value, 0);
        }
      }
    }
}
