/*
 * "$Id: printdef.h,v 1.7 2002/11/19 23:03:27 rleigh Exp $"
 *
 *   printdef XML parser header
 *
 *   Copyright 1997-2000 Michael Sweet (mike@easysw.com),
 *	Robert Krawitz (rlk@alum.mit.edu) and Michael Natterer (mitch@gimp.org)
 *   Copyright 2002 Roger Leigh (roger@whinlatter.uklinux.net)
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


typedef struct stp_printdef_printer
{
  const char	*long_name,			/* Long name for UI */
	        *driver,			/* Short name for printrc file */
                *family;                        /* Family driver name */
  int	        model;				/* Model number */
  const char    *printfuncs;
  stp_internal_vars_t printvars;
} stp_printdef_printer_t;

