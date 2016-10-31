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

/* $Id: killable.c,v 1.17 1999/04/12 16:31:47 artist Exp artist $ */

#include <cstdlib>
#include <iostream>
#include <cstdio>

using namespace std;

#include "game.h"
#include "baduki.h"

#define bdk_debug 0

extern game thegame;

extern stringRec sList[maxString];
extern int SMap[maxString], adjInAtari, adj2Libs, playMark, treeLibLim,
  utilPlayLevel, killFlag, depthLimit, dbStop, showTrees;

int libList[maxSPoint + 1];
int adj1, adj2;
sPointList dList;
extern intList aList;

int me, him, i, j, tryCount, tl, topMark, tkMark, mark2, mark3;
char sChar;
point tp;
int esc;
extern int last_tx, last_ty, last_try_is_pae;
int tryLimit = Default_tryLimit;

int mtNbrs(int x, int y)
{				/* mtNbrs */
  int n = 0;
  if ((x > 0) && (bord[x - 1][y] == 0))
    n = n + 1;
  if ((x < maxPoint) && (bord[x + 1][y] == 0))
    n = n + 1;
  if ((y > 0) && (bord[x][y - 1] == 0))
    n = n + 1;
  if ((y < maxPoint) && (bord[x][y + 1] == 0))
    n = n + 1;
  return n;
}				/* mtNbrs */

int check_pae(int x, int y, int color)
{
  if ((x > 0) && (bord[x - 1][y] != color))
    return FALSE;
  if ((x < maxPoint) && (bord[x + 1][y] != color))
    return FALSE;
  if ((y > 0) && (bord[x][y - 1] != color))
    return FALSE;
  if ((y < maxPoint) && (bord[x][y + 1] != color))
    return FALSE;

  return TRUE;
}

void listAdjacents(int x, int y, intList * iL);

int get_moves_for_kill(sPointList *lList, int gx, int gy, int depth)
{
  int k, ii, tlc, icnt = 0;

  for (k = 1; k <= lList->indx; k++)
    libList[k] = sList[SMap[stringIDs[gx][gy]]].libC - 1;

  /* 한칸 뜀 고려 */
  if (depth == 0 && sList[SMap[stringIDs[gx][gy]]].libC <= 3 && 
  	utilPlayLevel > 6) {
    listJump1(SMap[stringIDs[gx][gy]], *lList, &dList);
    
    for (j =1; j <= dList.indx && lList->indx < maxSPoint; j++) {
      lList->indx++;
      lList->p[lList->indx].px = dList.p[j].px;
      lList->p[lList->indx].py = dList.p[j].py;
/*      if (get_height(dList.p[j].px, dList.p[j].py) > 0)
        libList[lList->indx] = 0;
      else */
        libList[lList->indx] = 100;
    }
  }

  /* 상대돌이 단수상태가 아니고 내돌이 단수가 되어 있는 경우
     단수된 돌을 보강한다 */
  if (adjInAtari && GET_LIBC(gx, gy) > 1) {	/* one of my options is to kill */
    listAdjacents(gx, gy, &aList);
    for (k = 1; k <= aList.indx; k++)
      if (sList[SMap[aList.v[k]]].libC == 1 && lList->indx < maxSPoint) {
	sSpanString(sList[SMap[aList.v[k]]].lx, sList[SMap[aList.v[k]]].ly,
	    &dList);
	lList->indx = lList->indx + 1;
	libList[lList->indx] = 0;
	lList->p[lList->indx].px = dList.p[1].px;
	lList->p[lList->indx].py = dList.p[1].py;
      }
  }
  if ((depth == 0 || (utilPlayLevel > 7 && tryCount < tryLimit)) &&
      (sList[SMap[stringIDs[gx][gy]]].libC > 2) && adj2) {
    listAdjacents(gx, gy, &aList);
    for (k = 1; k <= aList.indx; k++)
      if (sList[SMap[aList.v[k]]].libC == 2 && lList->indx < maxSPoint) {
	sSpanString(sList[SMap[aList.v[k]]].lx, sList[SMap[aList.v[k]]].ly,
	    &dList);
	lList->indx++;
	libList[lList->indx] = 100;
	lList->p[lList->indx].px = dList.p[1].px;
	lList->p[lList->indx].py = dList.p[1].py;
	if (lList->indx < maxSPoint) {
	  lList->indx++;
	  libList[lList->indx] = 100;
	  lList->p[lList->indx].px = dList.p[2].px;
	  lList->p[lList->indx].py = dList.p[2].py;
	}
      }
  }
  /* 대각선 */
  if (depth == 0 || ((utilPlayLevel > 4) && (tryCount < tryLimit))) {
    listDiags(gx, gy, &dList);
    for (j = 1; j <= dList.indx && lList->indx < maxSPoint; j++) {
      lList->indx++;
      /* 장문일 경우는 대각선으로 씌우는 수를 먼저 고려 */
      if (sList[SMap[stringIDs[gx][gy]]].libC <= 3 && !adj2 &&
	  dList.p[j].px == dList.p[0].px &&
	  dList.p[j].py == dList.p[0].py)
	libList[lList->indx] = 0;
      else
	libList[lList->indx] = 100;
      lList->p[lList->indx].px = dList.p[j].px;
      lList->p[lList->indx].py = dList.p[j].py;
    }
  }
  if (lList->indx <= 1)
    return TRUE;

  /* 내 돌을 놓아 보고 상대돌의 활로가 몇개가 되는지를 조사한다 */
  for (k = 1; k <= lList->indx; k++)
    if (libList[k] != 100 && libList[k] != 0) {
      mark3 = playMark;
      tryPlay(lList->p[k].px, lList->p[k].py, me);
      if (GET_LIBC(lList->p[k].px, lList->p[k].py) == 0) {
      /* illegal move */
        lList->p[k].px = -1;
        libList[k] = 999;
        icnt++;
      } else
        libList[k] = sList[SMap[stringIDs[gx][gy]]].libC;
      undoTo(mark3);
    }
  /* 상대 활로를 적게 만드는 순으로 정렬 */
  for (k = 1; k <= lList->indx - 1; k++)
    for (j = k + 1; j <= lList->indx; j++)
      if (libList[k] > libList[j]) {
	tl = libList[k];
	libList[k] = libList[j];
	libList[j] = tl;
	tp = lList->p[k];
	lList->p[k] = lList->p[j];
	lList->p[j] = tp;
      } else if ((libList[k] == libList[j]) &&
	  libList[k] != 100 && libList[k] != 0 && libList[k] != 999) {
	mark3 = playMark;
	tryPlay(lList->p[j].px, lList->p[j].py, me);
	tryPlay(lList->p[k].px, lList->p[k].py, him);
	tlc = sList[SMap[stringIDs[gx][gy]]].libC;
	if ((lList->p[k].px > 0 &&
		bord[lList->p[k].px - 1][lList->p[k].py] == me &&
		GET_LIBC(lList->p[k].px - 1, lList->p[k].py) == 1) ||
	    (lList->p[k].px < maxPoint &&
		bord[lList->p[k].px + 1][lList->p[k].py] == me &&
		GET_LIBC(lList->p[k].px + 1, lList->p[k].py) == 1) ||
	    (lList->p[k].py > 0 &&
		bord[lList->p[k].px][lList->p[k].py - 1] == me &&
		GET_LIBC(lList->p[k].px, lList->p[k].py - 1) == 1) ||
	    (lList->p[k].py < maxPoint &&
		bord[lList->p[k].px][lList->p[k].py + 1] == me &&
		GET_LIBC(lList->p[k].px, lList->p[k].py + 1) == 1))
	  adj1 = 1;
	else
	  adj1 = 0;

	undoTo(mark3);
	mark3 = playMark;
	tryPlay(lList->p[k].px, lList->p[k].py, me);
	tryPlay(lList->p[j].px, lList->p[j].py, him);
	if ((lList->p[j].px > 0 &&
		bord[lList->p[j].px - 1][lList->p[j].py] == me &&
		GET_LIBC(lList->p[j].px - 1, lList->p[j].py) == 1) ||
	    (lList->p[j].px < maxPoint &&
		bord[lList->p[j].px + 1][lList->p[j].py] == me &&
		GET_LIBC(lList->p[j].px + 1, lList->p[j].py) == 1) ||
	    (lList->p[j].py > 0 &&
		bord[lList->p[j].px][lList->p[j].py - 1] == me &&
		GET_LIBC(lList->p[j].px, lList->p[j].py - 1) == 1) ||
	    (lList->p[j].py < maxPoint &&
		bord[lList->p[j].px][lList->p[j].py + 1] == me &&
		GET_LIBC(lList->p[j].px, lList->p[j].py + 1) == 1))
	  adj2 = 1;
	else
	  adj2 = 0;
	if (!adj1 && adj2) {
/* 상대 돌이 놓일때 내돌이 단수되지 않는 것을 우선한다. */
	  tl = libList[k];
	  libList[k] = libList[j];
	  libList[j] = tl;
	  tp = lList->p[k];
	  lList->p[k] = lList->p[j];
	  lList->p[j] = tp;
	} else if (tlc < sList[SMap[stringIDs[gx][gy]]].libC) {
	  tl = libList[k];
	  libList[k] = libList[j];
	  libList[j] = tl;
	  tp = lList->p[k];
	  lList->p[k] = lList->p[j];
	  lList->p[j] = tp;
	} else if (tlc == sList[SMap[stringIDs[gx][gy]]].libC &&
	      tlc == 2 &&
	      ((lList->p[j].px > 0 &&
		      bord[lList->p[j].px - 1][lList->p[j].py] == 0 &&
		      lList->p[j].px < maxPoint &&
		      bord[lList->p[j].px + 1][lList->p[j].py] == 0) ||

		  (lList->p[j].py > 0 &&
		      bord[lList->p[j].px][lList->p[j].py - 1] == 0 &&
		      lList->p[j].py < maxPoint &&
		      bord[lList->p[j].px][lList->p[j].py + 1] == 0)) &&

	      ((lList->p[k].px > 0 &&
		      bord[lList->p[k].px - 1][lList->p[k].py] == 0 &&
		      lList->p[k].py > 0 &&
		      bord[lList->p[k].px][lList->p[k].py - 1] == 0) ||

		  (lList->p[k].px > 0 &&
		      bord[lList->p[k].px - 1][lList->p[k].py] == 0 &&
		      lList->p[k].py < maxPoint &&
		      bord[lList->p[k].px][lList->p[k].py + 1] == 0) ||

		  (lList->p[k].py < maxPoint &&
		      bord[lList->p[k].px + 1][lList->p[k].py] == 0 &&
		      lList->p[k].py > 0 &&
		      bord[lList->p[k].px][lList->p[k].py - 1] == 0) ||

		  (lList->p[k].py < maxPoint &&
		      bord[lList->p[k].px + 1][lList->p[k].py] == 0 &&
		      lList->p[k].py < maxPoint &&
		    bord[lList->p[k].px][lList->p[k].py + 1] == 0))) {
/* 현재돌을 놓은 후 상대돌의 진행 후에도 활로의 숫자가 같을때는
   축이 되는 것을 먼저 살피기 위해 상대돌 진행후 활로가 대각인것을 먼저 찾는다. */
	  tl = libList[k];
	  libList[k] = libList[j];
	  libList[j] = tl;
	  tp = lList->p[k];
	  lList->p[k] = lList->p[j];
	  lList->p[j] = tp;
	} else if (tlc == sList[SMap[stringIDs[gx][gy]]].libC &&
	      height_cmp(lList->p[j].px, lList->p[j].py,
		lList->p[k].px, lList->p[k].py) > 0) {
/* 그래도 똑같은 조건이면 높은 위치에 돌을 먼저 고려한다 */
	  tl = libList[k];
	  libList[k] = libList[j];
	  libList[j] = tl;
	  tp = lList->p[k];
	  lList->p[k] = lList->p[j];
	  lList->p[j] = tp;
	}
	undoTo(mark3);
      }
  lList->indx -= icnt;
  if (bdk_debug) {
    cerr << "lKill " << depth << " C" << tryCount << ":";
    for (ii = 1; ii <= lList->indx; ii++)
      cerr << " " << ('a' + lList->p[ii].px) << (lList->p[ii].py + 1);
    cerr << endl;
  }
  return TRUE;
}

/* 돌을 살리기 위해 고려할 위치를 구한다 */
/* 돌을 확실히 살릴 수가 생기면 그 수가 lList 인덱스 값을 되돌린다. */
int get_moves_for_save(sPointList *lList, int gx, int gy, 
		int tx, int ty, int depth)
{
  int i, j, k, adj1, adj2;
  
  sSpanString(gx, gy, lList);
  adj1 = adjInAtari;
  adj2 = adj2Libs;

  for (k = 1; k <= lList->indx; k++)
    libList[k] = -1;

  /* 한칸 뜀 고려 */
  if (depth == 0 && sList[SMap[stringIDs[gx][gy]]].libC >= 2 && 
  	utilPlayLevel > 6) {
    listJump1(SMap[stringIDs[gx][gy]], *lList, &dList);
    
    for (j =1; j <= dList.indx && lList->indx < maxSPoint; j++) {
      lList->indx++;
      lList->p[lList->indx].px = dList.p[j].px;
      lList->p[lList->indx].py = dList.p[j].py;
/*      if (get_height(dList.p[j].px, dList.p[j].py) > 0)
        libList[lList->indx] = 100;
      else */
        libList[lList->indx] = 0;
    }
  }

  /* 주위에 단수 되어 있는 돌이 있으면 잡는다 */
  if (adj1) {
    listAdjacents(gx, gy, &aList);
    for (k = 1; k <= aList.indx; k++)
      if (sList[SMap[aList.v[k]]].libC == 1 && lList->indx < maxSPoint) {
	sSpanString(sList[SMap[aList.v[k]]].lx, sList[SMap[aList.v[k]]].ly,
	    &dList);
	/* 패가 아닌지 검사 한후 목록에 추가 한다 */
	if (!(last_try_is_pae && 
		sList[SMap[aList.v[k]]].lx == last_tx &&
		sList[SMap[aList.v[k]]].ly == last_ty)) {
	  lList->indx = lList->indx + 1;
	  lList->p[lList->indx].px = dList.p[1].px;
	  lList->p[lList->indx].py = dList.p[1].py;
	  libList[lList->indx] = 100;
	}
      }
  }
  /* 돌의 활로가 2이상이고 주위돌중 활로가 2개인것인 있을때
     잡을 수 있는지 고려 */
  if ((adj2 || (depth > 0 && GET_LIBC(tx,ty) == 2)) && 
  	utilPlayLevel > 6 && (tryCount < (tryLimit - 1)) &&
  	(sList[SMap[stringIDs[gx][gy]]].libC >= 2)) {
    listAdjacents(gx, gy, &aList);
    for (k = 1; k <= aList.indx; k++)
      if (sList[SMap[aList.v[k]]].libC == 2 && lList->indx < maxSPoint) {
	sSpanString(sList[SMap[aList.v[k]]].lx, sList[SMap[aList.v[k]]].ly,
	    &dList);
	lList->indx++;
	lList->p[lList->indx].px = dList.p[1].px;
	lList->p[lList->indx].py = dList.p[1].py;
	libList[lList->indx] = 100;
	if (lList->indx < maxSPoint) {
	  lList->indx++;
	  libList[lList->indx] = 100;
	  lList->p[lList->indx].px = dList.p[2].px;
	  lList->p[lList->indx].py = dList.p[2].py;
	}
      }
  }
/* 대각선 (모행마) 고려 */
  if ((utilPlayLevel > 4) && (tryCount < tryLimit) &&
      (lList->indx > 1) &&
      (sList[SMap[stringIDs[gx][gy]]].libC > 1)) {
    listDiags(gx, gy, &dList);
    j = 0;
    i = lList->indx;
    while ((j < dList.indx) &&
	(i < maxSPoint)) {
      j = j + 1;
      i = i + 1;
      libList[i] = 0;		/* mark this as a diag */
      lList->p[i].px = dList.p[j].px;
      lList->p[i].py = dList.p[j].py;
    }
    lList->indx = i;
  }
  if (bdk_debug) {
    printf("lSave %d C%d:", depth, tryCount);
    for (i = 1; i <= lList->indx; i++)
      printf(" %c%d", 'a' + lList->p[i].px, lList->p[i].py + 1);
    printf("\n");
  }
  if (lList->indx > 1) {		/* sort by decreasing lib count */
    for (i = 1; i <= lList->indx; i++)
      if (libList[i] != 0 && libList[i] != 100) {	/* diags are tried last */
	mark2 = playMark;
	tryPlay(lList->p[i].px, lList->p[i].py, him);
	libList[i] = sList[SMap[stringIDs[gx][gy]]].libC;
	if ((libList[i] > treeLibLim) ||
	    ((libList[i] > (depthLimit - depth)) &&
		(libList[i] > 2))) {
	  return i;
	}
	undoTo(mark2);
      }
    for (i = 1; i <= lList->indx - 1; i++)
      for (j = i + 1; j <= lList->indx; j++)
	if (libList[i] < libList[j]) {
	  tl = libList[i];
	  libList[i] = libList[j];
	  libList[j] = tl;
	  tp = lList->p[i];
	  lList->p[i] = lList->p[j];
	  lList->p[j] = tp;
	}
  }
  
  return 0;
}


int killTree(int tx, int ty, int gx, int gy, int *escape, int tkMark, int depth)

{				/* killTree */
  int curMark, mark2, i, j, k, dStart, result = -1;
  sPointList lList1, lList2;
  int esc = FALSE;
  char *rmsg = NULL;

  tryCount = tryCount + 1;
  depth = depth + 1;
  curMark = playMark;
  tryPlay(tx, ty, me);		/* try my move */
/*  bdk_pause(); */
  if (sList[SMap[stringIDs[tx][ty]]].libC == 0) {	/* I'm dead */
    result = FALSE;
    rmsg = "illegal move";	/* illegal move */
    goto one;
  } else if (tryCount > tryLimit /* && sList[SMap[stringIDs[gx][gy]]].libC > 1 */ ) {
    return FALSE;
  } else if (killFlag && 
  	((!sList[SMap[killFlag]].atLevel && sList[SMap[killFlag]].isSafe) ||
	  sList[SMap[killFlag]].size > sList[SMap[stringIDs[gx][gy]]].size)) {
    /* I killed something of his safe or bigger string */
    result = TRUE;
    rmsg = "I killed something of his";
    goto one;
  } else if (sList[SMap[stringIDs[gx][gy]]].libC > treeLibLim) {	/* safe */
    result = FALSE;
    rmsg = "safe";
    goto one;
  } else {
/*    sSpanString(gx, gy, &lList1);
    adj1 = adjInAtari;
    adj2 = adj2Libs; */

    if (get_moves_for_save(&lList1, gx, gy, tx, ty, depth)) {
      *escape = TRUE;
      result = FALSE;
      rmsg = "escaped";
      goto one;
    }

    for (i = 1; i <= lList1.indx + 1; i++) {	/* try his responses */
      mark2 = playMark;
      if (i <= lList1.indx) {	/* try his move */
	tryPlay(lList1.p[i].px, lList1.p[i].py, him);	/* play his response */
/*      bdk_pause(); */
	if (sList[SMap[stringIDs[gx][gy]]].libC < 2) {
	  rmsg = "bogus move";
	  goto two;		/* a bogus move */
	} else if (sList[SMap[stringIDs[lList1.p[i].px]
		    [lList1.p[i].py]]].libC == 0) {
	  rmsg = "illegal move";
	  goto two;		/* illegal move */
	} else if (killFlag && sList[SMap[killFlag]].size > 3) {
	  *escape = TRUE;
	  result = FALSE;
	  rmsg = "He killed something";
	  goto one;
	}
      } else if (sList[SMap[stringIDs[gx][gy]]].libC < 1) {
	result = TRUE;
	rmsg = "can capture";
	goto one;
      }
      if (sList[SMap[stringIDs[gx][gy]]].libC > treeLibLim) {
	*escape = TRUE;
	result = FALSE;
	rmsg = "escaped";
	goto one;
      }
      if (sList[SMap[stringIDs[gx][gy]]].libC >= 1) {	/* look at my responses */
	sSpanString(gx, gy, &lList2);	/* list his liberties */
	adj1 = adjInAtari;
	adj2 = adj2Libs;

	if (sList[SMap[stringIDs[gx][gy]]].libC == 1 && !adjInAtari) {
	  result = TRUE;
	  rmsg = "can capture";
	  goto one;
	}
	dStart = lList2.indx + 1;
	for (k = 1; k <= maxSPoint; k++)
	  libList[k] = -1;

	get_moves_for_kill(&lList2, gx, gy, depth);

	for (j = 1; j <= lList2.indx; j++) {
	  if (depth == 1)
	    tryCount = 0;

	  if (killTree(lList2.p[j].px, lList2.p[j].py, gx,
		  gy, &esc, tkMark, depth)) {
	    rmsg = "this kills him";
	    goto two;		/* this kills him */
	  }
	  if (esc && (j >= dStart) && depth > 2) {
	    result = FALSE;
	    rmsg = "don't bother with more diags if escapes";
	    goto one;		/* don't bother with more diags if escapes */
	  }
	}
	rmsg = "none of my responses kills him";
	result = FALSE;		/* none of my responses kills him */
	goto one;
      }
    two:
      if (bdk_debug) {
	printf("%3d ", tryCount);
	showStack();
	printf(" two:%d %s\n", result, rmsg ? rmsg : "-");
	bdk_pause();
      }
      undoTo(mark2);
    }
    result = TRUE;		/* none of his responses saves him */
  }
one:
  if (bdk_debug) {
    printf("%3d ", tryCount);
    showStack();
    printf(" one:%d %s\n", result, rmsg ? rmsg : "-");
    bdk_pause();
  }
  undoTo(curMark);

  return result;
}				/* killTree */

int tKillTree(int tx, int ty, int gx, int gy)
{				/* tKillTree */
  int tkMark, escape;
  tryCount = 0;
  tkMark = playMark;
  return killTree(tx, ty, gx, gy, &escape, tkMark, 0);
}				/* tKillTree */

/*
   returns true if the string (at x, y) is killable.
   if so, returns the point to play at in killx, killy.
 */

int killable(int gx, int gy, int *killx, int *killy)
{				/* killable */
  int kmark;
  sPointList lList;

#ifdef DEBUG
  printf("killable\n");
  showTrees = TRUE;
#endif
  dbStop = TRUE;
  him = bord[gx][gy];		/* find out who I am */
  me = -him;
  kmark = playMark;
  sSpanString(gx, gy, &lList);	/* find his liberties */
  if (lList.indx == 1) {
    /* 패는 바로 따내지 못한다. */
    if (last_try_is_pae &&
	SMap[stringIDs[gx][gy]] == SMap[stringIDs[last_tx][last_ty]]) {
      lList.indx = 0;
    } else {
      *killx = lList.p[1].px;
      *killy = lList.p[1].py;
      undoTo(kmark);
      return TRUE;
    }
  }
  if (lList.indx > treeLibLim) {
    undoTo(kmark);
    return FALSE;
  }

  adj1 = adjInAtari;
  adj2 = adj2Libs;
  get_moves_for_kill(&lList, gx, gy, 0);

  for (i = 1; i <= lList.indx; i++) {
    if (legal[lList.p[i].px][lList.p[i].py]) {
      *killx = lList.p[i].px;
      *killy = lList.p[i].py;
      if (tKillTree(*killx, *killy, gx, gy)) {
        undoTo(kmark);
	return TRUE;
      }
    }
  }
  undoTo(kmark);
  return FALSE;
}				/* killable */

/*
   returns true if (the string (at gx, gy) is saveable.
   if so, returns the point to play at in savex, savey
 */
/* (gx,gy)에 놓여진 돌을 살릴 수 있으면 (savex, savey)에 좌표를 넘김 */
int saveable(int gx, int gy, int *savex, int *savey)
{				/* saveable */
  int me, him, gx1, gx2, i, smark, result = FALSE;
  sPointList lList;

#ifdef DEBUG
  printf("saveable\n");
#endif
  dbStop = TRUE;
  me = bord[gx][gy];
  him = -me;
  smark = playMark;

/*  sSpanString(gx, gy, &lList);
  adj1 = adjInAtari;
  adj2 = adj2Libs; */

  if ((result = get_moves_for_save(&lList, gx, gy, -1, -1, 0))) {
    *savex = lList.p[result].px;
    *savey = lList.p[result].py;
  } else {
    for (i = 1; i <= lList.indx; i++) {
      *savex = lList.p[i].px;
      *savey = lList.p[i].py;
      if (legal[*savex][*savey]) {
	tryPlay(*savex, *savey, me);
	bdk_pause();
	if (killFlag && sList[SMap[killFlag]].size > 3) {
	  undoTo(smark);
	  return TRUE;
	}
	if (sList[SMap[stringIDs[*savex][*savey]]].libC > 1)
	  if (sList[SMap[stringIDs[gx][gy]]].libC > treeLibLim) {
	    undoTo(smark);
	    return TRUE;
	  } else if (sList[SMap[stringIDs[gx][gy]]].libC > 1)
	    if (!killable(gx, gy, &gx1, &gx2)) {
	      undoTo(smark);
	      return TRUE;
	    }
	undoTo(smark);
      }
    }
  }
  undoTo(smark);
  return result;
}				/* saveable */
