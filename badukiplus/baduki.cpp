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

/* $Id: baduki.c,v 1.10 1999/03/20 00:52:02 artist Exp $ */ 

#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <string>

using namespace std;

#include "baduki.h"
#include "game.h"

extern char *playReason;

extern int playLevel, showTrees;

game thegame; // the game class instantiated

/* List of vertices where handicap stones have been placed */
string handicapList;

struct bRec	goboard[maxSIZE][maxSIZE];	/*-- The main go board		--*/

struct String StringList[MAXSTRINGS]; /*-- The list of Strings		--*/
int DeletedStrings[4];
/*-- Codes of deleted strings	--*/

int StringCount = 0;
/*-- The total number of strings	--*/
int DeletedStringCount;
/*-- The total number of strings	--*/
/*-- deleted on a move		--*/

int ko, koX, koY;
int blackTerritory, whiteTerritory;
int Prisoners[2];
int current_score[2] = {0, 0};
int showMoveReason = TRUE,
    stringInfo = FALSE,
    whitePassed = FALSE,
    blackPassed = FALSE;

int current_move = 0;
int max_move = 0;
pointc move_log[400];
float dum = 0;
int board_size = 19;
int ignore_takecorner = FALSE;

// two dummy functions

void placestone(enum bVal color, int x, int y) {
// cout << "Placing stone, color: " << color << " column: " << x;
// cout << " row: " << y << endl;
}

void removestone(int x, int y) {
// cout << "Removing stone, column: " << x;
// cout << " row: " << y << endl;
}

enum bVal nextp (enum bVal p) {

/*Return the code for the other player, who is the next to play.*/

enum bVal np = BLACK;

if (p == BLACK) np = WHITE;
else np = BLACK;

return np;
}

void FillPoints(int x, int y, int val);

/* Arrays for use when checking around a point */
int xVec[4] =
{0, 1, 0, -1};
int yVec[4] =
{-1, 0, 1, 0};

int member(int string, int stringlist[4], int cnt)
{
  int i;


  for (i = 0; i < cnt; i++)
    if (stringlist[i] == string)
      return TRUE;
  return FALSE;
}

/* Does a stone at x, y connect to any strings of color? */
int Connect(enum bVal color, int x, int y, int fStrings[4], int *fCnt, int eStrings[4], int *eCnt)
{
  unsigned int point = 0;
  int tx, ty, total = 0;
  enum bVal opcolor = WHITE;


  *fCnt = 0;
  *eCnt = 0;
  if (color == WHITE)
    opcolor = BLACK;
  for (point = 0; point <= 3; point++) {
    tx = x + xVec[point];
    ty = y + yVec[point];
    if (!LegalPoint(tx, ty))
      continue;
    if (goboard[tx][ty].Val == color) {
      total++;
      if (!member(goboard[tx][ty].StringNum, fStrings, *fCnt))
	fStrings[(*fCnt)++] = goboard[tx][ty].StringNum;
    } else if (goboard[tx][ty].Val == opcolor) {
      total++;
      if (!member(goboard[tx][ty].StringNum, eStrings, *eCnt))
	eStrings[(*eCnt)++] = goboard[tx][ty].StringNum;
    }
  }
  return total;
}

/* Returns the maximum number of liberties for a given intersection */
int Maxlibs(int x, int y)
{
  int cnt = 4;


  if (x == 0 || x == 18)
    cnt--;
  if (y == 0 || y == 18)
    cnt--;
  return cnt;
}

void bdk_mark_death_stone(int x, int y)
{
  if (goboard[x][y].Val != EMPTY)
    StringCapture(goboard[x][y].StringNum);
}

/* Determine whether x, y is suicide for color */
int Suicide(enum bVal color, int x, int y)
{
  enum bVal opcolor = BLACK;
  int friendlycnt, friendlystrings[4], enemycnt, enemystrings[4], total;
  int maxlibs, i, libcnt = 0;


  if (color == BLACK)
    opcolor = WHITE;
  maxlibs = Maxlibs(x, y);
  total = Connect(color, x, y, friendlystrings, &friendlycnt,
      enemystrings, &enemycnt);

  /* 할일: 여기서 libboard외 기타등등 만듦 */

  if (total < maxlibs)
    return FALSE;

  /* Check for a capture */
  for (i = 0; i < enemycnt; i++)
    if (StringList[enemystrings[i]].liberties == 1)
      return FALSE;
  for (i = 0; i < friendlycnt; i++)
    libcnt += (StringList[friendlystrings[i]].liberties - 1);
  if (libcnt != 0)
    return FALSE;
  return TRUE;
}

/* Returns the number of liberties for x, y */
int StoneLibs(int x, int y)
{
  int cnt = 0, tx, ty;
  unsigned int point;


  for (point = 0; point <= 3; point++) {
    tx = x + xVec[point];
    ty = y + yVec[point];
    if (LegalPoint(tx, ty) && goboard[tx][ty].Val == EMPTY)
      cnt++;
  }
  return cnt;
}

void EraseMarks()
{
  register int i;
  register struct bRec *gpt = &goboard[0][0];


  for (i = 0; i < 361; gpt++, i++)
    gpt->marked = FALSE;
}

/* Place a stone of color at x, y */
int bdk_place_stone(enum bVal color, int x, int y)
{
  int fstrings[4], estrings[4];	/* string codes surrounding stone */
  int fcnt, ecnt, i;
  int lowest = StringCount + 1;
  int new_move = TRUE;

  DeletedStringCount = 0;
  
  if (max_move > current_move && 
	(move_log[current_move].px == x && move_log[current_move].py == y))
    new_move = FALSE;

  /* -1일때는 통과 */
  if (x == -1 || y == -1) {
    move_log[current_move].px = x;
    move_log[current_move].py = y;
    move_log[current_move].color = color;
    current_move++;
    ko = FALSE;
    if (new_move)
      max_move = current_move;
    
    return TRUE;
  }

  if (goboard[x][y].Val != EMPTY || Suicide(color, x, y))
    return FALSE;

  if (ko && koX == x && koY == y)
    return FALSE;

  move_log[current_move].px = x;
  move_log[current_move].py = y;
  move_log[current_move].color = color;
  current_move++;
  if (new_move)
    max_move = current_move;
  
  ko = FALSE;
  placestone(color, x, y);
  goboard[x][y].Val = color;
  /* Does the new stone connect to any friendly stone(s)? */
  Connect(color, x, y, fstrings, &fcnt, estrings, &ecnt);
  if (fcnt) {
    /* Find the connecting friendly string with the lowest code */
    for (i = 0; i < fcnt; i++)
      if (fstrings[i] <= lowest)
	lowest = fstrings[i];
/*-- Renumber resulting string --*/
/*-- Raise the stone count of the lowest by one to account --*/
/*-- for new stone --*/
    goboard[x][y].StringNum = lowest;
    StringList[lowest].count++;
    for (i = 0; i < fcnt; i++)
      if (fstrings[i] != lowest)
	MergeStrings(lowest, fstrings[i]);
    /* Fix the liberties of the resulting string */
    CountLiberties(lowest);
  } else {
    /* Isolated stone.  Create new string. */
    StringCount++;
    lowest = StringCount;
    StringList[lowest].color = color;
    StringList[lowest].count = 1;
    StringList[lowest].internal = 0;
    StringList[lowest].external = StoneLibs(x, y);
    StringList[lowest].liberties = StringList[lowest].external;
    StringList[lowest].eyes = 0;
    StringList[lowest].alive = 0;
    StringList[lowest].territory = 0;
    goboard[x][y].StringNum = lowest;
  }
  /* Now fix the liberties of enemy strings adjacent to played stone */
  FixLibs(color, x, y, PLACED);	/* Fix the liberties of opcolor */
  ReEvalStrings(color, x, y, lowest);
  RelabelStrings();
  return TRUE;
}

/*
void UndoLastMove()
{
  extern int max_move;
  extern int last_x[2], last_y[2], player_type[2];
  extern int pass_count;
  extern GtkGoBoard *go_board;
  int i, old_move, old_max_move;
  
  if (current_move < 1)
    return;

  old_max_move = max_move;
  
  old_move = current_move - 1;
  
  if (old_move > 1 && player_type[nextp(thegame.pla)] != TYPE_HUMAN) {
    old_move--;
  }
  
  gtk_go_board_clear_board(go_board);
  restart_baduk(thegame.blackHandicap);
  
  for (i = 0; i < old_move; i++) {
    if (move_log[i].px < 0)
      pass_count++;
    else {
      pass_count = 0;
    }
    if (move_log[i].color == BLACK) {
      last_x[BLACK] = move_log[i].px + 1;
      last_y[BLACK] = move_log[i].py + 1;
    } else {
      last_x[WHITE] = move_log[i].px + 1;
      last_y[WHITE] = move_log[i].py + 1;
    }
    bdk_place_stone(move_log[i].color, move_log[i].px, move_log[i].py);
  }
  thegame.pla = nextp(move_log[i-1].color);
  if (last_x[BLACK])
    gtk_go_board_set_mark(go_board, last_x[BLACK], last_y[BLACK], GTK_GO_BOARD_TR_MASK);
  if (last_x[WHITE])
    gtk_go_board_set_mark(go_board, last_x[WHITE], last_y[WHITE], GTK_GO_BOARD_TR_MASK);

  max_move = old_max_move;
  show_moveinfo();
}
 */

/*
void RedoNextMove()
{
  extern int current_turn;
  extern int pass_count;
  extern int last_x[2], last_y[2];
  extern GtkGoBoard *go_board;
  
  if (max_move <= current_move)
    return;
  
  current_turn = move_log[current_move].color;
  if (last_x[current_turn])
    gtk_go_board_set_mark(go_board,
    	last_x[current_turn], last_y[current_turn], 0);
   
  last_x[current_turn] = move_log[current_move].px + 1;
  last_y[current_turn] = move_log[current_move].py + 1;
  
  if (move_log[current_move].px == -1)
    pass_count++;
  else
    pass_count = 0;

  bdk_place_stone(current_turn, move_log[current_move].px,
  			move_log[current_move].py);

  if (last_x[current_turn])
    gtk_go_board_set_mark(go_board,
	last_x[current_turn], last_y[current_turn], GTK_GO_BOARD_TR_MASK);
 
  current_turn = NEXT_TURN;
  show_moveinfo();
}
 */

/* Remove a stone from the board */
void GoRemoveStone(int x, int y)
{
  goboard[x][y].Val = EMPTY;
  goboard[x][y].StringNum = 0;
  removestone(x, y);
}

/* Merges two strings -- Renumbers stones and deletes second string from
   list.  Fixes stone count of strings.  This does not fix anything else. 
   FixLibs must be called to fix liberties, etc. */
void MergeStrings(int g1, int g2)
{
  int x, y;


  ForeachPoint(y, x)
      if (goboard[x][y].StringNum == g2)
    goboard[x][y].StringNum = g1;
  StringList[g1].count += StringList[g2].count;
  DeleteString(g2);		/* Removes string from StringList */
}

/* Stores a string code to be deleted */
void DeleteString(int code)
{
  DeletedStrings[DeletedStringCount++] = code;
}

/* Re-evaluate the strings given the last move.  This assumes that the
   last move has been merged into adjoining strings and all liberty counts
   are correct.  Handles capture. Checks for Ko.  Keeps track of captured
   stones. code is the string number of the stone just played. */
void ReEvalStrings(enum bVal color, int x, int y, int code)
{
  int fstrings[4], estrings[4], fcnt, ecnt, i, killcnt = 0, count = 0;
  enum bVal opcolor = BLACK;

  if (color == BLACK)
    opcolor = WHITE;
  /* Check for capture */
  Connect(color, x, y, fstrings, &fcnt, estrings, &ecnt);
  if (ecnt) {
    /* See if any of the strings have no liberties */
    for (i = 0; i < ecnt; i++)
      if (StringList[estrings[i]].liberties == 0) {
	killcnt++;
	count = StringList[estrings[i]].count;
	StringCapture(estrings[i]);
      }
  }
  /* Check for ko.  koX and koY are set in StringCapture above. */
  if (killcnt == 1 && count == 1 && StringList[code].count == 1
      && StringList[code].liberties == 1) {
    ko = TRUE;
  }
  if (killcnt)
//    intrPrisonerReport(Prisoners[BLACK], Prisoners[WHITE]);
  /* Set eye count for strings */
  CountEyes();
}

/* Remove a captured string from the board and fix the liberties of any
   adjacent strings.  Fixes prisoner count. Sets KoX and KoY */
/*-- update display of captured stones -neilb --*/
void StringCapture(int code)
{
  int x, y;

  if (StringList[code].color == BLACK)
    Prisoners[BLACK] += StringList[code].count;
  else
    Prisoners[WHITE] += StringList[code].count;
//  intrPrisonerReport(Prisoners[BLACK], Prisoners[WHITE]);
  ForeachPoint(y, x)
      if (goboard[x][y].StringNum == code) {
    FixLibs(StringList[code].color, x, y, REMOVED);
    GoRemoveStone(x, y);
    koX = x;
    koY = y;
  }
  DeleteString(code);
}

/* Fix the liberties of strings adjacent to x, y.  move indicates
   whether a stone of color was placed or removed at x, y
   This does not change liberty counts of friendly strings when a stone
   is placed.  Does not do captures. */
void FixLibs(enum bVal color, int x, int y, int move)
{
  int fstrings[4], fcnt, estrings[4], ecnt, i;
  enum bVal opcolor = BLACK;

  if (color == BLACK)
    opcolor = WHITE;
  Connect(color, x, y, fstrings, &fcnt, estrings, &ecnt);
  if (move == PLACED)
    for (i = 0; i < ecnt; i++)
      StringList[estrings[i]].liberties--;
  else				/* Stone removed so increment opcolor */
    for (i = 0; i < ecnt; i++)
      StringList[estrings[i]].liberties++;
}

void place_handicap_stone (int x, int y) {
  bdk_place_stone( BLACK, x, y );
  // add vertex to handicapList
  // a space
  handicapList += " ";
  // a letter
  handicapList += (char)lettercol(x);
  // a number
  if ( y >= 9 ){
    handicapList += "1";
    if ( y == 9 )
      handicapList += "0";
    else
      handicapList += ('1' + ( y % 10 ));
  }
  else
    handicapList += ( '1' + y );
}

int colletter(int letter)
/*
Return the column # (0 to boardsize-1) which corresponds to letter.
Due to perversity of tournament organizers etc., 'i' or 'I' is
omitted, leading to an increase in complexity of this routine.
Return (-1) on illegal input.
*/
{ register int result;

  if('a'<=letter&&letter<='z')
    result= letter-'a';
  else if('A'<=letter&&letter<='Z')
    result= letter-'A';
  else
    return (-1);

  if(8>result)
    ;
  else if(8==result)
    return (-1);
  else
    --result;

  if(result >= thegame.boardSize)
    return (-1);
  else
    return result;
}

void initgame() {

  // Initialize the Random Number Generator
  srand(thegame.RANDOM_SEED);

  // Initialize various thegame variables
  // ko information
  thegame.kox = thegame.koy = (-1);
  // Captured pieces
  thegame.capturedWhite = thegame.capturedBlack = 0;
  // turn
  thegame.tur = 1;
  // pass status
  thegame.qpa = 0;
  // who is to play
  if(thegame.blackHandicap < 2)
    thegame.pla= BLACK;
  else
    thegame.pla= WHITE;

  // Amigo initialization
  goRestart();
}


void goRestart()
{
  register int x, y;

  StringCount = 0;
  ko = FALSE;
  Prisoners[BLACK] = Prisoners[WHITE] = 0;
//  intrPrisonerReport(0, 0);
  // Erase the board
	for (x = 0; x < thegame.boardSize ; x++)
    for (y = 0; y < thegame.boardSize ; y++) {
      goboard[x][y].Val = EMPTY;
      goboard[x][y].StringNum = 0;
    }
  place_fixed_handicap();
  current_move = 0;
  max_move = 0;
}

void place_fixed_handicap()
{
  // place the stones based on board size and build
  // the handicap stones vertices list simultaneously

  // clear the handicap stones vertices list first
  handicapList.erase();

  if(thegame.blackHandicap > 1){
    if (thegame.boardSize == 9)
      setHandicap9(thegame.blackHandicap);
    if (thegame.boardSize == 13)
      setHandicap13(thegame.blackHandicap);
    if (thegame.boardSize == 19)
      setHandicap19(thegame.blackHandicap);
  }
}

void setHandicap9(int handicap9) {
 if (handicap9 < 2)
    return;

  place_handicap_stone(2, 2);
  place_handicap_stone(6, 6);

  if (handicap9 >= 3)
    place_handicap_stone(2, 6);
  if (handicap9 >= 4)
    place_handicap_stone(6, 2);
}

void setHandicap13(int handicap13) {
 if (handicap13 < 2)
    return;

  place_handicap_stone(3, 3);
  place_handicap_stone(9, 9);

  if (handicap13 >= 3)
    place_handicap_stone(3, 9);
  if (handicap13 >= 4)
    place_handicap_stone(9, 3);
  if (handicap13 == 5 || handicap13 == 7 || handicap13 == 9)
    place_handicap_stone(6, 6);
  if (handicap13 >= 6) {
    place_handicap_stone(9, 6);
    place_handicap_stone(3, 6);
  }
  if (handicap13 >= 8) {
    place_handicap_stone(6, 9);
    place_handicap_stone(6, 3);
  }
}

void setHandicap19(int handicap19) {
 if (handicap19 < 2)
    return;

  place_handicap_stone(3, 3);     // D4
  place_handicap_stone(15, 15);   // Q16

  if (handicap19 >= 3)
    place_handicap_stone(3, 15);  // D16
  if (handicap19 >= 4)
    place_handicap_stone(15, 3);  // Q4
  if (handicap19 == 5 || handicap19 == 7 || handicap19 == 9)
    place_handicap_stone(9, 9);   // K10
  if (handicap19 >= 6) {
    place_handicap_stone(3, 9);   // D10
    place_handicap_stone(15, 9);  // Q10
  }
  if (handicap19 >= 8) {
    place_handicap_stone(9, 3);   // K4
    place_handicap_stone(9, 15);  // K16
  }
}

void opponentpass()
{
  if (thegame.BadukiColor == BLACK) whitePassed = TRUE;
  else blackPassed = TRUE;
  thegame.qpa= 1;
  // clear the ko status
  ko = FALSE;
  // done
  movedone();
}

void movedone()
/*
  Do everything which must be done to complete a move after the stone
  is placed.
 */
{ thegame.pla= nextp(thegame.pla);
  ++thegame.tur;
}

int mymove()
{
  int move;
  move = enginemove();
  if ( (move == PASS) || (move == BOTHPASS) ){
    cout << ((thegame.BadukiColor == WHITE)?"White":"Black");
    cout << " passes." << endl;
  }
  else {
    cout << ((thegame.BadukiColor == WHITE)?"White":"Black");
    cout << " moves to ";
    cout.put(lettercol(thegame.ex));
    cout << thegame.ey+1 << "." << endl;
  }
  return move;
}

int enginemove()
/*Calculate and execute the engine's move.*/
{
  int x, y;   // coords of the move selected

  if (genMove(thegame.BadukiColor, &x, &y)) {
    // if baduki found a move, play it
    bdk_place_stone(thegame.BadukiColor, x, y);
    // update pass information
    if (thegame.BadukiColor == BLACK) blackPassed = FALSE;
    else whitePassed = FALSE;
    // update game object
    thegame.qpa= 0;
    thegame.ex = x;
    thegame.ey = y;
    // write vertex to string
    thegame.emove = (char)lettercol(x);
    if ( y >= 9 ){
      if ( y >= 19 )
        thegame.emove += "2";
      else
        thegame.emove += "1";
      if ( y == 9 || y == 19 )
        thegame.emove += "0";
      else
        thegame.emove += ('1' + ( y % 10 ));
    }
    else
      thegame.emove += ( '1' + y );
    return PLAY_MOVE;
  }
  else {
    if (thegame.BadukiColor == BLACK) blackPassed = TRUE;
    else whitePassed = TRUE;
    thegame.emove = "pass";
    if(thegame.qpa) return BOTHPASS;
    else {
      thegame.qpa= 1;
      // clear the ko status?
      return PASS;
    }
  }
  movedone(); // the engine is done
}


/* if any strings have been deleted as a result of the last move, this
   routine will delete the old string numbers from StringList and
   reassign string numbers. */
void RelabelStrings()
{
  int i, j, x, y;

  for (i = 0; i < DeletedStringCount; i++) {
    /* Relabel all higher strings */
    ForeachPoint(y, x)
	if (goboard[x][y].StringNum > DeletedStrings[i])
      goboard[x][y].StringNum--;
    /* Move the strings down */
    for (y = DeletedStrings[i]; y < StringCount; y++)
      StringList[y] = StringList[y + 1];
    /* fix the string numbers stored in the deleted list */
    for (j = i + 1; j < DeletedStringCount; j++)
      if (DeletedStrings[j] > DeletedStrings[i])
	DeletedStrings[j]--;
    StringCount--;
  }
}

/* Returns liberty count for x, y intersection.  Sets marked to true
   for each liberty */
int CountAndMarkLibs(int x, int y)
{
  int tx, ty, i;
  int cnt = 0;


  for (i = 0; i < 4; i++) {
    tx = x + xVec[i];
    ty = y + yVec[i];
    if (LegalPoint(tx, ty) && goboard[tx][ty].Val == EMPTY
	&& goboard[tx][ty].marked == FALSE) {
      cnt++;
      goboard[tx][ty].marked = TRUE;
    }
  }
  return cnt;
}

/* Determine the number of liberties for a string given the string code
   num */
void CountLiberties(int code)
{
  int x, y, libcnt = 0;

  ForeachPoint(y, x)
      if (goboard[x][y].StringNum == code)
    libcnt += CountAndMarkLibs(x, y);
  EraseMarks();
  StringList[code].liberties = libcnt;
}

void CheckForEye(int x, int y, int strings[4], int cnt, int *recheck)
{
  int i;

  for (i = 0; i < (cnt - 1); i++)
    if (strings[i] != strings[i + 1]) {
      /* Mark liberty for false eye check */
      goboard[x][y].marked = TRUE;
      (*recheck)++;
      return;
    }
  /* It is an eye */
  StringList[strings[i]].eyes += 1;
}

/* Set the eye count for the strings */
void CountEyes()
{
  int i, x, y, wstrings[4], bstrings[4], wcnt, bcnt, max, cnt, recheck = 0,
    eye;

  for (i = 1; i <= StringCount; i++)
    StringList[i].eyes = 0;

  ForeachPoint(y, x) {
    if (goboard[x][y].Val != EMPTY)
      continue;
    cnt = Connect(WHITE, x, y, wstrings, &wcnt, bstrings, &bcnt);
    max = Maxlibs(x, y);
    if (cnt == max && wcnt == 1 && bcnt == 0)
      StringList[wstrings[0]].eyes++;
    else if (cnt == max && bcnt == 1 && wcnt == 0)
      StringList[bstrings[0]].eyes++;
    else if (cnt == max && (bcnt == 0 || wcnt == 0)) {
      goboard[x][y].marked = TRUE;
      recheck++;
    }
  }

/*-- Now recheck marked liberties to see if two or more one eye --*/
/*-- strings contribute to a false eye */
  if (recheck == 0)
    return;

  ForeachPoint(y, x)
      if (goboard[x][y].marked) {
    recheck--;
    goboard[x][y].marked = FALSE;
    Connect(WHITE, x, y, wstrings, &wcnt, bstrings, &bcnt);
    /* If all the strings have at least one eye then all the
       strings are safe from capture because of the common
       liberty at x, y */
    eye = TRUE;
    for (i = 0; i < wcnt; i++)
      if (StringList[wstrings[i]].eyes == 0)
	eye = FALSE;
    if (eye)
      for (i = 0; i < wcnt; i++)
	StringList[wstrings[i]].eyes++;
    for (i = 0; i < bcnt; i++)
      if (StringList[bstrings[i]].eyes == 0)
	eye = FALSE;
    if (eye)
      for (i = 0; i < bcnt; i++)
	StringList[bstrings[i]].eyes++;
    if (recheck == 0)
      return;
  }
}


int foo[19][19];

/*----------------------------------------------------------------
-- CountUp()							--
--	Count up final scores at the end of the game.		--
----------------------------------------------------------------*/
void CountUp()
{
  int x, y;
  float btotal, wtotal;
//  char buff[512];

  blackTerritory = whiteTerritory = 0;
  ForeachPoint(y, x) {
    goboard[x][y].marked = FALSE;
    foo[x][y] = CNT_UNDECIDED;
  }
  ForeachPoint(y, x)
      if (goboard[x][y].Val == EMPTY && foo[x][y] == CNT_UNDECIDED) {
    FillPoints(x, y, CountFromPoint(x, y));
  }
  wtotal = whiteTerritory + Prisoners[BLACK] + dum;
  btotal = blackTerritory + Prisoners[WHITE];

/*  sprintf(buff, _("\
White  : %3d territory + %3d prisoners + %3.1f komi = %.1f\n\
Black  : %3d territory + %3d prisoners = %.0f\n\n\
Result : %s wins by %.1f"),
      whiteTerritory, Prisoners[BLACK], dum, wtotal,
      blackTerritory, Prisoners[WHITE], btotal,
      (btotal > wtotal ? _("Black") : _("White")),
      (btotal > wtotal ? (btotal - wtotal) : (wtotal - btotal)));
 */
//  show_message_box(buff);

/*  if (btotal > wtotal)
    show_message(("Black Wins!"));
  else if (wtotal > btotal)
    show_message(("White Wins!"));
  else
    show_message(("Even"));
 */
}

void FillPoints(int x, int y, int val)
{
  int i;
  int tx, ty;


  if ((foo[x][y] = val) == CNT_BLACK_TERR)
    blackTerritory++;
  else if (val == CNT_WHITE_TERR)
    whiteTerritory++;
  for (i = 0; i < 4; i++) {
    tx = x + xVec[i];
    ty = y + yVec[i];
    if (!LegalPoint(tx, ty))
      continue;
    if (goboard[tx][ty].Val == EMPTY && foo[tx][ty] == CNT_UNDECIDED)
      FillPoints(tx, ty, val);
  }
}

int CountFromPoint(int x, int y)
{
  int i;
  int tx, ty;
  int blkcnt = 0, whtcnt = 0;
  int baz;

  goboard[x][y].marked = TRUE;
  for (i = 0; i < 4; i++) {
    tx = x + xVec[i];
    ty = y + yVec[i];
    if (!LegalPoint(tx, ty))
      continue;
    if (goboard[tx][ty].Val == BLACK)
      blkcnt++;
    else if (goboard[tx][ty].Val == WHITE)
      whtcnt++;
    else {
      if (goboard[tx][ty].marked)
	continue;
      baz = CountFromPoint(tx, ty);
      if (baz == CNT_NOONE)
	return CNT_NOONE;
      else if (baz == CNT_BLACK_TERR)
	blkcnt++;
      else if (baz == CNT_WHITE_TERR)
	whtcnt++;
    }
    if (blkcnt && whtcnt)
      return CNT_NOONE;
  }
  if (blkcnt && !whtcnt)
    return CNT_BLACK_TERR;
  else if (whtcnt && !blkcnt)
    return CNT_WHITE_TERR;
  else
    return CNT_UNDECIDED;
}


/*---------------*/

int play_level = 7;

void restart_baduk(int handicap)
{
  register int i;
  register struct bRec *gpt = &goboard[0][0];


  StringCount = 0;
  ko = FALSE;
  ignore_takecorner = FALSE;
  Prisoners[BLACK] = Prisoners[WHITE] = 0;
//  intrPrisonerReport(0, 0);
  for (i = 0; i < 361; gpt++, i++) {
    gpt->Val = EMPTY;
    gpt->StringNum = 0;
  }
  initGPUtils();
  thegame.blackHandicap = handicap;
  place_fixed_handicap();
  current_move = 0;
  max_move = 0;
}

/* 각종 초기화 */
void bdk_init()
{
}

// Scoring

void score()
{
  // Count territories
  estimatescore();

  // Output results
  cout_score();
}


void cout_score()
{
  float finalScore;

  cout << "(the end of the game)\n";
  cout << "Komi is: " << setprecision(1) << thegame.komi << endl;
  cout << "Points count is:\n";
  // Black
  cout << "Black has " << thegame.territoryBlack;
  cout << " territory, and has captured " << thegame.capturedWhite;
  cout << " White stones." << endl;
  // White
  cout << "White has " << thegame.territoryWhite;
  cout << " territory, and has captured " << thegame.capturedBlack;
  cout << " Black stones." << endl;
  // Who wins?
  finalScore = thegame.territoryBlack + thegame.capturedWhite
             - thegame.territoryWhite - thegame.capturedBlack
             - thegame.komi;
  if( 0 == finalScore )
    cout << "(a tie)\n"; // impossible
  else if( finalScore > 0 )
    cout << "(Black wins.)\n";
  else
    cout << "(White wins.)\n";
}


void estimatescore()
{
	short	x,y;
	int	btotal, wtotal;

	blackTerritory = whiteTerritory = 0;
	ForeachPoint(y,x)
	{
		goboard[x][y].marked = FALSE;
		foo[x][y] = CNT_UNDECIDED;
	}
	ForeachPoint(y,x)
		if (goboard[x][y].Val==EMPTY && foo[x][y]==CNT_UNDECIDED)
		{
			FillPoints(x,y,CountFromPoint(x,y));
		}

	wtotal = whiteTerritory + Prisoners[BLACK];
	btotal = blackTerritory + Prisoners[WHITE];

  thegame.territoryBlack = blackTerritory;
  thegame.territoryWhite = whiteTerritory;

  thegame.capturedBlack = Prisoners[BLACK];
  thegame.capturedWhite = Prisoners[WHITE];
}

void printgame()
/*
Output the game board theboard (with ko info from thegame) to the console.
Currently the style is:

   a b c d e f g h j
 1 . . . . . . . . . 1
 2 . . . . . O O . . 2
 3 . . O . . . # . . 3
 4 . . O . . . # . . 4
 5 O O ^ O . . . . . 5
 6 . # O # . # # . . 6
 7 . # # # . . . . . 7
 8 . . . . . . . . . 8
 9 . . . . . . . . . 9
   a b c d e f g h j

where '^' is a ko, '#' is black, 'O' is white.
*/
{ int x, y;
  // update thegame ko information
  if (ko) {
    thegame.kox = koX;
    thegame.koy = koY;
  }
  else thegame.kox = thegame.koy = (-1);

  // update thegame captured pieces information
  thegame.capturedBlack = Prisoners[BLACK];
  thegame.capturedWhite = Prisoners[WHITE];

  // print the board
  cout << "  "; // avoid printing an empty line for GTP V2
  cout << endl << "  ";
  for(x=0; x<thegame.boardSize; ++x) {
    cout << " ";
    cout.put(lettercol(x));
  }
  cout << "\n";
  for(y=0; y<thegame.boardSize; ++y)
  { cout.setf( ios::right, ios::adjustfield );
    cout.width(2);
    cout << (1+y) << " ";
    for(x=0; x<thegame.boardSize; ++x) {
      enum bVal here = goboard[x][y].Val;
      cout << (( thegame.kox == x && thegame.koy == y ) ? '^' :
      (EMPTY==here) ? '.' :
      (WHITE==here) ? 'O' :
      (BLACK==here) ? '#' :
      // Replaced
      // panic("illegal board type in printboard()"));
      // by
      '@');  // call the panic function later
      if ( x != (thegame.boardSize-1) ) cout << " ";
    }
    cout << " " << flush;
    cout.unsetf( ios::right );
    cout.width(2);
    cout.setf( ios::left, ios::adjustfield );
    cout << (y+1);
    cout.unsetf( ios::left );
    if ( y == 0 )
      cout << "   BLACK (#) has captured " << thegame.capturedWhite << " stones.";
    if ( y == 1 )
      cout << "   WHITE (O) has captured " << thegame.capturedBlack << " stones.";
    cout << endl;
  }
  cout << flush;
  cout << "  ";
  for(x=0; x<thegame.boardSize; ++x) {
    cout << " ";
    cout.put(lettercol(x));
  }
  cout << endl;
}

char getnb()
/*Read and return the first nonblank character from the standard input.*/
{ char c;
  const char TAB = 8;
  do
    c = cin.get();
  while(' '==c||'\n'==c||TAB==c);
  return c;
}

int scanfcoo(int *x, int *y)
/*
Read a coordinate from the standard input and write it to *x and *y.  Like
scanf, return 1 for success, 0 for failure, negative numbers for special
moves, e.g. PASS and RESIGN.
On any input error, the function eats input up to the next newline.
*/
{
  int xx, yy;
  char c1, c2, c3;

/* Get first nonblank character of input, normally a letter code for a column. */
  c1 = getnb();

/* Check for possible resignation (a '.' at the start of the input). */
  if('.'==c1)
  {
    do c3 = cin.get();
    while( c3 != '\n' ); /* Skip to end of line. */
    return RESIGN;
  }

/* Check for possible "pass". */
  if('p'==lowercase(c1))
  { c2 = cin.get();
    if('a'!=lowercase(c2))
      cin.putback(c2);
    else {
      c2 = cin.get();
      if( 's' != lowercase(c2) ) {
        cin.putback(c2);
        goto skipquit;
      }
      else {
        c2 = cin.get();
        if( 's' != lowercase(c2) ) {
            cin.putback(c2);
            goto skipquit;
          }
        else
          return PASS;
      }
    }
  }
  xx = colletter(c1);
  if(!(cin >> yy))       // get a valid integer input, otherwise return 0
  {
skipquit:                /* Skip to the next newline and return failure.*/
    do c3 = cin.get();
    while( c3 != '\n' ); /* Skip to end of line.*/
    return 0;
  }
  if(0==LegalPoint(xx,yy-1)) /* The -1 converts between C arrays starting */
                          /* at 0 and human boards counting from 1.*/
    goto skipquit;
  else
  { *x= xx;
    *y= yy-1;
    return 1;
  }
}

int enemymove()
/*Read and execute opponent's move.*/
{ int x, y;	/*coordinates of move*/
  int s;	/*return value of scanfcoo()*/

endrecurse:

  printgame();
  cout << "Game turn= " << thegame.tur << ".  ";
  cout << ((thegame.pla == WHITE)?"White":"Black") << " to play." << endl;
  cout << "Please input your move (" << ((thegame.pla == WHITE)?"White":"Black");
  cout << " #" << thegame.tur << "): ";
  s= scanfcoo(&x, &y);	/*Try to get a move.*/
  if(RESIGN==s)		/*If opponent resigns*/
   return RESIGN;
  if(PASS==s) {	/*If opponent passes*/
    movedone();
    thegame.kox = thegame.koy = (-1);
    if(thegame.qpa) return BOTHPASS;
    else
    { thegame.qpa= 1;
      return PASS;
    }
  }
  if( 0==s )
  { cout << "Please try again.  Input your move as a letter for the column, \n";
    cout << " followed by a number for the row number.\n";
    goto endrecurse;
  }
  if( 0==LegalPoint(x,y) )
  { cout << "Only moves which are on the board are legal.  Please try again.\n";
    goto endrecurse;
  }
  if( EMPTY != goboard[x][y].Val )
  { cout << "You cannot stack a stone on another.  ";
pleasetryanothermove:
    cout << "Please try another move." << endl;
    goto endrecurse;
  }
  if((x == koX) && (y == koY))
  { cout << "That move would violate the rule of ko.  ";
    goto pleasetryanothermove;
  }
  bdk_place_stone(thegame.pla, (short)x, (short)y);
  thegame.qpa= 0;
    cout.put(lettercol(x));
    cout << y+1 << endl;

  movedone();
  return 0;
}


