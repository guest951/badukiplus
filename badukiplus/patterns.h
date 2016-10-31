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

/* $Id: patterns.h,v 1.12 1999/04/01 20:24:58 artist Exp artist $ */

#define MAX_PATTERN_SIZE	37	/* x_size * y_size */

#define PT_WHOLE	(1 << 0)
#define PT_CENTER	(1 << 1)
#define PT_SIDE		(1 << 2)
#define PT_CORNER	(1 << 3)
#define PT_IGNORE_D1	(1 << 4)
#define PT_IGNORE_D2	(1 << 5)
#define PT_IGNORE_D3	(1 << 6)
#define PT_IGNORE_D4	(1 << 7)
#define PT_IGNORE_D5	(1 << 8)
#define PT_IGNORE_D6	(1 << 9)
#define PT_IGNORE_D7	(1 << 10)

#define P_IGNORE_D	3

struct pattern_type {
  int id;
  int type;
  int x_size;
  int y_size;
  int x_pos;
  int y_pos;
  float value;
  char pat[MAX_PATTERN_SIZE];
};

float pattern_value(int x, int y, struct pattern_type pat, int dir, int *px, int *py);
int get_px(int x, int rx, int ry, struct pattern_type pat, int dir);
int get_py(int y, int rx, int ry, struct pattern_type pat, int dir);


#define PX(a, b)	get_px(x, a, b, pat, dir)
#define PY(a, b)	get_py(y, a, b, pat, dir)
#define PXY(a, b)	get_px(x, a, b, pat, dir)][get_py(y, a, b, pat, dir)
#define PCXY(a, b)	get_px(x, a, b, pat, dir), get_py(y, a, b, pat, dir)

#define STRING(a, b)	sList[SMap[stringIDs[PXY(a,b)]]]
#define LIBC(a, b)	sList[SMap[stringIDs[PXY(a,b)]]].libC
#define SSIZE(a, b)	sList[SMap[stringIDs[PXY(a,b)]]].size
#define LIBB(a, b)	libBoard[PXY(a,  b)]
#define PROT(a, b)	protPoints[PXY(a, b)]
#define BORD(a, b)	bord[PXY(a, b)]
#define SCORE(a, b)	scoreBoard[PXY(a, b)]
#define CLAIM(a, b)	claim[PXY(a, b)]
#define POW(a, b)	powerBoard[PXY(a,b)]
#define POW2(a, b)	powerBoard2[PXY(a,b)]
#define MYSTONES(a,b)	myStones[PXY(a,b)]
#define HISSTONES(a,b)	hisStones[PXY(a,b)]
#define SAFECON(a,b)	safeConnect[PXY(a,b)]
#define CONNECTMAP(a,b)	connectMap[PXY(a,b)]
#define CONNECTMAP2(a,b)	connectMap2[PXY(a,b)]
#define GID(a,b)	GIDMap[PXY(a,b)]
#define WGID(a,b)	wGIDMap[PXY(a,b)]


#define ISSAFE(a, b)	sList[SMap[stringIDs[PXY(a,b)]]].isSafe
#define ISDEAD(a, b)	sList[SMap[stringIDs[PXY(a,b)]]].isDead

#define TRY_PLAY(a, b, c)	tryPlay(PCXY(a,b), c)
#define KILLABLE(a, b)		killable(PCXY(a,b), &xx, &yy)
#define SAVEABLE(a, b)		saveable(PCXY(a,b), &xx, &yy)

#define PPOS(a, b)	*px = PX(a,b);\
			*py = PY(a,b)
#define PERCENT(a)	(rand() % 100) < a
