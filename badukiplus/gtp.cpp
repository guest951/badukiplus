/***************************************************************************
              gtp.cpp  -  an implementation of the Go Text Protocol
                             -------------------
    begin                : Fri Oct 4 2002
    copyright            : (C) 2002 by Andrew D. Balsa
    email                : andrebalsa@mailingaddress.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/*  The following code is roughly based on the implementation of GTP found in
 *  Gnu Go version 3.3.6, and the version 2, draft 1 - Specification of the
 *  Go Text protocol by Gunnar Farnebäck.
 *
 *  See http://www.lysator.liu.se/gunnar/gtp for details
 */


#include <iostream>
#include <iomanip>
#include <cstring>
#include <cstdlib>

using namespace std;

#include "gtp.h"
#include "game.h"

extern game thegame;

extern char* progname;
extern char* proginfo;
extern char* version;
extern string handicapList;

extern int colletter(int letter);
extern void initgame();
extern void opponentpass();
extern int bdk_place_stone(enum bVal color, int x, int y);
extern void printgame();
extern void estimatescore();
extern void place_fixed_handicap();
extern int enginemove();

// list of commands known to this GTP implementation

#define DECLARE(commandFunction) static int commandFunction(char *s)

// Required commands
DECLARE(gtp_protocol_version);
DECLARE(gtp_name);
DECLARE(gtp_version);
DECLARE(gtp_known_command);
DECLARE(gtp_list_commands);
DECLARE(gtp_quit);
DECLARE(gtp_boardsize);
DECLARE(gtp_clear_board);
DECLARE(gtp_komi);
DECLARE(gtp_play);
DECLARE(gtp_genmove);
// Debug commands
DECLARE(gtp_showboard);
// Tournament commands
DECLARE(gtp_fixed_handicap);
// Other commands
DECLARE(gtp_get_random_seed);
DECLARE(gtp_set_random_seed);
DECLARE(gtp_final_score);


static struct gtp_command commands[] = {
  {"protocol_version",        gtp_protocol_version},
  {"name",                    gtp_name},
  {"version",                 gtp_version},
  {"known_command",           gtp_known_command},
  {"list_commands",           gtp_list_commands},
  {"quit",                    gtp_quit},
  {"boardsize",               gtp_boardsize},
  {"clear_board",             gtp_clear_board},
  {"komi",                    gtp_komi},
  {"play",                    gtp_play},
  {"genmove",                 gtp_genmove},
  {"showboard",               gtp_showboard},
  {"fixed_handicap",          gtp_fixed_handicap},
  {"get_random_seed",         gtp_get_random_seed},
  {"set_random_seed",         gtp_set_random_seed},
  {"final_score",             gtp_final_score},
  {NULL,                      NULL}
};

const int SIZE = 127;
char id[SIZE];  // optional identity number for commands

void respond(char *result="");
void error(char *error_message="");

int gtp()
{
  char command[SIZE];
  char *ps = "0";
  int i;
  int status = 1;	// 1 = everything OK, 0 = quit

  while (status) {
    cin >> command;
/*    if (isdigit(command[0])) {
      strcpy( id, command );
      cin >> command;
    } else
      strcpy( id, "" );
 */
     /* Search the list of commands and call the corresponding function
     * if it's found.
     */
    for (i = 0; commands[i].name != NULL; i++) {
      if (strcmp(command, commands[i].name) == 0) {
	status = (*commands[i].function)(ps);
	break;
      }
    }
    if (commands[i].name == NULL)
      cerr << "unknown command: " << command;
  }
  return status;
}


static int
gtp_version(char *s)
{
  respond( version );
  return 1;
}
static int
gtp_protocol_version(char *s)
{
  respond( "2" );
  return 1;
}
static int
gtp_name(char *s)
{
  respond( progname );
  return 1;
}
static int
gtp_known_command(char *s)
{
  int k;
  bool known = false;
  char commandname[SIZE];
  cin >> commandname;
  cout << "= ";
  for (k = 0; commands[k].name != NULL; k++) {
    if (strcmp(commandname, commands[k].name) == 0) {
      known = true;
      break;
    }
  }
  cout << (known?"true":"false") << endl;
  cout << endl;
  return 1;
}
static int
gtp_list_commands(char *s)
{
  int k;
  cout << "= ";
  for (k = 0; commands[k].name != NULL; k++)
    cout << commands[k].name << endl;
  cout << endl;
  return 1;
}
static int
gtp_quit(char *s)
{
  respond();
  return 0;
}
static int
gtp_boardsize(char *s)
{
  int b;
  cin >> b;
  if ((b < 9) || (b > 19)) {
    error( "unacceptable size" );
    return 1;
  }
  thegame.boardSize = b;
  respond();
  return 1;
}
static int
gtp_clear_board(char *s)
{
  initgame();
  respond();
  return 1;
}
static int
gtp_komi(char *s)
{
  cin >> thegame.komi;
  respond();
  return 1;
}
static int
gtp_play(char *s)
{
  int i;
  char color[SIZE];    // color of the stone being played
  char vertx[SIZE];    // "pass", "resign" or vertex (letternumber format)
  cin >> color;
  cin >> vertx;
  for (i=0; color[i]; i++) color[i] = tolower(color[i]);  // get lowercase color
  for (i=0; vertx[i]; i++) vertx[i] = tolower(vertx[i]);  // get lowercase vertex
  if ( (strcmp(color, "white") != 0) && (strcmp(color, "black") != 0) &&  \
       (strcmp(color, "w")     != 0) && (strcmp(color, "b")     != 0) ) {
    error( "syntax error in color parameter" );
    return 1;
  }

  // deal with "pass"
  if (strcmp(vertx, "pass") == 0) {   // is the opponent's move "pass" ?
  opponentpass();
  respond();
  return 1;
  }
  // parse vertex
  int x, y;
  enum bVal player;
  char verty[3] = "  ";
  
  // first x (column letter)
  x = colletter( (int)vertx[0] );
  if (x == -1) {
    error( "illegal move" );
    return 1;
  }

  // then y (row number)
  for (i=1; (vertx[i] && (i <= 2)); i++) verty[i-1] = vertx[i];  // strip the letter
  y = atoi(verty) - 1;  // convert to integer in range 0 to boardsize-1
  
  // and player
  if(color[0] == 'b')
    player = BLACK;
  else
    player = WHITE;

  // send move to engine
  if (bdk_place_stone( player, x, y ) != 0) {
    respond();
    return 1;
  }
  else {
    error( "illegal move" );
    return 1;
  }
}
static int
gtp_genmove(char *s)
{
  int i;
  int movecode = 0;             // function enginemove() returns 0 for normal moves
  char color[SIZE];             // the engine must generate a move for this color
  char moveVertex[] = "resign"; // reply from engine: "pass" or vertex
    //check the color
  cin >> color;
  for (i=0; color[i]; i++) color[i] = tolower(color[i]); // get valid lowercase color
  if ( (strcmp(color, "white") != 0) && (strcmp(color, "black") != 0) &&  \
       (strcmp(color, "w")     != 0) && (strcmp(color, "b")     != 0) ) {
    error( "syntax error in color parameter" );
    return 1;
  }
  // set the color
  if (color[0] == 'b')
    thegame.BadukiColor = BLACK;
  else
    thegame.BadukiColor = WHITE;
  // generate the move  
  movecode = enginemove();   // function enginemove() returns 0 for normal moves
  int len = thegame.emove.length();
  thegame.emove.copy( moveVertex, len, 0 );
  moveVertex[ len ] = 0;     // null-terminate
  // send move to client
  respond( moveVertex ); 
  return 1;
}


// Debug Commands

static int
gtp_showboard(char *s)
{
  cout << "=" << id << " " << endl;
  printgame();
  cout << endl; // second consecutive newline indicates we're done
  return 1;
}


// Tournament commands

static int
gtp_fixed_handicap(char *s)
{
  int h;
  cin >> h;
  if ((h < 2) || (h > 9)) {
    error( "invalid number of stones" );
    return 1;
  }
  thegame.blackHandicap = h;
  place_fixed_handicap();
  // note: handicapList begins with a space
  cout << "=" << id << handicapList << endl << endl;
  return 1;
}


// Other commands
static int
gtp_get_random_seed(char *s)
{
  cout << "=" << id << " " << thegame.RANDOM_SEED << endl << endl;
  return 1;
}
static int
gtp_set_random_seed(char *s)
{
  int seed;
  if ( !(cin >> seed) ) {
      error( "unacceptable seed parameter" );
    return 1;
  }
  thegame.RANDOM_SEED = seed;
  respond();
  return 1;
}
static int
gtp_final_score(char *s)
{
  estimatescore();
  float finalScore = thegame.territoryBlack + thegame.capturedWhite
                 - ( thegame.territoryWhite + thegame.capturedBlack + thegame.komi );
  cout << "=" << id << " ";
  cout << (finalScore > 0?"B+":"W+");
  cout << setiosflags( ios::fixed ) << setprecision(1);
  cout << ((finalScore > 0) ? finalScore : -finalScore);
  cout << endl << endl;
  return 1;
}


// Miscellaneous functions

void respond(char *result) {
  cout << "=" << id << " " << result << endl << endl;
}

void error(char *error_message) {
  cout << "?" << id << " " << error_message << endl << endl;
}
