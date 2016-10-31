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

/* $Id: baduki.h,v 1.16 1999/04/12 16:31:47 artist Exp artist $ */ 

#include "game.h"

#ifndef __baduki_h
#define __baduki_h

/* 기본 정의 */
#ifndef NULL
#define NULL    ((void*) 0)
#endif

#ifndef FALSE
#define FALSE   (0)
#endif

#ifndef TRUE
#define TRUE    (!FALSE)
#endif
/*
#define BLACK	0
#define	WHITE	1
#define EMPTY	2
*/

#define TT_EMPTY	0
#define TT_MINE		1
#define TT_HIS		2
#define TT_EDGE		3

#define LEFT		0
#define RIGHT		1
#define UP		2
#define DOWN		3

#define INTERSECTIONMSG 1	/* User buttoned an intersection */
#define QUITMSG 2		/* User buttoned QUIT icon */
#define PLAYMSG 3
#define RESTARTMSG 4
#define PASSMSG 5

#define MAXSTRINGS 100

#define PLACED 0
#define REMOVED 1

/* 화면 표시 */
#define DMODE_PLAY	0
#define DMODE_SCORE	1
#define DMODE_GROUP	2
#define DMODE_CLAIM	3
#define DMODE_BORD	4
#define DMODE_NDBOARD	5
#define DMODE_SGROUP	6
#define DMODE_THREAT	7
#define DMODE_STRINGID	8
#define DMODE_CONNECT	9
#define DMODE_SAFECON	10
#define DMODE_PROT	11
#define DMODE_POWER	12
#define DMODE_POWER2	13
#define DMODE_LIBERTY	14
#define DMODE_WGROUP	15
#define DMODE_VEYE	16
#define DMODE_POWER1	17
#define DMODE_MYSTONES	18
#define DMODE_HISSTONES 19
#define DMODE_SAFEBOARD 20
#define DMODE_IGNORESAFE 21
#define DMODE_CONNECT2	22
#define DMODE_JUMP1	23

/*-- definitions used when counting up --*/

#define CNT_UNDECIDED	0
#define CNT_BLACK_TERR	1
#define CNT_WHITE_TERR	2
#define CNT_NOONE	3

/*-- macro functions --*/

#define GET_LIBC(x, y)		sList[SMap[stringIDs[x][y]]].libC

#define NEXT_TURN	(current_turn == BLACK ? WHITE : BLACK)

#define Getmove(func)	void(func)(void)

#define BB_TAKECORNER	1
#define BB_FINDPAT	2
#define BB_SAVE		3
#define BB_SAVEN	4
#define BB_SAVE2	5
#define BB_EXTEND	6
#define BB_KILL		7
#define BB_DOUBLE	8
#define BB_ATTACK	9
#define BB_EXTEND2	10
#define BB_CONNECT	11
#define BB_BLOCK	12
#define BB_CUT		13
#define BB_WALL		14
#define BB_WALL2	15
#define BB_ATTACK2	16
#define BB_PUSH		17
#define BB_EDGE		18
#define BB_UNDER	19
#define BB_BLOCK1	20
#define BB_ATARI	21
#define BB_REDUCE	22
#define BB_EDGE2	23
#define BB_JUMP1	24
#define BB_DAME		25
#define BB_CUTNKILL	26

struct moveseqtype {
  int id;
  Getmove(*get);
  char *why;
  int value;
};

struct dmode_type
{
  char *name;
  int mode, func;
  intBoard *board;
};

/* 사용자 형태 */
#define TYPE_BADUKI	0
#define TYPE_HUMAN	1
#define TYPE_GMP	2
#define TYPE_IGS	3


/* 바둑관련 정보 */
#define MAX_PASSES	2

/* */
#define COLOR_NAME(color)	((color) == BLACK ? _("Black") : _("White"))

/* New */

void initgame();
int colletter(int letter);
void estimatescore();
void cout_score();
void score();
void place_fixed_handicap();
void place_handicap_stone (int x, int y);
void setHandicap9(int handicap9);
void setHandicap13(int handicap13);
void setHandicap19(int handicap19);
int enginemove();
int mymove();
void printgame();
void movedone();
int scanfcoo(int *x, int *y);
int enemymove();

// dummy functions, originally used to update the Goban widget
void  removestone( int x, int y);
void  placestone( enum bVal color, int x, int y );

/* baduki.cpp */
void restart_baduk(int handicap);
void bdk_mark_death_stone(int x, int y);
void UndoLastMove(void);
void RedoNextMove(void);
int bdk_place_stone(enum bVal color, int x, int y);
void bdk_init();

/* main.c */
void show_message(const char *buf);
void show_moveinfo();

/* */

/* Procedures from baduki.cpp */

int Connect (enum bVal, int, int, int[4], int[4], int *, int *);
int Maxlibs (int, int);
int Suicide (enum bVal, int, int);
int StoneLibs (int, int);
void EraseMarks ();
void GoRemoveStone (int, int);
void MergeStrings (int, int);
void DeleteString (int);
void ReEvalStrings (enum bVal, int, int, int);
void StringCapture (int);
void FixLibs (enum bVal, int, int, int);
void goRestart ();
void RelabelStrings ();
int CountAndMarkLibs (int, int);
void CountLiberties (int);
void CheckForEye (int, int, int[4], int, int *);
void CountEyes ();
void printStringReport (int, int);
void CountUp();
int CountFromPoint(int, int);

/* think.cpp */
int genMove(enum bVal color, int *x, int *y);
void add_altmoves(int x, int y, float value, int target);

/* killable.c */

void sSpanString (int, int, sPointList *);
void spanString (int, int, pointList *);
void bdk_pause();

void genState ();
void initGPUtils ();
void genBord (enum bVal);

int checkPos (int, int, int);

int noNbrs (int, int);
int heCanCut (int, int);
int safeMove (int, int);

Getmove(takeCorner);
Getmove(extend);
Getmove(extend2);
Getmove(lookForSave);
Getmove(lookForSaveN);
Getmove(lookForSave2);
Getmove(lookForKill);
Getmove(doubleAtari);
Getmove(lookForAttack);
Getmove(threaten);
Getmove(connectCut);
Getmove(extendWall);
Getmove(extendWall2);
Getmove(findAttack2);
Getmove(blockCut);
Getmove(cutHim);
Getmove(atariAnyway);
Getmove(underCut);
Getmove(BlockUnderCut);
Getmove(dropToEdge);
Getmove(pushWall);
Getmove(reduceHisLiberties);
Getmove(dropToEdge2);
Getmove(find_pattern);
Getmove(thk_jump1);
Getmove(fill_dame);
Getmove(cutNkill);

/* utils.c */

int saveable (int, int, int *, int *);
int killable (int, int, int *, int *);
void initBoolBoard (boolBoard);
void intersectPlist (pointList *, pointList *, pointList *);
void initArray (intBoard);
void initState ();
void copyArray (intBoard, intBoard);
void stake ();
void spread ();
void respreicen ();
void tryPlay (int, int, int);
void saveState ();
void restoreState ();
int tencen (int, int);
void genConnects ();
void sortLibs();
void markDead();
void markLive();
void undoTo(int uMark);
void listAdjacents(int x, int y, intList *iL);
void listDiags(int x, int y, sPointList *diags);
int listJump1(int sid, sPointList llist, sPointList *jump1);
void save_current_to_sgf();
void showStack(void);
int get_dxy(int dir, int x, int y, int dist, int *dx, int *dy);
int get_ddxy(int dir, int x, int y, int xdist, int ydist, int *dx, int *dy);
void get_cxy(int dir, int x, int y, int *dx, int *dy);

int height_cmp(int x1, int y1, int x2, int y2);
int height_cmp2(int x1, int y1, int x2, int y2);
int get_height(int x, int y);

void merge_group(int gid1, int gid2);

/* pattern.c */
void init_pattern();

extern intBoard legal, claim, extra, bord, ndbord, safeboard, sGroups, threatBord,
	stringIDs, connectMap, protPoints, powerBoard, powerBoard2, libBoard,
	scoreBoard, safeConnect, GIDMap, wGIDMap, vEyeMap, powerBoard1,
	valueBoard, myStones, hisStones, ignoreSafe, connectMap2,
	extMap[4], jump1Map;

#endif  // __baduki_h
