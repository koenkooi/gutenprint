/*
 * "$Id: image.c,v 1.3.16.2 2004/02/22 04:05:47 rlk Exp $"
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <gimp-print/gimp-print.h>
#include "gimp-print-internal.h"

void
stpi_image_init(stp_image_t *image)
{
  if (image->init)
    image->init(image);
}

void
stpi_image_reset(stp_image_t *image)
{
  if (image->reset)
    image->reset(image);
}

int
stpi_image_width(stp_image_t *image)
{
  return image->width(image);
}

int
stpi_image_height(stp_image_t *image)
{
  return image->height(image);
}

stp_image_status_t
stpi_image_get_row(stp_image_t *image, unsigned char *data,
		  size_t byte_limit, int row)
{
  return image->get_row(image, data, byte_limit, row);
}

const char *
stpi_image_get_appname(stp_image_t *image)
{
  if (image->get_appname)
    return image->get_appname(image);
  else
    return NULL;
}

void
stpi_image_conclude(stp_image_t *image)
{
  if (image->conclude)
    image->conclude(image);
}
