/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : Sun Oct 13 21:33:08 HKT 2002
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <cstdlib>
#include <iostream>
#include <cstring>

using namespace std;

/*  main.cpp
 *  Initializes and interfaces with the Go playing engine
 *  This program is based on the original Baduki program and
 *  on Wallyplus and Badukiplus, two other simple Go engines.
 */

#include "game.h"

extern int gtp();
extern game thegame;

extern void fatal(char *);
extern void panic(char *);

extern int mymove();
extern int enemymove();

extern void initgame();
extern void score();

/* strings used to identify program */
char *progname = PACKAGE;
char *proginfo = "A simple program to play Go";
char *version  = VERSION;
string BadukiHelp ( "Usage: badukiplus [-s n] [-opts]"
		    "\n where n is an optional board size from 9 to 19"
		    "\n(default is 9)"
		    "\nMain Options:"
"\n	-handicap m		Specifies handicap of m stones"
"\n			Default is 0"
"\n	-komi p		Specifies komi"
"\n			Default is 0"
"\n	-white		Forces Badukiplus to play White"
"\n			Default is Badukiplus plays Black"
"\n	-gtp		Forces the playing mode to 'gtp'"
"\n			Default is console playing mode"
"\n\nInformative Output:"
"\n	-v, --version	Display the version of Badukiplus"
"\n	-h, --help	Display this help message\n" );

/*--------------  Prototypes  ------------------*/

void fatal(char *error_message);
void panic(char *panic_message);

/***************************************************************************
 *                                                                         *
 *                              Main Function                             *
 *                                                                         *
 ***************************************************************************
 */

int main( int argc, char *argv[] )
{
  bool playgtp = false;        // assume we are playing on the console

  thegame.RANDOM_SEED = time(NULL);     // Baduki does use rand()

  thegame.BadukiColor = BLACK; // Badukiplus plays Black by default

  thegame.boardSize = 19;      // Size of board is 19x19 by default for Baduki

  thegame.blackHandicap = 0;   // number of handicap stones granted to Black

  thegame.komi = 0;            // komi for this game


  argv++;            /* skip program name */

/* Parse command line arguments */

  for( ; --argc; ++argv) {

    // GTP mode ?
    if(0==strcmp(*argv,"-gtp")) {
      playgtp = true;
      break;    // no need to parse any other arguments
    }

    // Board size ?
    if (0==strcmp(*argv, "-s")) {
      ++argv;
      if (atoi(*argv)) {
        thegame.boardSize = atoi(*argv);
        if(thegame.boardSize < 9 || thegame.boardSize > 19)
          fatal("board size out of range on input line");
        --argc;
      }
      else
        fatal("board size not found");
    }

    // Black handicap ?
    if (0==strcmp(*argv, "-handicap")) {
      ++argv;
      if (atoi(*argv)) {
        thegame.blackHandicap = atoi(*argv);
        if(thegame.blackHandicap < 2 || thegame.blackHandicap > 17)
             fatal("handicap out of range on input line");
        --argc;
      }
      else
        fatal("handicap not specified");
    }
     // Komi ?
    if (0==strcmp(*argv, "-komi")) {
      ++argv;
      if (atof(*argv)){
        thegame.komi = atof(*argv);
        --argc;
      }
      else
        fatal("komi not specified");
    }

    // Baduki plays white ?
    else if(0==strcmp(*argv,"-white"))
      thegame.BadukiColor = WHITE;

    // Display program name and version and exit ?
    else if((0==strcmp(*argv,"-v")) || (0==strcmp(*argv,"--version"))) {
      cout << progname << endl << proginfo << endl << "Version " << version << endl;
      exit( EXIT_SUCCESS );
    }
    // Display short help and exit ?
    else if((0==strcmp(*argv,"-h")) || (0==strcmp(*argv,"--help"))) {
      cout << progname << endl << proginfo << endl << "Version ";
      cout << version << endl << BadukiHelp;
      exit( EXIT_SUCCESS );
    }
  }

  if (playgtp){
       if ( gtp() ) // GTP was called, did it quit or abort?
         panic("GTP aborted unexpectedly");
  }
  else {            // play a game on the console

/*Make a brief explanation.*/
       cout << "This program plays (poorly) the ancient game of Go, also \n";
       cout << "known as wei-chi." << endl;

/*Play the game.*/
       initgame();

       int rvalue = PLAY_MOVE;  /*return values from enemymove() or mymove()*/

       // Decide who plays first
       if(thegame.blackHandicap < 2)
         thegame.pla= BLACK;
       else
         thegame.pla= WHITE;

       // Start playing
       do {
         if(thegame.pla==thegame.BadukiColor)
           rvalue=mymove();
         else
           rvalue=enemymove();
       }
       while(rvalue!=BOTHPASS && rvalue!=RESIGN);

       score();
  }
  exit( EXIT_SUCCESS );         // Game over!
}

void fatal(char *error_message) {

    /*Print an error message and abort program.*/

    cerr << "\n?!fatal error--" << error_message << endl;
    exit(EXIT_FAILURE);
}


void panic(char *panic_message) {

    /*Fatal error routine reserved for error conditions that ought not happen.*/

    cerr << "\n?!panic--" << panic_message << endl;
    exit(EXIT_FAILURE);
}

