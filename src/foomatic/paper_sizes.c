/*
 * "$Id: paper_sizes.c,v 1.2 2001/04/21 21:51:28 rlk Exp $"
 *
 *   Dump the per-printer options for Grant Taylor's *-omatic database
 *
 *   Copyright 2000 Robert Krawitz (rlk@alum.mit.edu)
 *
 *   This program is free software; you can redistribute it and/or modify it
 *   under the terms of the GNU General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option)
 *   any later version.
 *
 *   This program is distributed in the hope that it will be useful, but
 *   WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 *   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *   for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#ifdef INCLUDE_GIMP_PRINT_H
#include INCLUDE_GIMP_PRINT_H
#else
#include <gimp-print.h>
#endif
#include "../../lib/libprintut.h"

char *params[] =
{
  "PageSize",
  "Resolution",
  "InkType",
  "MediaType",
  "InputSlot"
};

int nparams = sizeof(params) / sizeof(const char *);

int
main(int argc, char **argv)
{
  int i;
  for (i = 0; i < stp_known_papersizes(); i++)
    {
      const stp_papersize_t p = stp_get_papersize_by_index(i);
      printf("%s %d %d\n", stp_papersize_get_name(p),
	     stp_papersize_get_width(p),
	     stp_papersize_get_height(p));
    }
  return 0;
}
