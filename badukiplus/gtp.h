/***************************************************************************
                        gtp.h  -  header file for GTP
                          -------------------------
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

#ifndef GTP_H
#define GTP_H

/* Whether the GTP command was successful. */
#define GTP_SUCCESS  0
#define GTP_FAILURE  1

/* Function pointer for callback functions. */
typedef int (*gtp_fn_ptr)(char *s);

/* Elements in the array of commands required by gtp_main_loop. */
struct gtp_command {
  const char *name;
  gtp_fn_ptr function;
};

#endif // GTP_H
