/*
 * "$Id: printers.c,v 1.1.2.1 2002/10/21 01:15:31 rlk Exp $"
 *
 *   Print plug-in driver utility functions for the GIMP.
 *
 *   Copyright 1997-2000 Michael Sweet (mike@easysw.com) and
 *	Robert Krawitz (rlk@alum.mit.edu)
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

/*
 * This file must include only standard C header files.  The core code must
 * compile on generic platforms that don't support glib, gimp, gtk, etc.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <gimp-print/gimp-print.h>
#include "gimp-print-internal.h"
#include <gimp-print/gimp-print-intl-internal.h>
#include <math.h>
#include <limits.h>
#include <string.h>
#include "printer.h"

#define FMIN(a, b) ((a) < (b) ? (a) : (b))

/*
 * The list of printers has been moved to printers.c
 */
#include "print-printers.c"

int
stp_known_printers(void)
{
  return printer_count;
}

const stp_printer_t
stp_get_printer_by_index(int idx)
{
  if (idx < 0 || idx >= printer_count)
    return NULL;
  return (stp_printer_t) &(printers[idx]);
}

const stp_printer_t
stp_get_printer_by_long_name(const char *long_name)
{
  const stp_internal_printer_t *val = &(printers[0]);
  int i;
  if (!long_name)
    return NULL;
  for (i = 0; i < stp_known_printers(); i++)
    {
      if (!strcmp(val->long_name, long_name))
	return (stp_printer_t) val;
      val++;
    }
  return NULL;
}

const stp_printer_t
stp_get_printer_by_driver(const char *driver)
{
  const stp_internal_printer_t *val = &(printers[0]);
  int i;
  if (!driver)
    return NULL;
  for (i = 0; i < stp_known_printers(); i++)
    {
      if (!strcmp(val->driver, driver))
	return (stp_printer_t) val;
      val++;
    }
  return NULL;
}

int
stp_get_printer_index_by_driver(const char *driver)
{
  int idx = 0;
  const stp_internal_printer_t *val = &(printers[0]);
  if (!driver)
    return -1;
  for (idx = 0; idx < stp_known_printers(); idx++)
    {
      if (!strcmp(val->driver, driver))
	return idx;
      val++;
    }
  return -1;
}

const char *
stp_printer_get_long_name(const stp_printer_t p)
{
  const stp_internal_printer_t *val = (const stp_internal_printer_t *) p;
  return val->long_name;
}

const char *
stp_printer_get_driver(const stp_printer_t p)
{
  const stp_internal_printer_t *val = (const stp_internal_printer_t *) p;
  return val->driver;
}

int
stp_printer_get_model(const stp_printer_t p)
{
  const stp_internal_printer_t *val = (const stp_internal_printer_t *) p;
  return val->model;
}

const stp_printfuncs_t *
stp_printer_get_printfuncs(const stp_printer_t p)
{
  const stp_internal_printer_t *val = (const stp_internal_printer_t *) p;
  return val->printfuncs;
}

const stp_vars_t
stp_printer_get_printvars(const stp_printer_t p)
{
  const stp_internal_printer_t *val = (const stp_internal_printer_t *) p;
  return (stp_vars_t) &(val->printvars);
}
