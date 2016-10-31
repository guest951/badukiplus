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

/* $Id: utils.c,v 1.24 1999/04/01 20:23:41 artist Exp artist $ */

#include <string.h>
#include <cstdlib>
#include <iostream>
#include <sys/time.h>
#include <unistd.h>

using namespace std;

#include "game.h"
#include "baduki.h"

extern game thegame;
extern struct bRec goboard[maxSIZE][maxSIZE];

intBoard legal, claim, extra, bord, ndbord, safeboard, sGroups, threatBord,
	stringIDs, connectMap, protPoints, powerBoard, powerBoard2, libBoard,
	scoreBoard, safeConnect, GIDMap, wGIDMap, vEyeMap, powerBoard1,
	valueBoard, myStones, hisStones, ignoreSafe, connectMap2,
	extMap[4], jump1Map;
intBoard stringSeen, markBoard;

intBoard nullBoard = {
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};
int maxStringID;
int max_veid;
pointList pList, pList1, plist2, plist3, pPlist;
extern intList aList;
intList nlcString;
sgRec sgList[401];
vsgRec vsgList[181];
stringRec sList[maxString];
groupRec gList[MAX_GROUP];
groupRec wgList[MAX_GROUP];
int killFlag, numCapt, utilPlayLevel, treeLibLim;
sType mySType;
int showTrees;
int sGlist[maxString + 1];
int depthLimit;
int marker;

int adjInAtari, adj2Libs, intersectNum, spanNum, libMark;
playRec playStack[1025];
int playMark, newSID, tryLevel, grpMark, SMap[maxString];
int dbStop, inGenState;
int last_tx = 0, last_ty = 0;	/* last move by tryPlay() */
int last_try_is_pae = FALSE;
int try_counter = 0;

extern int board_size;
int height_cmp(int x1, int y1, int x2, int y2);

void bdk_pause()
{				/* pause */
}				/* pause */

void sstone(int w, int x, int y, int numb)
{				/* sstone */
  if (w == 1)
    placestone(mySType, x, y);
  else if (mySType == WHITE)
    placestone(BLACK, x, y);
  else
    placestone(WHITE, x, y);
}				/* sstone */

void rstone(int x, int y)
{				/* rstone */
  removestone(x, y);
}				/* rstone */

void sortLibs()
{				/* sortLibs */
  int i, j, t;
#ifdef DEBUG
  printf("sortLibs\n");
#endif
  for (i = 1; i <= maxStringID; i++)
    sGlist[i] = i;
  for (i = 1; i < maxStringID; i++)
    for (j = i + 1; j <= maxStringID; j++)
      if (sList[sGlist[i]].libC > sList[sGlist[j]].libC) {
	t = sGlist[i];
	sGlist[i] = sGlist[j];
	sGlist[j] = t;
      }
}				/* sortLibs */

void spanStringspan(int x, int y, pointList *libs, int lookFor)
{				/* span */
  markBoard[x][y] = marker;
  if (bord[x][y] == 0) {
    libs->indx = libs->indx + 1;
    libs->p[libs->indx].px = x;
    libs->p[libs->indx].py = y;
  } else if (bord[x][y] == lookFor) {
    stringSeen[x][y] = TRUE;
    if ((x > 0) && (markBoard[x - 1][y] != marker))
      spanStringspan(x - 1, y, libs, lookFor);
    if ((y > 0) && (markBoard[x][y - 1] != marker))
      spanStringspan(x, y - 1, libs, lookFor);
    if ((x < maxPoint) && (markBoard[x + 1][y] != marker))
      spanStringspan(x + 1, y, libs, lookFor);
    if ((y < maxPoint) && (markBoard[x][y + 1] != marker))
      spanStringspan(x, y + 1, libs, lookFor);
  } else if (sList[SMap[stringIDs[x][y]]].libC == 1)
    adjInAtari = stringIDs[x][y];
  else if ((sList[SMap[stringIDs[x][y]]].libC == 2) &&
      (!sList[SMap[stringIDs[x][y]]].isSafe))
    adj2Libs = stringIDs[x][y];
}				/* span */

void spanString(int x, int y, pointList *libs)
{				/* spanString */
  int lookFor;
#ifdef DEBUG
  printf("spanString\n");
#endif
  marker = marker + 1;
  if (marker == 0) {
    initArray(markBoard);
    marker = 1;
  }
  adjInAtari = 0;
  adj2Libs = 0;
  lookFor = bord[x][y];
  libs->indx = 0;
  spanStringspan(x, y, libs, lookFor);
}				/* spanString */

void sSpanStringspan(int x, int y, sPointList *libs, int lookFor)
{				/* span */
  markBoard[x][y] = marker;
  if (bord[x][y] == 0) {
    libs->indx += 1;
    if (libs->indx <= maxSPoint) {
      libs->p[libs->indx].px = x;
      libs->p[libs->indx].py = y;
    }
  } else if (bord[x][y] == lookFor) {
    stringSeen[x][y] = TRUE;
    if ((x > 0) && (markBoard[x - 1][y] != marker))
      sSpanStringspan(x - 1, y, libs, lookFor);
    if ((y > 0) && (markBoard[x][y - 1] != marker))
      sSpanStringspan(x, y - 1, libs, lookFor);
    if ((x < maxPoint) && (markBoard[x + 1][y] != marker))
      sSpanStringspan(x + 1, y, libs, lookFor);
    if ((y < maxPoint) && (markBoard[x][y + 1] != marker))
      sSpanStringspan(x, y + 1, libs, lookFor);
  } else if (sList[SMap[stringIDs[x][y]]].libC == 1)
    adjInAtari = stringIDs[x][y];
  else if ((sList[SMap[stringIDs[x][y]]].libC == 2) &&
      (!sList[SMap[stringIDs[x][y]]].isSafe))
    adj2Libs = stringIDs[x][y];
}				/* span */

void sSpanString(int x, int y, sPointList *libs)
{				/* sSpanString */
  int lookFor;
#ifdef DEBUG
  printf("sSpanString\n");
#endif
  marker = marker + 1;
  if (marker == 0) {
    initArray(markBoard);
    marker = 1;
  }
  adjInAtari = 0;
  adj2Libs = 0;
  lookFor = bord[x][y];
  libs->indx = 0;
  sSpanStringspan(x, y, libs, lookFor);
}				/* sSpanString */

void LAspan(int x, int y, int me, int him, intList *iL)
{				/* span */
#ifdef DEBUG
  printf("LAspan\n");
#endif
  markBoard[x][y] = marker;
  if (bord[x][y] == me) {
    if ((x > 0) && (markBoard[x - 1][y] != marker))
      LAspan(x - 1, y, me, him, iL);
    if ((x < maxPoint) && (markBoard[x + 1][y] != marker))
      LAspan(x + 1, y, me, him, iL);
    if ((y > 0) && (markBoard[x][y - 1] != marker))
      LAspan(x, y - 1, me, him, iL);
    if ((y < maxPoint) && (markBoard[x][y + 1] != marker))
      LAspan(x, y + 1, me, him, iL);
  } else if (bord[x][y] == him)
    if (sList[SMap[stringIDs[x][y]]].stringMark != grpMark) {
      sList[SMap[stringIDs[x][y]]].stringMark = grpMark;
      iL->indx = iL->indx + 1;
      iL->v[iL->indx] = SMap[stringIDs[x][y]];
    }
}				/* span */

void listAdjacents(int x, int y, intList *iL)
{				/* listAdjacents */
  int me, him;
#ifdef DEBUG
  printf("listAdjacents\n");
#endif
  grpMark = grpMark + 1;
  marker = marker + 1;
  if (marker == 0) {
    initArray(markBoard);
    marker = 1;
  }
  iL->indx = 0;
  me = bord[x][y];
  him = -me;
  LAspan(x, y, me, him, iL);
}				/* listAdjacents */

void LDspan(int x, int y, int me, sPointList *diags)
{				/* span */
#ifdef DEBUG
  printf("LDspan\n");
#endif
  markBoard[x][y] = marker;
  if ((x > 0) && (y > 0) &&
      (bord[x - 1][y - 1] == 0) &&
      (bord[x][y - 1] != me) &&
      (bord[x - 1][y] != me) &&
      !((bord[x][y - 1] != 0) && (bord[x - 1][y] != 0) &&
      	sList[SMap[stringIDs[x][y-1]]].libC > 2 && 
      	sList[SMap[stringIDs[x-1][y]]].libC > 2) &&
      (markBoard[x - 1][y - 1] != marker)) {
    markBoard[x - 1][y - 1] = marker;
    diags->indx = diags->indx + 1;
    if (diags->indx <= maxSPoint) {
      diags->p[diags->indx].px = x - 1;
      diags->p[diags->indx].py = y - 1;
      if ((bord[x][y - 1] == 0) && (bord[x - 1][y] == 0)) {
        diags->p[0].px = x - 1;
        diags->p[0].py = y - 1;
      }
    }
  }
  if ((x < maxPoint) && (y > 0) &&
      (bord[x + 1][y - 1] == 0) &&
      (bord[x][y - 1] != me) &&
      (bord[x + 1][y] != me) &&
      !((bord[x][y - 1] != 0) && (bord[x + 1][y] != 0) &&
      	sList[SMap[stringIDs[x][y-1]]].libC > 2 && 
      	sList[SMap[stringIDs[x+1][y]]].libC > 2) &&
      (markBoard[x + 1][y - 1] != marker)) {
    markBoard[x + 1][y - 1] = marker;
    diags->indx = diags->indx + 1;
    if (diags->indx <= maxSPoint) {
      diags->p[diags->indx].px = x + 1;
      diags->p[diags->indx].py = y - 1;
      if ((bord[x][y - 1] == 0) && (bord[x + 1][y] == 0)) {
        diags->p[0].px = x + 1;
        diags->p[0].py = y - 1;
      }
    }
  }
  if ((x > 0) && (y < maxPoint) &&
      (bord[x - 1][y + 1] == 0) &&
      (bord[x][y + 1] != me) &&
      (bord[x - 1][y] != me) &&
      !((bord[x][y + 1] != 0) && (bord[x - 1][y] != 0) &&
      	sList[SMap[stringIDs[x][y+1]]].libC > 2 && 
      	sList[SMap[stringIDs[x-1][y]]].libC > 2) &&
      (markBoard[x - 1][y + 1] != marker)) {
    markBoard[x - 1][y + 1] = marker;
    diags->indx = diags->indx + 1;
    if (diags->indx <= maxSPoint) {
      diags->p[diags->indx].px = x - 1;
      diags->p[diags->indx].py = y + 1;
      if ((bord[x][y + 1] == 0) && (bord[x - 1][y] == 0)) {
        diags->p[0].px = x - 1;
        diags->p[0].py = y + 1;
      }
    }
  }
  if ((x < maxPoint) && (y < maxPoint) &&
      (bord[x + 1][y + 1] == 0) &&
      (bord[x][y + 1] != me) &&
      (bord[x + 1][y] != me) &&
      !((bord[x][y + 1] != 0) && (bord[x + 1][y] != 0) &&
      	sList[SMap[stringIDs[x][y+1]]].libC > 2 && 
      	sList[SMap[stringIDs[x+1][y]]].libC > 2) &&
      (markBoard[x + 1][y + 1] != marker)) {
    markBoard[x + 1][y + 1] = marker;
    diags->indx = diags->indx + 1;
    if (diags->indx <= maxSPoint) {
      diags->p[diags->indx].px = x + 1;
      diags->p[diags->indx].py = y + 1;
      if ((bord[x][y + 1] == 0) && (bord[x + 1][y] == 0)) {
        diags->p[0].px = x + 1;
        diags->p[0].py = y + 1;
      }
    }
  }
  if ((x > 0) && (bord[x - 1][y] == me) &&
      (markBoard[x - 1][y] != marker))
    LDspan(x - 1, y, me, diags);
  if ((x < maxPoint) && (bord[x + 1][y] == me) &&
      (markBoard[x + 1][y] != marker))
    LDspan(x + 1, y, me, diags);
  if ((y > 0) && (bord[x][y - 1] == me) &&
      (markBoard[x][y - 1] != marker))
    LDspan(x, y - 1, me, diags);
  if ((y < maxPoint) && (bord[x][y + 1] == me) &&
      (markBoard[x][y + 1] != marker))
    LDspan(x, y + 1, me, diags);
}				/* span */

void listDiags(int x, int y, sPointList *diags)
{				/* listDiags */
  int me;
#ifdef DEBUG
  printf("listDiags\n");
#endif
  me = bord[x][y];
  diags->indx = 0;
  marker = marker + 1;
  if (marker == 0) {
    initArray(markBoard);
    marker = 1;
  }
  LDspan(x, y, me, diags);
}				/* listDiags */

int addPlist(sPointList *pList, int x, int y)
{
  if (pList->indx >= maxSPoint)
    return FALSE;
  pList->indx++;
  pList->p[pList->indx].px = x;
  pList->p[pList->indx].py = y;
  return TRUE;
}

int listJump1(int sid, sPointList lList, sPointList *jump1)
{
  int i, counter = 0;
  
  jump1->indx = 0;
  for (i = 1; i <= lList.indx; i++) {
    /* 추가로 돌이 놓이지 않은 경우에만 부르기때문에 myStones등의
       정보를 참조해도 된다 */
    if (myStones[lList.p[i].px][lList.p[i].py] > 1 ||
    	hisStones[lList.p[i].px][lList.p[i].py] > 1)
      continue;
    if (lList.p[i].px != 0 && lList.p[i].px != maxPoint &&
    	bord[lList.p[i].px + 1][lList.p[i].py] == 0 &&
    	SMap[stringIDs[lList.p[i].px - 1][lList.p[i].py]] == sid)
      counter += addPlist(jump1, lList.p[i].px + 1, lList.p[i].py);
    else if (lList.p[i].px != 0 && lList.p[i].px != maxPoint &&
    	bord[lList.p[i].px - 1][lList.p[i].py] == 0 &&
    	SMap[stringIDs[lList.p[i].px + 1][lList.p[i].py]] == sid)
      counter += addPlist(jump1, lList.p[i].px - 1, lList.p[i].py);
    else if (lList.p[i].py != 0 && lList.p[i].py != maxPoint &&
    	bord[lList.p[i].px][lList.p[i].py + 1] == 0 &&
    	SMap[stringIDs[lList.p[i].px][lList.p[i].py - 1]] == sid)
      counter += addPlist(jump1, lList.p[i].px, lList.p[i].py + 1);
    else if (lList.p[i].py != 0 && lList.p[i].py != maxPoint &&
    	bord[lList.p[i].px][lList.p[i].py - 1] == 0 &&
    	SMap[stringIDs[lList.p[i].px][lList.p[i].py + 1]] == sid)
      counter += addPlist(jump1, lList.p[i].px, lList.p[i].py - 1);
  }
  return counter;
}

void intersectPlist(pointList *p1, pointList *p2, pointList *pr)
{				/* intersectPlist */
  int i, j;
#ifdef DEBUG
  printf("intersectPlist\n");
#endif
  marker = marker + 1;
  if (marker == 0) {
    initArray(markBoard);
    marker = 1;
  }
  pr->indx = 0;
  for (i = 1; i <= p1->indx; i++)
    markBoard[p1->p[i].px][p1->p[i].py] = marker;
  j = 0;
  for (i = 1; i <= p2->indx; i++)
    if (markBoard[p2->p[i].px][p2->p[i].py] == marker) {
      j = j + 1;
      pr->p[j] = p2->p[i];
    }
  pr->indx = j;
}				/* intersectPlist */

void old_initArray(intBoard ary)
{				/* initArray */
  int i, j;
  for (i = 0; i <= maxPoint; i++)
    for (j = 0; j <= maxPoint; j++)
      ary[i][j] = 0;
}				/* initArray */

void initArray(intBoard ary)
{
  memcpy(ary, nullBoard, sizeof(intBoard));
}

void initState()
{				/* initState */
  initArray(extra);
  initArray(claim);
  initArray(stringIDs);
  initArray(connectMap);
  initArray(connectMap2);
  initArray(safeConnect);
  initArray(protPoints);
  initArray(ndbord);
  initArray(safeboard);
  initArray(sGroups);
  initArray(powerBoard);
  initArray(valueBoard);
  initArray(powerBoard1);
  initArray(powerBoard2);
  initArray(scoreBoard);
  initArray(GIDMap);
  initArray(wGIDMap);
  initArray(jump1Map);
  initArray(extMap[0]);
  initArray(extMap[1]);
  initArray(extMap[2]);
  initArray(extMap[3]);
}				/* initState */

void old_copyArray(intBoard dest, intBoard src)
{
  int x, y;
  for (y = 0; y <= maxPoint; y++)
    for (x = 0; x <= maxPoint; x++)
      dest[x][y] = src[x][y];
}

void copyArray(intBoard dest, intBoard src)
{
  memcpy(dest, src, sizeof(intBoard));
}

/*
   generates a one-point spread in the force field array (claim)

   the spread from a single point after four calls is:

               1
            2  2  2
         2  4  6  4  2
      2  4  8 10  8  4  2
   1  2  6 10 62 10  6  2  1  
      2  4  8 10  8  4  2
         2  4  6  4  2
            2  2  2
               1

 */
void stake()
{
  int x, y;
  initArray(extra);
  for (y = 0; y <= maxPoint; y++)
    for (x = 0; x <= maxPoint; x++) {
      extra[x][y] = extra[x][y] + claim[x][y];
      if (claim[x][y] > 0) {
	if (x > 0)
	  extra[x - 1][y] += 1;
	if (y > 0)
	  extra[x][y - 1] += 1;
	if (x < maxPoint)
	  extra[x + 1][y] += 1;
	if (y < maxPoint)
	  extra[x][y + 1] += 1;
      } else if (claim[x][y] < 0) {
	if (x > 0)
	  extra[x - 1][y] -= 1;
	if (y > 0)
	  extra[x][y - 1] -= 1;
	if (x < maxPoint)
	  extra[x + 1][y] -= 1;
	if (y < maxPoint)
	  extra[x][y + 1] -= 1;
      }
    }
  copyArray(claim, extra);
}				/* stake */

/*
   sets up claim from the current board position
 */
void spread()
{
  int x, y;
  for (y = 0; y <= maxPoint; y++)
    for (x = 0; x <= maxPoint; x++)
      claim[x][y] = ndbord[x][y] * 50;
  stake();
  stake();
  stake();
  stake();
}				/* spread */

/*
   sList is initialized with the size, loc, and libCount of each string
   stringIDs contains the serial numbers of the strings.
 */
void Resspan(int x, int y, int SID, int *gSize, int *libCount, int who)
{				/* span */
  if ((bord[x][y] == 0) &&
      (markBoard[x][y] != marker)) {	/* a liberty */
    markBoard[x][y] = marker;
    *libCount = *libCount + 1;
  } else if ((bord[x][y] == who) &&
      (stringIDs[x][y] == 0)) {
    stringIDs[x][y] = SID;
    *gSize = *gSize + 1;
    if (x > 0)
      Resspan(x - 1, y, SID, gSize, libCount, who);
    if (x < maxPoint)
      Resspan(x + 1, y, SID, gSize, libCount, who);
    if (y > 0)
      Resspan(x, y - 1, SID, gSize, libCount, who);
    if (y < maxPoint)
      Resspan(x, y + 1, SID, gSize, libCount, who);
  }
}				/* span */

void respreicen()
{				/* respreicen */
  int i, j, SID, libCount, gSize, who;
  SID = 0;
#ifdef DEBUG
  printf("respreicen\n");
#endif
  for (i = 0; i <= maxPoint; i++)
    for (j = 0; j <= maxPoint; j++)
      stringIDs[i][j] = 0;
  for (i = 0; i <= maxPoint; i++)
    for (j = 0; j <= maxPoint; j++)
      if ((bord[i][j] != 0) &&	/* a stone there */
	  (stringIDs[i][j] == 0)) {	/* not seen yet */
	marker = marker + 1;
	if (marker == 0) {
	  initArray(markBoard);
	  marker = 1;
	}
	SID = SID + 1;
	libCount = 0;
	gSize = 0;
	who = bord[i][j];
	Resspan(i, j, SID, &gSize, &libCount, who);	/* span the string, collecting info */
	sList[SID].stringMark = 0;
	sList[SID].atLevel = 0;
	sList[SID].isLive = FALSE;	/* we don't know yet */
	sList[SID].isDead = FALSE;
	sList[SID].isSafe = FALSE;
	sList[SID].numEyes = -1;
	sList[SID].size = gSize;
	sList[SID].libC = libCount;
	sList[SID].lx = i;
	sList[SID].ly = j;
	sList[SID].kx = -1;
	sList[SID].ky = -1;
	sList[SID].sx = -1;
	sList[SID].sy = -1;
	sList[SID].kc = 0;
	sList[SID].sc = 0;
	SMap[SID] = SID;	/* set up identity map */
      }
  maxStringID = SID;
  newSID = SID;
  grpMark = 0;
}				/* respreicen */

/* (x,y)에서 dir방향으로 dist만큼 떨어진 좌표를 (dx,dy)에 넣는다 */
/* 해당 좌표가 판을 벗어나면 FALSE를 되돌린다 */
int get_dxy(int dir, int x, int y, int dist, int *dx, int *dy)
{
  switch (dir) {
  case LEFT:
    if (x < dist)
      return FALSE;
    *dx = x - dist;
    *dy = y;
    break;
  case RIGHT:
    if (x > (maxPoint - dist))
      return FALSE;
    *dx = x + dist;
    *dy = y;
    break;
  case DOWN:
    if (y < dist)
      return FALSE;
    *dx = x;
    *dy = y - dist;
    break;
  case UP:
    if (y > (maxPoint - dist))
      return FALSE;
    *dx = x;
    *dy = y + dist;
    break;
  }
  return TRUE;
}

/* (x,y)에서 dir방향으로 (xdist,ydist)만큼 떨어진 좌표를 (dx,dy)에 넣는다 */
/* 해당 좌표가 판을 벗어나면 FALSE를 되돌린다 */
int get_ddxy(int dir, int x, int y, int xdist, int ydist, int *dx, int *dy)
{
  switch (dir) {
  case LEFT:
    *dx = x - xdist;
    *dy = y - ydist;
    break;
  case RIGHT:
    *dx = x + xdist;
    *dy = y + ydist;
    break;
  case UP:
    *dx = x - ydist;
    *dy = y + xdist;
    break;
  case DOWN:
    *dx = x + ydist;
    *dy = y - xdist;
    break;
  }
  if (*dx < 0 || *dy <0 || *dx > maxPoint || *dy > maxPoint)
    return FALSE;
  else
    return TRUE;
}

void get_cxy(int dir, int x, int y, int *dx, int *dy)
{
  switch (dir) {
  case LEFT:
    *dx = x;
    *dy = y;
    break;
  case RIGHT:
    *dx = maxPoint - x;
    *dy = maxPoint - y;
    break;
  case UP:
    *dx = x;
    *dy = maxPoint - y;
    break;
  case DOWN:
    *dx = maxPoint - x;
    *dy = y;
    break;
  }
}

/*
   play z at [x, y].
   killFlag is set true if anything is killed.
 */
void killString(int x, int y, int me, int him)
{				/* killString */
#ifdef DEBUG
  printf("killString\n");
#endif
  playMark = playMark + 1;
  /* record this kill */
  playStack[playMark].kind = rem;
  playStack[playMark].uval.rem.who = him;
  playStack[playMark].uval.rem.xl = x;
  playStack[playMark].uval.rem.yl = y;
  playStack[playMark].SID = stringIDs[x][y];
  playStack[playMark].uval.rem.sNumber = goboard[x][y].mNum;
  if (showTrees)
    rstone(x, y);
  numCapt = numCapt + 1;
  bord[x][y] = 0;
  stringIDs[x][y] = 0;
  if (x > 0) {
    if (bord[x - 1][y] == me) {
      nlcString.indx = nlcString.indx + 1;
      nlcString.v[nlcString.indx] = SMap[stringIDs[x - 1][y]];
    } else if (bord[x - 1][y] == him)
      killString(x - 1, y, me, him);
  }
  if (x < maxPoint) {
    if (bord[x + 1][y] == me) {
      nlcString.indx = nlcString.indx + 1;
      nlcString.v[nlcString.indx] = SMap[stringIDs[x + 1][y]];
    } else if (bord[x + 1][y] == him)
      killString(x + 1, y, me, him);
  }
  if (y > 0) {
    if (bord[x][y - 1] == me) {
      nlcString.indx = nlcString.indx + 1;
      nlcString.v[nlcString.indx] = SMap[stringIDs[x][y - 1]];
    } else if (bord[x][y - 1] == him)
      killString(x, y - 1, me, him);
  }
  if (y < maxPoint) {
    if (bord[x][y + 1] == me) {
      nlcString.indx = nlcString.indx + 1;
      nlcString.v[nlcString.indx] = SMap[stringIDs[x][y + 1]];
    } else if (bord[x][y + 1] == him)
      killString(x, y + 1, me, him);
  }
}				/* killString */

void mergeString(int sSID, int mySID)
{				/* mergeString */
  int i;
#ifdef DEBUG
  printf("mergeString\n");
#endif
  for (i = 1; i <= newSID; i++)
    if (SMap[i] == sSID) {
      playMark = playMark + 1;
      playStack[playMark].kind = reMap;
      playStack[playMark].SID = i;
      playStack[playMark].uval.reMap.oldSID = sSID;
      SMap[i] = mySID;
    }
}				/* mergeString */

void merge_group(int gid1, int gid2)
{
  int i, j;
  int g1, g2;
  
  if (gid1 > gid2) {
    g1 = gid2;
    g2 = gid1;
  } else {
    g1 = gid1;
    g2 = gid2;
  }

  for (i = 0; i <= maxPoint; i++)
    for (j = 0; j <= maxPoint; j++) {
      if (GIDMap[i][j] == g2)
        GIDMap[i][j] = g1;
    }
  
  gList[g1].strength += gList[g2].strength;
  gList[g1].tr1 += gList[g2].tr1;
  gList[g1].tr2 += gList[g2].tr2;
  gList[g1].score += gList[g2].score;
  gList[g1].eyes += gList[g2].eyes;
  gList[g1].exts += gList[g2].exts;
  gList[g1].stones += gList[g2].stones;

  for (i = 0; i < gList[g2].strings; i++) {
    gList[g1].string[gList[g1].strings] = gList[g2].string[i];
    gList[g1].strings++;
  }
}

void tryPlay(int x, int y, int z)
{				/* plei */
  int i, me, him, mySID;
  int isNew;
#ifdef DEBUG
  printf("tryPlay\n");
#endif
  me = z;
  him = -me;
  killFlag = FALSE;		/* set true if something is killed */
  numCapt = 0;
  tryLevel = tryLevel + 1;
  isNew = FALSE;
  try_counter++;
  bord[x][y] = z;		/* play the stone */
  if ((x > 0) && (bord[x - 1][y] == me))	/* connect to adjacent string */
    mySID = SMap[stringIDs[x - 1][y]];
  else if ((x < maxPoint) && (bord[x + 1][y] == me))
    mySID = SMap[stringIDs[x + 1][y]];
  else if ((y > 0) && (bord[x][y - 1] == me))
    mySID = SMap[stringIDs[x][y - 1]];
  else if ((y < maxPoint) && (bord[x][y + 1] == me))
    mySID = SMap[stringIDs[x][y + 1]];
  else {			/* nobody to connect to */
    newSID = newSID + 1;
    isNew = TRUE;
    mySID = newSID;
    sList[mySID].stringMark = 0;
    sList[mySID].atLevel = tryLevel;
    sList[mySID].isLive = FALSE;
    sList[mySID].numEyes = -1;
    sList[mySID].size = -1;
    sList[mySID].lx = x;
    sList[mySID].ly = y;
    SMap[mySID] = mySID;
  }
  stringIDs[x][y] = mySID;
  playMark = playMark + 1;
  /* record this move */
  playStack[playMark].kind = add;
  playStack[playMark].uval.add.who = me;
  playStack[playMark].uval.add.xl = x;
  playStack[playMark].uval.add.yl = y;
  playStack[playMark].uval.add.lx = last_tx;
  playStack[playMark].uval.add.ly = last_ty;
  playStack[playMark].uval.add.pae = last_try_is_pae;
  last_tx = x;
  last_ty = y;
  playStack[playMark].SID = mySID;
  playStack[playMark].uval.add.sNumber = 0;
  if (isNew)
    playStack[playMark].uval.add.nextSID = newSID - 1;
  else
    playStack[playMark].uval.add.nextSID = newSID;
  if (showTrees)
    sstone(me, x, y, 0);
  /* merge adjacent strings */
  if ((x > 0) && (bord[x - 1][y] == me) &&
      (SMap[stringIDs[x - 1][y]] != mySID))
    mergeString(SMap[stringIDs[x - 1][y]], mySID);
  if ((x < maxPoint) && (bord[x + 1][y] == me) &&
      (SMap[stringIDs[x + 1][y]] != mySID))
    mergeString(SMap[stringIDs[x + 1][y]], mySID);
  if ((y > 0) && (bord[x][y - 1] == me) &&
      (SMap[stringIDs[x][y - 1]] != mySID))
    mergeString(SMap[stringIDs[x][y - 1]], mySID);
  if ((y < maxPoint) && (bord[x][y + 1] == me) &&
      (SMap[stringIDs[x][y + 1]] != mySID))
    mergeString(SMap[stringIDs[x][y + 1]], mySID);
  /* kill opposing strings, listing affected strings */
  nlcString.indx = 1;
  nlcString.v[1] = mySID;	/* init list to include me */
  if ((x > 0) && (bord[x - 1][y] == him) &&
      (sList[SMap[stringIDs[x - 1][y]]].libC == 1)) {
    killFlag = stringIDs[x - 1][y];
    killString(x - 1, y, me, him);
  }
  if ((x < maxPoint) && (bord[x + 1][y] == him) &&
      (sList[SMap[stringIDs[x + 1][y]]].libC == 1)) {
      killFlag = stringIDs[x + 1][y];
    killString(x + 1, y, me, him);
  }
  if ((y > 0) && (bord[x][y - 1] == him) &&
      (sList[SMap[stringIDs[x][y - 1]]].libC == 1)) {
      killFlag = stringIDs[x][y - 1];
    killString(x, y - 1, me, him);
  }
  if ((y < maxPoint) && (bord[x][y + 1] == him) &&
      (sList[SMap[stringIDs[x][y + 1]]].libC == 1)) {
      killFlag = stringIDs[x][y + 1];
    killString(x, y + 1, me, him);
  }
  if (isNew && numCapt == 1)
    last_try_is_pae = TRUE;
  else
    last_try_is_pae = FALSE;
  /* list strings adjacent to me */
  if ((x > 0) && (bord[x - 1][y] == him)) {
    nlcString.indx = nlcString.indx + 1;
    nlcString.v[nlcString.indx] = SMap[stringIDs[x - 1][y]];
  }
  if ((x < maxPoint) && (bord[x + 1][y] == him)) {
    nlcString.indx = nlcString.indx + 1;
    nlcString.v[nlcString.indx] = SMap[stringIDs[x + 1][y]];
  }
  if ((y > 0) && (bord[x][y - 1] == him)) {
    nlcString.indx = nlcString.indx + 1;
    nlcString.v[nlcString.indx] = SMap[stringIDs[x][y - 1]];
  }
  if ((y < maxPoint) && (bord[x][y + 1] == him)) {
    nlcString.indx = nlcString.indx + 1;
    nlcString.v[nlcString.indx] = SMap[stringIDs[x][y + 1]];
  }
  /* fix liberty count for affected strings */
  grpMark = grpMark + 1;
  for (i = 1; i <= nlcString.indx; i++)
    if (sList[nlcString.v[i]].stringMark != grpMark) {
      if (sList[nlcString.v[i]].atLevel != tryLevel) {
	playMark = playMark + 1;
	playStack[playMark].kind = chLib;
	playStack[playMark].SID = nlcString.v[i];
	playStack[playMark].uval.chLib.oldLevel =
	    sList[nlcString.v[i]].atLevel;
	playStack[playMark].uval.chLib.oldLC =
	    sList[nlcString.v[i]].libC;
      }
      sList[nlcString.v[i]].stringMark = grpMark;
      sList[nlcString.v[i]].atLevel = tryLevel;
      spanString(sList[nlcString.v[i]].lx, sList[nlcString.v[i]].ly, &pPlist);
      sList[nlcString.v[i]].libC = pPlist.indx;
    }
}				/* plei */

void saveState()
{				/* saveState */
  playMark = 0;
  tryLevel = 0;
  newSID = maxStringID;
}				/* saveState */

/*
   undoes a move sequence back to uMark
 */
void undoTo(int uMark)
{				/* undoTo */
  int i, xl, yl;
#ifdef DEBUG
  printf("undoTo\n");
#endif
  for (i = playMark; i >= uMark + 1; i--)
    if (playStack[i].kind == rem) {
      xl = playStack[i].uval.rem.xl;
      yl = playStack[i].uval.rem.yl;
      bord[xl][yl] = playStack[i].uval.rem.who;
      stringIDs[xl][yl] = playStack[i].SID;
      if (showTrees)
	sstone(playStack[i].uval.rem.who, xl, yl,
	    playStack[i].uval.rem.sNumber);
    } else if (playStack[i].kind == add) {
      xl = playStack[i].uval.add.xl;
      yl = playStack[i].uval.add.yl;
      last_tx = playStack[i].uval.add.lx;
      last_ty = playStack[i].uval.add.ly;
      last_try_is_pae = playStack[i].uval.add.pae;
      bord[xl][yl] = 0;
      stringIDs[xl][yl] = 0;
      tryLevel = tryLevel - 1;
      newSID = playStack[i].uval.add.nextSID;
      if (showTrees)
	rstone(xl, yl);
    } else if (playStack[i].kind == reMap)
      SMap[playStack[i].SID] = playStack[i].uval.reMap.oldSID;
    else {			/* change libs of string - SID is pre-mapped */
      sList[playStack[i].SID].libC = playStack[i].uval.chLib.oldLC;
      sList[playStack[i].SID].atLevel = playStack[i].uval.chLib.oldLevel;
    }
  playMark = uMark;
}				/* undoTo */

void showStack(void)
{
}

/*
   restores the state of the world after trying a move sequence
 */
void restoreState()
{				/* restoreState */
#ifdef DEBUG
  printf("restoreState\n");
#endif
  if (playMark > 0) {
    undoTo(0);
    playMark = 0;
    tryLevel = 0;
    last_try_is_pae = FALSE;
  }
}				/* restoreState */


/*
   marks unsavable strings as dead
 */
void markDead()
{				/* markDead */
  int i, j, gx, gy, last_tc, check_list[30], check_counter = 0, asize;
  intList aList;
  
#ifdef DEBUG
  printf("markDead\n");
#endif
  try_counter = 0;
  for (i = 1; i <= maxStringID; i++) {
    last_tc = try_counter;
    if (killable(sList[i].lx, sList[i].ly, &gx, &gy)) {
      sList[i].isSafe = FALSE;
      sList[i].kx = gx;
      sList[i].ky = gy;
      sList[i].kc = try_counter - last_tc;
      last_tc = try_counter;
      if (saveable(sList[i].lx, sList[i].ly, &gx, &gy)) {
        sList[i].sx = gx;
        sList[i].sy = gy;
        sList[i].sc = try_counter - last_tc;
        sList[i].isDead = FALSE;
      } else {
        sList[i].isDead = TRUE;
        sList[i].sc = try_counter - last_tc;
      }
    } else {
      sList[i].isSafe = TRUE;
      sList[i].kc = try_counter - last_tc;
    }
  }
  /* 죽은 돌과 죽은돌이 붙어 있는 경우를 찾아 고친다 */
  /* 또 죽은돌과 잡을 수 있는 돌이 붙어 있는 경우도 찾는다. */
  /* 제대로 한다면 killable()에서 이런경우가 생기지 않도록 해야한다. */
  for (i = 1; i <= maxStringID; i++) {
    if (sList[i].isDead) {
      listAdjacents(sList[i].lx, sList[i].ly, &aList);
      asize = 0;
      for (j = 1; j <= aList.indx; j++) {
        if (!sList[aList.v[j]].isSafe && sList[aList.v[j]].size > asize) {
          tryPlay(sList[aList.v[j]].kx, sList[aList.v[j]].ky,
          	bord[sList[i].lx][sList[i].ly]);
          if (!killable(sList[i].lx, sList[i].ly, &gx, &gy)) {
	    check_list[check_counter] = i;
	    check_counter++;
	    sList[i].sx = sList[aList.v[j]].kx;
	    sList[i].sy = sList[aList.v[j]].ky;
	    asize = sList[aList.v[j]].size;
	  }
	  restoreState();
        }
      }
    }
  }
  /* 위에서 죽은 돌중 살릴 수 있는것을 다시 죽지 않은돌로 표시 */
  for (i = 0; i < check_counter; i++) {
    sList[check_list[i]].isDead = FALSE;
/*    j = check_list[i];
    if (sList[j].sx >= 0 && sList[j].sy >= 0) {
      tryPlay(sList[j].sx, sList[j].sy, bord[sList[j].lx][sList[j].ly]);
      if (!killable(sList[j].lx, sList[j].ly, &gx, &gy))
        sList[j].isDead = FALSE;
    } else {
      fprintf(stderr, "ERROR: markDead() %d K:%d,%d, S:%d,%d\n",
      	j, sList[j].kx, sList[j].ky, sList[j].sx, sList[j].sy);
    } */
  }

  /* 잡힐 수 있는 돌 옆에 죽은 돌이 있는경우 
     잡힐 수 있는돌이 잡힐경우 잡은 돌을 다시 잡을 수 있는 것으로 보아
     안전한 돌로 간주한다 */
/*  for (i = 1; i <= maxStringID; i++) {
    if (!sList[i].isSafe) {
      listAdjacents(sList[i].lx, sList[i].ly, &aList);
      for (j = 1; j <= aList.indx; j++) {
        if (sList[aList.v[j]].isDead) {
          sList[i].isSafe = TRUE;
        }
      }
    }
  } */
  
  for (i = 0; i <= maxPoint; i++)
    for (j = 0; j <= maxPoint; j++)
      if (bord[i][j] == 0) {
	ndbord[i][j] = 0;
	safeboard[i][j] = 0;
      } else if (sList[stringIDs[i][j]].isDead) {
	ndbord[i][j] = 0;
      } else if (!sList[stringIDs[i][j]].isSafe) {
        safeboard[i][j] = 0;
        ndbord[i][j] = bord[i][j];
      } else {
	ndbord[i][j] = bord[i][j];
	safeboard[i][j] = bord[i][j];
      }
}				/* markDead */

/*
   marks strings with two eyes as live
 */
void MLspan(int x, int y, int *saw1, int *sawm1, int *size, int sMark)
{				/* span */
  if (ndbord[x][y] == 1)
    *saw1 = TRUE;
  else if (ndbord[x][y] == -1)
    *sawm1 = TRUE;
  else if (sGroups[x][y] == 0) {
    sGroups[x][y] = sMark;
    *size = *size + 1;
    if (x > 0)
      MLspan(x - 1, y, saw1, sawm1, size, sMark);
    if (x < maxPoint)
      MLspan(x + 1, y, saw1, sawm1, size, sMark);
    if (y > 0)
      MLspan(x, y - 1, saw1, sawm1, size, sMark);
    if (y < maxPoint)
      MLspan(x, y + 1, saw1, sawm1, size, sMark);
  }
}				/* span */

int CLspan(int x, int y, int *numEyes, int who)
{				/* span */
  markBoard[x][y] = marker;
  if (ndbord[x][y] == 0) {
    if ((sgList[sGroups[x][y]].sm != marker) &&
	(sgList[sGroups[x][y]].w == who)) {
      sgList[sGroups[x][y]].sm = marker;
      if (sgList[sGroups[x][y]].s > 6)
	return TRUE;
      *numEyes = *numEyes + 1;
      if (*numEyes > 1)
	return TRUE;
    }
  } else if (bord[x][y] == who) {
    if ((x > 0) &&
	(markBoard[x - 1][y] != marker))
      if (CLspan(x - 1, y, numEyes, who))
	return TRUE;
    if ((x < maxPoint) &&
	(markBoard[x + 1][y] != marker))
      if (CLspan(x + 1, y, numEyes, who))
	return TRUE;
    if ((y > 0) &&
	(markBoard[x][y - 1] != marker))
      if (CLspan(x, y - 1, numEyes, who))
	return TRUE;
    if ((y < maxPoint) &&
	(markBoard[x][y + 1] != marker))
      if (CLspan(x, y + 1, numEyes, who))
	return TRUE;
  }
  return FALSE;
}				/* span */

int checkLive(int x, int y)
{				/* checkLive */
  int numEyes, who;
#ifdef DEBUG
  printf("checkLive\n");
#endif
  numEyes = 0;
  who = bord[x][y];
  marker = marker + 1;
  return CLspan(x, y, &numEyes, who);
}				/* checkLive */

void markLive()
{				/* markLive */
  int i, j, size, sMark = 0;
  int saw1, sawm1;
#ifdef DEBUG
  printf("markLive\n");
#endif
  initArray(sGroups);
  for (i = 0; i <= maxPoint; i++)
    for (j = 0; j <= maxPoint; j++)
      if ((sGroups[i][j] == 0) &&
	  (ndbord[i][j] == 0)) {
	size = 0;
	sMark = sMark + 1;
	sawm1 = FALSE;
	saw1 = FALSE;
	MLspan(i, j, &saw1, &sawm1, &size, sMark);
	sgList[sMark].s = size;
	sgList[sMark].sm = 0;
	if (sawm1)
	  if (saw1)
	    sgList[sMark].w = 0;
	  else
	    sgList[sMark].w = -1;
	else if (saw1)
	  sgList[sMark].w = 1;
	else
	  sgList[sMark].w = 0;
      }
  for (i = 1; i <= maxStringID; i++)
    if (sList[i].isSafe)
      sList[i].isLive = checkLive(sList[i].lx, sList[i].ly);
}				/* markLive */

/*
   generates the connection map and the protected point map.
 */
void genConnects()
{				/* genConnects */
  int x, y, numStones;
#ifdef DEBUG
  printf("genConnects\n");
#endif
  initArray(connectMap);
  initArray(connectMap2);
  initArray(protPoints);
  for (x = 0; x <= maxPoint; x++)
    for (y = 0; y <= maxPoint; y++)
      if (bord[x][y] == 1) {	/* map connections to this stone */
	if (x > 0)		/* direct connection */
	  connectMap[x - 1][y] += 1;
	if (x < maxPoint)
	  connectMap[x + 1][y] += 1;
	if (y > 0)
	  connectMap[x][y - 1] += 1;
	if (y < maxPoint)
	  connectMap[x][y + 1] += 1;
	if ((x > 0) && (y > 0) &&	/* diagonal connection */
	    (bord[x - 1][y] == 0) && (bord[x][y - 1] == 0))
	  connectMap[x - 1][y - 1] += 1;
	if ((x < maxPoint) && (y > 0) &&
	    (bord[x + 1][y] == 0) && (bord[x][y - 1] == 0))
	  connectMap[x + 1][y - 1] += 1;
	if ((x < maxPoint) && (y < maxPoint) &&
	    (bord[x + 1][y] == 0) && (bord[x][y + 1] == 0))
	  connectMap[x + 1][y + 1] += 1;
	if ((x > 0) && (y < maxPoint) &&
	    (bord[x - 1][y] == 0) && (bord[x][y + 1] == 0))
	  connectMap[x - 1][y + 1] += 1;
	if ((x > 1) && (claim[x - 1][y] > 3))	/* one point jump */
	  connectMap[x - 2][y] += 1;
	if ((x < (maxPoint - 1)) && (claim[x + 1][y] > 3))
	  connectMap[x + 2][y] += 1;
	if ((y > 1) && (claim[x][y - 1] > 3))
	  connectMap[x][y - 2] += 1;
	if ((y < (maxPoint - 1)) && (claim[x][y + 1] > 3))
	  connectMap[x][y + 2] += 1;
	if ((x > 1) && (y > 0) &&	/* knight's move */
	    (claim[x - 1][y] > 3) && (claim[x - 1][y - 1] > 3))
	  connectMap[x - 2][y - 1] += 1;
	if ((x > 0) && (y > 1) &&
	    (claim[x][y - 1] > 3) && (claim[x - 1][y - 1] > 3))
	  connectMap[x - 1][y - 2] += 1;
	if ((x < (maxPoint - 1)) && (y > 0) &&
	    (claim[x + 1][y] > 3) && (claim[x + 1][y - 1] > 3))
	  connectMap[x + 2][y - 1] += 1;
	if ((x < maxPoint) && (y > 1) &&
	    (claim[x][y - 1] > 3) && (claim[x + 1][y - 1] > 3))
	  connectMap[x + 1][y - 2] += 1;
	if ((x > 1) && (y < maxPoint) &&
	    (claim[x - 1][y] > 3) && (claim[x - 1][y + 1] > 3))
	  connectMap[x - 2][y + 1] += 1;
	if ((x > 0) && (y < (maxPoint - 1)) &&
	    (claim[x][y + 1] > 3) && (claim[x - 1][y + 1] > 3))
	  connectMap[x - 1][y + 2] += 1;
	if ((x < (maxPoint - 1)) && (y < maxPoint) &&
	    (claim[x + 1][y] > 3) && (claim[x + 1][y + 1] > 3))
	  connectMap[x + 2][y + 1] += 1;
	if ((x < maxPoint) && (y < (maxPoint - 1)) &&
	    (claim[x][y + 1] > 3) && (claim[x + 1][y + 1] > 3))
	  connectMap[x + 1][y + 2] += 1;
      } else if (bord[x][y] == -1) {	/* 상대 돌에 대한 연결 상태 */
	if (x > 0)		/* direct connection */
	  connectMap2[x - 1][y] += 1;
	if (x < maxPoint)
	  connectMap2[x + 1][y] += 1;
	if (y > 0)
	  connectMap2[x][y - 1] += 1;
	if (y < maxPoint)
	  connectMap2[x][y + 1] += 1;
	if ((x > 0) && (y > 0) &&	/* diagonal connection */
	    (bord[x - 1][y] == 0) && (bord[x][y - 1] == 0))
	  connectMap2[x - 1][y - 1] += 1;
	if ((x < maxPoint) && (y > 0) &&
	    (bord[x + 1][y] == 0) && (bord[x][y - 1] == 0))
	  connectMap2[x + 1][y - 1] += 1;
	if ((x < maxPoint) && (y < maxPoint) &&
	    (bord[x + 1][y] == 0) && (bord[x][y + 1] == 0))
	  connectMap2[x + 1][y + 1] += 1;
	if ((x > 0) && (y < maxPoint) &&
	    (bord[x - 1][y] == 0) && (bord[x][y + 1] == 0))
	  connectMap2[x - 1][y + 1] += 1;
	if ((x > 1) && (claim[x - 1][y] < -3))	/* one point jump */
	  connectMap2[x - 2][y] += 1;
	if ((x < (maxPoint - 1)) && (claim[x + 1][y] < -3))
	  connectMap2[x + 2][y] += 1;
	if ((y > 1) && (claim[x][y - 1] < -3))
	  connectMap2[x][y - 2] += 1;
	if ((y < (maxPoint - 1)) && (claim[x][y + 1] < -3))
	  connectMap2[x][y + 2] += 1;
	if ((x > 1) && (y > 0) &&	/* knight's move */
	    (claim[x - 1][y] < -3) && (claim[x - 1][y - 1] < -3))
	  connectMap2[x - 2][y - 1] += 1;
	if ((x > 0) && (y > 1) &&
	    (claim[x][y - 1] < -3) && (claim[x - 1][y - 1] < -3))
	  connectMap2[x - 1][y - 2] += 1;
	if ((x < (maxPoint - 1)) && (y > 0) &&
	    (claim[x + 1][y] < -3) && (claim[x + 1][y - 1] < -3))
	  connectMap2[x + 2][y - 1] += 1;
	if ((x < maxPoint) && (y > 1) &&
	    (claim[x][y - 1] < -3) && (claim[x + 1][y - 1] < -3))
	  connectMap2[x + 1][y - 2] += 1;
	if ((x > 1) && (y < maxPoint) &&
	    (claim[x - 1][y] < -3) && (claim[x - 1][y + 1] < -3))
	  connectMap2[x - 2][y + 1] += 1;
	if ((x > 0) && (y < (maxPoint - 1)) &&
	    (claim[x][y + 1] < -3) && (claim[x - 1][y + 1] < -3))
	  connectMap2[x - 1][y + 2] += 1;
	if ((x < (maxPoint - 1)) && (y < maxPoint) &&
	    (claim[x + 1][y] < -3) && (claim[x + 1][y + 1] < -3))
	  connectMap2[x + 2][y + 1] += 1;
	if ((x < maxPoint) && (y < (maxPoint - 1)) &&
	    (claim[x][y + 1] < -3) && (claim[x + 1][y + 1] < -3))
	  connectMap2[x + 1][y + 2] += 1;
      } else if (bord[x][y] == 0) {	/* see if protected point */
	numStones = 0;
	if (x == 0)
	  numStones = numStones + 1;
	if (y == 0)
	  numStones = numStones + 1;
	if (x == maxPoint)
	  numStones = numStones + 1;
	if (y == maxPoint)
	  numStones = numStones + 1;
	if ((x > 0) && (bord[x - 1][y] == 1))
	  numStones = numStones + 1;
	if ((y > 0) && (bord[x][y - 1] == 1))
	  numStones = numStones + 1;
	if ((x < maxPoint) && (bord[x + 1][y] == 1))
	  numStones = numStones + 1;
	if ((y < maxPoint) && (bord[x][y + 1] == 1))
	  numStones = numStones + 1;
	if (numStones == 4)
	  protPoints[x][y] = 1;
	else if (numStones == 3) {
	  if ((x > 0) &&
	      ((bord[x - 1][y] == 0) ||
		  ((bord[x - 1][y] == -1) &&
		      (sList[stringIDs[x - 1][y]].libC == 1))))
	    protPoints[x][y] = 1;
	  else if ((x < maxPoint) &&
		((bord[x + 1][y] == 0) ||
		    ((bord[x + 1][y] == -1) &&
		      (sList[stringIDs[x + 1][y]].libC == 1))))
	    protPoints[x][y] = 1;
	  else if ((y > 0) &&
		((bord[x][y - 1] == 0) ||
		    ((bord[x][y - 1] == -1) &&
		      (sList[stringIDs[x][y - 1]].libC == 1))))
	    protPoints[x][y] = 1;
	  else if ((y < maxPoint) &&
		((bord[x][y + 1] == 0) ||
		    ((bord[x][y + 1] == -1) &&
		      (sList[stringIDs[x][y + 1]].libC == 1))))
	    protPoints[x][y] = 1;
	}
      }
  for (x = 0; x <= maxPoint; x++)
    for (y = 0; y <= maxPoint; y++)
      if (bord[x][y] != 0) {
	connectMap[x][y] = 0;
	connectMap2[x][y] = 0;
	protPoints[x][y] = 0;
      }
}				/* genConnects */

void genConnects2()
{				/* genConnects */
  int x, y;
#ifdef DEBUG
  printf("genConnects\n");
#endif
  initArray(safeConnect);
  for (x = 0; x <= maxPoint; x++)
    for (y = 0; y <= maxPoint; y++)
      if (ndbord[x][y] == 1 && sList[stringIDs[x][y]].isSafe) {	/* map connections to this stone */
	if (x > 0)		/* direct connection */
	  safeConnect[x - 1][y] += 1;
	if (x < maxPoint)
	  safeConnect[x + 1][y] += 1;
	if (y > 0)
	  safeConnect[x][y - 1] += 1;
	if (y < maxPoint)
	  safeConnect[x][y + 1] += 1;
	if ((x > 0) && (y > 0) &&	/* diagonal connection */
	    (bord[x - 1][y] == 0) && (bord[x][y - 1] == 0))
	  safeConnect[x - 1][y - 1] += 1;
	if ((x < maxPoint) && (y > 0) &&
	    (bord[x + 1][y] == 0) && (bord[x][y - 1] == 0))
	  safeConnect[x + 1][y - 1] += 1;
	if ((x < maxPoint) && (y < maxPoint) &&
	    (bord[x + 1][y] == 0) && (bord[x][y + 1] == 0))
	  safeConnect[x + 1][y + 1] += 1;
	if ((x > 0) && (y < maxPoint) &&
	    (bord[x - 1][y] == 0) && (bord[x][y + 1] == 0))
	  safeConnect[x - 1][y + 1] += 1;
	if ((x > 1) && (claim[x - 1][y] > 3))	/* one point jump */
	  safeConnect[x - 2][y] += 1;
	if ((x < (maxPoint - 1)) && (claim[x + 1][y] > 3))
	  safeConnect[x + 2][y] += 1;
	if ((y > 1) && (claim[x][y - 1] > 3))
	  safeConnect[x][y - 2] += 1;
	if ((y < (maxPoint - 1)) && (claim[x][y + 1] > 3))
	  safeConnect[x][y + 2] += 1;
	if ((x > 1) && (y > 0) &&	/* knight's move */
	    (claim[x - 1][y] > 3) && (claim[x - 1][y - 1] > 3))
	  safeConnect[x - 2][y - 1] += 1;
	if ((x > 0) && (y > 1) &&
	    (claim[x][y - 1] > 3) && (claim[x - 1][y - 1] > 3))
	  safeConnect[x - 1][y - 2] += 1;
	if ((x < (maxPoint - 1)) && (y > 0) &&
	    (claim[x + 1][y] > 3) && (claim[x + 1][y - 1] > 3))
	  safeConnect[x + 2][y - 1] += 1;
	if ((x < maxPoint) && (y > 1) &&
	    (claim[x][y - 1] > 3) && (claim[x + 1][y - 1] > 3))
	  safeConnect[x + 1][y - 2] += 1;
	if ((x > 1) && (y < maxPoint) &&
	    (claim[x - 1][y] > 3) && (claim[x - 1][y + 1] > 3))
	  safeConnect[x - 2][y + 1] += 1;
	if ((x > 0) && (y < (maxPoint - 1)) &&
	    (claim[x][y + 1] > 3) && (claim[x - 1][y + 1] > 3))
	  safeConnect[x - 1][y + 2] += 1;
	if ((x < (maxPoint - 1)) && (y < maxPoint) &&
	    (claim[x + 1][y] > 3) && (claim[x + 1][y + 1] > 3))
	  safeConnect[x + 2][y + 1] += 1;
	if ((x < maxPoint) && (y < (maxPoint - 1)) &&
	    (claim[x][y + 1] > 3) && (claim[x + 1][y + 1] > 3))
	  safeConnect[x + 1][y + 2] += 1;
      }

  for (x = 0; x <= maxPoint; x++)
    for (y = 0; y <= maxPoint; y++)
      if (bord[x][y] != 0) {
	safeConnect[x][y] = 0;
      }
}				/* genConnects */


/*
   0  0  0  0  1  0  0  0  0
   0  0  0  2  2  2  0  0  0
   0  0  2  4  6  4  2  0  0
   0  2  4  8 10  8  4  2  0
   1  2  6 10 62 10  6  2  1  
   0  2  4  8 10  8  4  2  0
   0  0  2  4  6  4  2  0  0
   0  0  0  2  2  2  0  0  0
   0  0  0  0  1  0  0  0  0
*/
/* int pp[7][7] = {
  {0, 0, 2, 2, 2, 0, 0},
  {0, 2, 4, 6, 4, 2, 0},
  {2, 4, 8, 10, 8, 4, 2},
  {2, 6, 10, 62, 10, 6, 2},
  {2, 4, 8, 10, 8, 4, 2},
  {0, 2, 4, 6, 4, 2, 0},
  {0, 0, 2, 2, 2, 0, 0}}; */

int pp[7][7] = {
  {0, 0, 1, 1, 1, 0, 0},
  {0, 1, 2, 3, 2, 1, 0},
  {1, 2, 4, 5, 4, 2, 1},
  {1, 3, 5,50, 5, 3, 1},
  {1, 2, 4, 5, 4, 2, 1},
  {0, 1, 2, 3, 2, 1, 0},
  {0, 0, 1, 1, 1, 0, 0}};

int get_power(int x, int y)
{
  int i = 0, j, jj = 0, mx = 7, my = 7, dx, dy, dp, power;

  if (x < 3)
    i = 3 - x;
  if (y < 3)
    jj = 3 - y;

  if (x > 15)
    mx = 7 - (x - 15);
  if (y > 15)
    my = 7 - (y - 15);

  power = 0;
  for (; i < mx; i++) {
    dx = x - 3 + i;
    for (j = jj; j < my; j++) {
      dy = y - 3 + j;
      if (pp[i][j] > 0) {
	if (claim[dx][dy] == 0) {
	  power++;
	} else if (claim[dx][dy] < 0) {
	  dp = pp[i][j] + claim[dx][dy];
	  if (dp == 0)
	    power++;
	  else if (dp > 0)
	    power += 2;
	}
      }
    }
  }
  return power;
}

int get_power1(int x, int y)
{
  int i = 0, j, jj = 0, mx = 7, my = 7, dx, dy, dp, power;

  if (x < 3)
    i = 3 - x;
  if (y < 3)
    jj = 3 - y;

  if (x > 15)
    mx = 7 - (x - 15);
  if (y > 15)
    my = 7 - (y - 15);

  power = 0;
  for (; i < mx; i++) {
    dx = x - 3 + i;
    for (j = jj; j < my; j++) {
      dy = y - 3 + j;
      if (pp[i][j] > 0) {
	if (claim[dx][dy] == 0) {
	  power++;
	} else if (claim[dx][dy] > 0) {
	  dp = pp[i][j] - claim[dx][dy];
	  if (dp == 0)
	    power++;
	  else if (dp > 0)
	    power += 2;
	}
      }
    }
  }
  return power;
}

int get_power2(int x, int y)
{
  int i = 0, j, jj = 0, mx = 7, my = 7, dx, dy, dp, power;

  if (x < 3)
    i = 3 - x;
  if (y < 3)
    jj = 3 - y;

  if (x > 15)
    mx = 7 - (x - 15);
  if (y > 15)
    my = 7 - (y - 15);

  power = 0;
  for (; i < mx; i++) {
    dx = x - 3 + i;
    for (j = jj; j < my; j++) {
      dy = y - 3 + j;
      if (pp[i][j] > 0) {
	if (claim[dx][dy] == 0) {
	  power += 2;
	} else if (claim[dx][dy] < 0) {
	  dp = pp[i][j] + claim[dx][dy];
	  if (dp == 0)
	    power++;
	  else if (dp > 0)
	    power += 2;
	} else {
	  dp = pp[i][j] - claim[dx][dy];
	  if (dp == 0)
	    power++;
	  else if (dp > 0)
	    power += 2;
	}
      }
    }
  }
  power = power / 2;
  return power;
}

void gen_powerboard()
{
  int i, j;
  
  initArray(powerBoard);
  initArray(powerBoard1);
  initArray(powerBoard2);
  
  for (i = 0; i < 19; i++)
    for (j = 0; j < 19; j++)
      if (legal[i][j]) {
        powerBoard[i][j] = get_power(i, j);
        powerBoard1[i][j] = get_power1(i, j);
        powerBoard2[i][j] = (powerBoard[i][j] + powerBoard1[i][j]) / 2;
/*        powerBoard2[i][j] = get_power2(i, j); */
      }
}

int get_libs(int x, int y)
{
  int l = 0;

  if (x > 0 && !bord[x - 1][y])
    l++;
  if (x < maxPoint && !bord[x + 1][y])
    l++;
  if (y > 0 && !bord[x][y - 1])
    l++;
  if (y < maxPoint && !bord[x][y + 1])
    l++;
  /* 할일: 돌을 잡을 수 있을 경우 잡은 다음의 활로 숫자 계산 */
  
  return l;
}

void count_libs(int x, int y, int dx, int dy)
{
  switch(bord[dx][dy]) {
  case 0:
    libBoard[x][y]++;
    break;
  case 1:
    myStones[x][y]++;
    break;
  case -1:
    hisStones[x][y]++;
    break;
  }
}


void gen_libboard()
{
  int i, j;
  
  initArray(libBoard);
  initArray(myStones);
  initArray(hisStones);
  
  for (i = 0; i < 19; i++)
    for (j = 0; j < 19; j++) 
      if (bord[i][j] == 0) {
        if (i > 0)
          count_libs(i, j, i - 1, j);
        if (i < maxPoint)
          count_libs(i, j, i + 1, j);
        if (j > 1)
          count_libs(i, j, i, j - 1);
        if (j < maxPoint)
          count_libs(i, j, i, j + 1);
      }
}


void gen_scoreboard()
{
  int x, y, ml, l, my_score = 0, his_score = 0;
  extern int current_score[], Prisoners[];

  initArray(scoreBoard);

  for (x = 0; x < 19; x++)
    for (y = 0; y < 19; y++) {
      ml = Maxlibs(x, y);
      l = 0;
      if (x > 0) {
	if (claim[x - 1][y] < 0)
	  l--;
	else if (claim[x - 1][y] > 0)
	  l++;
      }
      if (x < 18) {
	if (claim[x + 1][y] < 0)
	  l--;
	else if (claim[x + 1][y] > 0)
	  l++;
      }
      if (y > 0) {
	if (claim[x][y - 1] < 0)
	  l--;
	else if (claim[x][y - 1] > 0)
	  l++;
      }
      if (y < 18) {
	if (claim[x][y + 1] < 0)
	  l--;
	else if (claim[x][y + 1] > 0)
	  l++;
      }
      if (l == ml || ndbord[x][y] == 1)
	scoreBoard[x][y] = 1;
      else if (l == -ml || ndbord[x][y] == -1)
	scoreBoard[x][y] = -1;
    }

  for (x = 0; x < 19; x++)
    for (y = 0; y < 19; y++)
/*      if (ndbord[x][y] == 0) */{
	ml = Maxlibs(x, y);
	l = 0;
	if (x > 0) {
	  if (scoreBoard[x - 1][y] < 0)
	    l--;
	  else if (scoreBoard[x - 1][y] > 0)
	    l++;
	}
	if (x < 18) {
	  if (scoreBoard[x + 1][y] < 0)
	    l--;
	  else if (scoreBoard[x + 1][y] > 0)
	    l++;
	}
	if (y > 0) {
	  if (scoreBoard[x][y - 1] < 0)
	    l--;
	  else if (scoreBoard[x][y - 1] > 0)
	    l++;
	}
	if (y < 18) {
	  if (scoreBoard[x][y + 1] < 0)
	    l--;
	  else if (scoreBoard[x][y + 1] > 0)
	    l++;
	}
	if (l == ml)
	  scoreBoard[x][y]++;
	else if (l == -ml)
	  scoreBoard[x][y]--;
      }
  for (x = 0; x < 19; x++)
    for (y = 0; y < 19; y++) {
      if (bord[x][y] == 0) {
        if (scoreBoard[x][y] > 0)
          my_score++;
        else if (scoreBoard[x][y] < 0)
          his_score++;
      } else if (ndbord[x][y] == 0) {
        if (scoreBoard[x][y] > 0)
          my_score += 2;
        else if (scoreBoard[x][y] < 0)
          his_score += 2;
      }
    }
  if (thegame.pla == BLACK) {
    current_score[BLACK] = my_score + Prisoners[WHITE];
    current_score[WHITE] = his_score + Prisoners[BLACK];
  } else {
    current_score[BLACK] = his_score + Prisoners[WHITE];
    current_score[WHITE] = my_score + Prisoners[BLACK];
  }
//  intrPrisonerReport(current_score[BLACK], current_score[WHITE]);
}

#define WGSPAN(x, y)	if (GIDMap[x][y] == 0 && \
  ((me == 1 && scoreBoard[x][y] > 0) || (me == -1 && scoreBoard[x][y] < 0))) \
		WGspan(x, y, gid, me)

void WGspan(int x, int y, int gid, int me)
{
  static int d, dx, dy;
  
  GIDMap[x][y] = gid;

  if (me == bord[x][y]) {
    gList[gid].stones++;
    for (d = 0; d < 4; d++)
/* 한칸 뜀이 가능한 곳을 찾는다 */
      if (extMap[d][x][y] > 2) {
        gList[gid].exts++;
        sList[stringIDs[x][y]].exts++;
        get_dxy(d, x, y, 2, &dx, &dy);
        if (jump1Map[dx][dy] == 0)
          jump1Map[dx][dy] = me == 1 ? 1 : 2;
        else if ((jump1Map[dx][dy] == 1 && me == -1) ||
        	(jump1Map[dx][dy] == 2 && me == 1))
          jump1Map[dx][dy] = 3;
      }
    if (sList[SMap[stringIDs[x][y]]].stringMark == 0) {
      sList[SMap[stringIDs[x][y]]].stringMark = 1;
      gList[gid].string[gList[gid].strings] = stringIDs[x][y];
      gList[gid].strings++;
    }
  } else if (bord[x][y] != 0)
    gList[gid].score++;
  else {
    switch(abs(scoreBoard[x][y])) {
    case 1:
      gList[gid].tr1++;
      break;
    case 2:
      gList[gid].tr2++;
      break;
    }
    if (vsgList[vEyeMap[x][y]].mark == 0) {
      vsgList[vEyeMap[x][y]].mark = 1;
      gList[gid].eyes += vsgList[vEyeMap[x][y]].eyes;
    }
  }

  if (x > 0)
    WGSPAN(x - 1, y);
  if (y > 0)
    WGSPAN(x, y - 1);
  if (x < maxPoint)
    WGSPAN(x + 1, y);
  if (y < maxPoint)
    WGSPAN(x, y + 1);
}

#define WGSPAN2(x, y)	if (wGIDMap[x][y] == 0 && \
	((me == 1 && claim[x][y] > 0) || (me == -1 && claim[x][y] < 0))) \
		WGspan2(x, y, gid, me)

void WGspan2(int x, int y, int gid, int me)
{
  static int d;
  wGIDMap[x][y] = gid;

  if (me == bord[x][y]) {
    wgList[gid].stones++;
    for (d = 0; d < 4; d++)
      if (extMap[d][x][y] > 2)
        wgList[gid].exts++;
    if (sList[SMap[stringIDs[x][y]]].stringMark == 0) {
      sList[SMap[stringIDs[x][y]]].stringMark = 1;
      wgList[gid].string[wgList[gid].strings] = stringIDs[x][y];
      wgList[gid].strings++;
    }
  } else if (bord[x][y] != 0)
    wgList[gid].score++;
  else {
    switch(abs(scoreBoard[x][y])) {
    case 1:
      wgList[gid].tr1++;
      break;
    case 2:
      wgList[gid].tr2++;
      break;
    }
    if (vsgList[vEyeMap[x][y]].mark == 0) {
      vsgList[vEyeMap[x][y]].mark = 1;
      wgList[gid].eyes += vsgList[vEyeMap[x][y]].eyes;
    }
  }

  if (x > 0)
    WGSPAN2(x - 1, y);
  if (y > 0)
    WGSPAN2(x, y - 1);
  if (x < maxPoint)
    WGSPAN2(x + 1, y);
  if (y < maxPoint)
    WGSPAN2(x, y + 1);
}

void init_gr(groupRec *gr)
{
/*  static groupRec null_gr = {0, 0, 0, 0, 0, 0, 0, 0, 
  	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};

  memcpy(&gr, null_gr, sizeof(groupRec)); */

  gr->strength = 0;
  gr->tr1 = 0;
  gr->tr2 = 0;
  gr->score = 0;
  gr->eyes = 0;
  gr->exts = 0;
  gr->stones = 0;
  gr->strings = 0;
}

void gen_wGIDMap(void)
{
  int i, j, gid = 0, gid2 = 0, me;

  initArray(GIDMap);
  initArray(wGIDMap);

  for (i = 1; i <= max_veid; i++)
    vsgList[i].mark = 0;
  for (i = 1; i <= maxStringID; i++)
    sList[i].stringMark = 0;
    
  for (i = 0; i < board_size; i++)
    for (j = 0; j < board_size; j++) {
      if (ndbord[i][j] != 0 && GIDMap[i][j] == 0) {
        gid++;
	gList[gid].tr1 = 0;
	gList[gid].tr2 = 0;
	gList[gid].score = 0;
	gList[gid].eyes = 0;
	gList[gid].exts = 0;
	gList[gid].stones = 0;
	gList[gid].strings = 0;
        me = scoreBoard[i][j] > 0 ? 1 : -1;
        WGspan(i, j, gid, me);
/*        gList[gid].score += gList[gid].tr2; */
      }
    }

  for (i = 1; i <= max_veid; i++)
    vsgList[i].mark = 0;
  for (i = 1; i <= maxStringID; i++)
    sList[i].stringMark = 0;
    
  for (i = 0; i < board_size; i++)
    for (j = 0; j < board_size; j++) {
      if (ndbord[i][j] != 0 && wGIDMap[i][j] == 0) {
        gid2++;
	wgList[gid2].tr1 = 0;
	wgList[gid2].tr2 = 0;
	wgList[gid2].score = 0;
	wgList[gid2].eyes = 0;
	wgList[gid2].exts = 0;
	wgList[gid2].stones = 0;
	wgList[gid2].strings = 0;
        me = claim[i][j] > 0 ? 1 : -1;
        WGspan2(i, j, gid2, me);
      }
    }

}

#define VSSPAN(x, y)	if (vEyeMap[x][y] == 0 && ndbord[x][y] == 0 && \
	((me == scoreBoard[x][y]) || (me == scoreBoard[x][y]))) \
		VSspan(x, y, veid, me)

void VSspan(int x, int y, int veid, int me)
{
  vEyeMap[x][y] = veid;
  
  vsgList[veid].score++;
  if (x < vsgList[veid].min_x)
    vsgList[veid].min_x = x;
  if (x > vsgList[veid].max_x)
    vsgList[veid].max_x = x;
  if (y < vsgList[veid].min_y)
    vsgList[veid].min_y = y;
  if (y > vsgList[veid].max_y)
    vsgList[veid].max_y = y;
  
  if (x > 0)
    VSSPAN(x - 1, y);
  if (y > 0)
    VSSPAN(x, y - 1);
  if (x < maxPoint)
    VSSPAN(x + 1, y);
  if (y < maxPoint)
    VSSPAN(x, y + 1);
}

int count_eyes(int veid)
{
  int owner;
  
/* eyes에 눈의 갯수*2를 저장 */
/* 현재는 대략의 계산이다 */
  if (vsgList[veid].score > 10)
    vsgList[veid].eyes = 4;
  else if (vsgList[veid].score > 5) {
    if ((vsgList[veid].max_x - vsgList[veid].min_x > 4) ||
	(vsgList[veid].max_y - vsgList[veid].min_y > 4))
      vsgList[veid].eyes = 4;
    else
      vsgList[veid].eyes = 3;
  } else if (vsgList[veid].score > 3)
    vsgList[veid].eyes = 2;
  else
    vsgList[veid].eyes = 1;

  owner = scoreBoard[vsgList[veid].lx][vsgList[veid].ly] / 2;

#ifdef DEBUG
  int i, j, inner;
  char ch;
  for (j = vsgList[veid].max_y + 1; j >= vsgList[veid].min_y - 1; j--) {
    for (i = vsgList[veid].min_x - 1; i <= vsgList[veid].max_x + 1; i++) {
      if (vEyeMap[i][j] == veid)
        inner = TRUE;
      else
        inner = FALSE;
      if (i < 0 || j < 0 || i > maxPoint || j > maxPoint)
        ch = '+';
      else if (bord[i][j] == 0)
        ch = inner ? '.' : scoreBoard[i][j] == owner ? '1' : 
        	scoreBoard[i][j] == owner * 2 ? '2' : ' ';
      else if (bord[i][j] == owner)
        ch = 'O';
      else
        ch = inner ? 'X' : 'x';
//      printf("%c", ch);
    }
//    printf("\n");
  }
//  printf ("EYE ID:%d size:%dx%d eyes:%d\n", veid, 
  	vsgList[veid].max_x - vsgList[veid].min_x + 1,
  	vsgList[veid].max_y - vsgList[veid].min_y + 1,
  	vsgList[veid].eyes);
#endif

  return vsgList[veid].eyes;
}

void gen_vEyeMap()
{
  int i, j, veid = 0, me;

  initArray(vEyeMap);
  
  for (i = 0; i < board_size; i++)
    for (j = 0; j < board_size; j++) {
      if (abs(scoreBoard[i][j]) == 2 && ndbord[i][j] == 0 &&
      		vEyeMap[i][j] == 0) {
        veid++;
/*        vsgList[veid].mark = 0; */
        vsgList[veid].score = 0;
        vsgList[veid].min_x = maxPoint;
        vsgList[veid].max_x = 0;
        vsgList[veid].min_y = maxPoint;
        vsgList[veid].max_y = 0;
        vsgList[veid].lx = i;
        vsgList[veid].ly = j;
        me = scoreBoard[i][j];
        VSspan(i, j, veid, me);
        count_eyes(veid);
      }
    }
  max_veid = veid;
}

/* 놓여진 돌이 상하좌우 네 방향으로 몇칸을 뻗을 수 있는지를 따져서
   extMap[]에 저장한다 */
void gen_extMap()
{
  int d, i, j, k, color, dx, dy, t1, t2;
  
  for (d = 0; d < 4; d++)
    initArray(extMap[d]);
  
  initArray(jump1Map);
    
  for (i = 0; i <= maxPoint; i++)
    for (j = 0; j <= maxPoint; j++) {
      if ((color = bord[i][j]) != 0) {
        for (d = 0; d < 4; d++) {
          for (k = 0;;k++) {
            if (!get_ddxy(d, i, j, k+1, 0, &dx, &dy) || bord[dx][dy] != 0)
              break;
            if (get_ddxy(d, i, j, k, -1, &dx, &dy))
              t1 = bord[dx][dy];
            else
              t1 = 2;
            if (get_ddxy(d, i, j, k, +1, &dx, &dy))
              t2 = bord[dx][dy];
            else
              t2 = 2;
            if (k == 0) {
              if (t1 == -color || t2 == -color) {
                k++;
                break;
              }
            } else if (t1 != 0 || t2 != 0)
              break;
          }
          extMap[d][i][j] = k;
        }
      }
    }
}

/*
   generates the whole state of the game.
 */
void genState()
{				/* genState */
#ifdef DEBUG
  printf("genState\n");
#endif
  inGenState = TRUE;
  respreicen();
  markDead();
  markLive();
  spread();
  genConnects();
  genConnects2();
  gen_powerboard();
  gen_libboard();
  gen_scoreboard();
  gen_vEyeMap();
  gen_extMap();
  gen_wGIDMap();
#ifdef DEBUG
/*  printBoard( claim, "claim" ); */
/*  printBoard( bord, "bord" ); */
   printBoard( ndbord, "ndbord" );
   printBoard( sGroups, "sGroups" );
   printBoard( stringIDs, "stringIDs" );
   printBoard( connectMap, "connectMap" );
   printBoard( protPoints, "protPoints" );
#endif
  inGenState = FALSE;
  initArray(valueBoard);
  initArray(ignoreSafe);
}				/* genState */

/*
   generates a value for the [x, y] location that appears to get larger
   for points that are saddle points in the influence graph (klein)
 */
int tencen(int x, int y)
{				/* tencen */
  int a, b, c, d, w, z;
#ifdef DEBUG
  printf("tencen\n");
#endif
  if (claim[x][y] > -1) {	/* if (he does not influence this area, return 50 */
    return 50;
  }
  w = claim[x][y];		/* w <= -1 */
  a = iNil;
  if (x > 0)
    if (claim[x - 1][y] > -1)	/* if (neighbor is not influenced by him */
      a = claim[x - 1][y] - w;	/* score is sum of his influence on central */
  b = iNil;			/*  point and my influence on this neighbor */
  if (y > 0)
    if (claim[x][y - 1] > -1)
      b = claim[x][y - 1] - w;
  c = iNil;
  if (x < maxPoint)
    if (claim[x + 1][y] > -1)
      c = claim[x + 1][y] - w;
  d = iNil;
  if (y < maxPoint)
    if (claim[x][y + 1] > -1)
      d = claim[x][y + 1] - w;
  z = a;			/* z = max(a, b, c, d) */
  if (z != iNil) {
    if ((b != iNil) &&
	(b > z))
      z = b;
  } else
    z = b;
  if (z != iNil) {
    if ((c != iNil) &&
	(c > z))
      z = c;
  } else
    z = c;
  if (z != iNil) {
    if ((d != iNil) &&
	(d > z))
      z = d;
  } else
    z = d;
  if ((z != iNil) &&
      ((x == 0) ||
	  (y == 0) ||
	  (x == maxPoint) ||
	  (y == maxPoint)))
    z = z * 2;			/* double z if (on the edge of the board ?? */
  if (z != iNil)
    return z;
  else
    return 50;
}				/* tencen */

void initGPUtils()
{				/* initGPUtils */
#ifdef DEBUG
  printf("initGPUtils\n");
#endif
  initArray(markBoard);
  initState();
  marker = 0;
  playMark = 0;
  sList[0].isLive = FALSE;
  sList[0].isSafe = FALSE;
  sList[0].isDead = FALSE;
  sList[0].libC = 0;
  sList[0].size = 0;
  sList[0].numEyes = 0;
  sList[0].lx = -1;
  sList[0].ly = -1;
  SMap[0] = 0;
  dbStop = FALSE;
  inGenState = FALSE;
}				/* initGPUtils */

char *get_sgfcoord(int x, int y)
{
  static char scc[80];
  
  if (x < 0 || x > 18){}
//    sprintf(scc, "tt");
  else{}
//    sprintf(scc, "%c%c", x + 'a', (18 - y) + 'a');
  
  return scc;
}

/* sgf로 자동 저장 : 예술인 (98. 8. 16) */
/*
void save_current_to_sgf()
{
  extern float dum;
  extern int player_type[], playLevel, max_move, ingame;
  extern pointc move_log[];
  extern char player_name[][80];
  FILE *f;
  int i;
  time_t timep;
  struct tm *lt;
  char fname[80];
  
  timep = time(NULL);
  lt = localtime(&timep);
  
  if (ingame) {
    strcpy(fname, "autosave.sgf");
  } else {
    sprintf(fname, "last_game%02d%02d.sgf", lt->tm_hour, lt->tm_min);
  }
  if (!(f = fopen(fname, "wt"))) {
    perror("SYSERR: save_current_to_sgf()");
    return;
  }
  
  fprintf(f, "(;GM[1]FF[3]SZ[19]KM[%.1f]HA[%d]", dum, thegame.blackHandicap);

  fprintf(f, "DT[%d-%d-%d]\n", lt->tm_year, lt->tm_mon + 1, lt->tm_mday);

  if (player_type[WHITE] == TYPE_BADUKI) {
    fprintf(f, "PW[BadukI-%d]", playLevel);
  } else {
    if (player_name[WHITE] && *player_name)
      fprintf(f, "PW[%s]", player_name[WHITE]);
    else
      fprintf(f, "PW[??]");
  }
  if (player_type[BLACK] == TYPE_BADUKI) {
    fprintf(f, "PB[BadukI-%d]", playLevel);
  } else {
    if (player_name[BLACK] && *player_name)
      fprintf(f, "PB[%s]", player_name[BLACK]);
    else
      fprintf(f, "PB[??]");
  }
  
  for (i = 0; i < max_move; i++) {
    if (move_log[i].color == WHITE)
      fprintf(f, ";W[%s]", get_sgfcoord(move_log[i].px, move_log[i].py));
    else
      fprintf(f, ";B[%s]", get_sgfcoord(move_log[i].px, move_log[i].py));
  }
  fprintf(f, ")\n");
        
  fclose(f);
}
 */

#define CenterPoint	9

/* x1,y1이 높으면 1을 x2,y2가 높으면 -1을 되돌림*/
int height_cmp(int x1, int y1, int x2, int y2)
{
  int dx1, dy1, dx2, dy2, h1, h2;
  
  dx1 = x1 >= CenterPoint ? maxPoint - x1 : x1;
  dx2 = x2 >= CenterPoint ? maxPoint - x2 : x2;
  dy1 = y1 >= CenterPoint ? maxPoint - y1 : y1;
  dy2 = y2 >= CenterPoint ? maxPoint - y2 : y2;
  
  h1 = dx1 > dy1 ? dy1 : dx1;
  h2 = dx2 > dy2 ? dy2 : dx2;
  
  if (h1 > h2)
    return 1;
  else if (h1 < h2)
    return -1;
  else
    return 0;
}

int height_cmp2(int x1, int y1, int x2, int y2)
{
  int dx1, dy1, dx2, dy2, h1, h2, l1, l2;
  
  dx1 = x1 >= CenterPoint ? maxPoint - x1 : x1;
  dx2 = x2 >= CenterPoint ? maxPoint - x2 : x2;
  dy1 = y1 >= CenterPoint ? maxPoint - y1 : y1;
  dy2 = y2 >= CenterPoint ? maxPoint - y2 : y2;
  
  h1 = dx1 > dy1 ? dy1 : dx1;
  h2 = dx2 > dy2 ? dy2 : dx2;
  l1 = dx1 < dy1 ? dy1 : dx1;
  l2 = dx2 < dy2 ? dy2 : dx2;
  
  if (h1 > h2)
    return 1;
  else if (h1 < h2)
    return -1;
  else if (l1 > l2)
    return 1;
  else if (l1 < l2)
    return -1;
  else
    return 0;
}

int get_height(int x, int y)
{
  int dx, dy, h;
  
  dx = x >= CenterPoint ? maxPoint - x : x;
  dy = y >= CenterPoint ? maxPoint - y : y;
  
  h = dx > dy ? dy : dx;
  
  return h;
}
