/*
 * "$Id: printer.h,v 1.1.2.1 2002/10/21 01:15:31 rlk Exp $"
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

typedef struct stp_internal_option
{
  char *name;
  size_t length;
  char *data;
  struct stp_internal_option *next;
  struct stp_internal_option *prev;
} stp_internal_option_t;

typedef struct					/* Plug-in variables */
{
  const char	*output_to,	/* Name of file or command to print to */
	*driver,		/* Name of printer "driver" */
	*ppd_file,		/* PPD file */
        *resolution,		/* Resolution */
	*media_size,		/* Media size */
	*media_type,		/* Media type */
	*media_source,		/* Media source */
	*ink_type,		/* Ink or cartridge */
	*dither_algorithm;	/* Dithering algorithm */
  int	output_type;		/* Color or grayscale output */
  float	brightness;		/* Output brightness */
  float	scaling;		/* Scaling, percent of printable area */
  int	orientation,		/* Orientation - 0 = port., 1 = land.,
				   -1 = auto */
	left,			/* Offset from lower-lefthand corner, points */
	top;			/* ... */
  float gamma;                  /* Gamma */
  float contrast,		/* Output Contrast */
	cyan,			/* Output red level */
	magenta,		/* Output green level */
	yellow;			/* Output blue level */
  float	saturation;		/* Output saturation */
  float	density;		/* Maximum output density */
  int	image_type;		/* Image type (line art etc.) */
  int	unit;			/* Units for preview area 0=Inch 1=Metric */
  float app_gamma;		/* Application gamma */
  int	page_width;		/* Width of page in points */
  int	page_height;		/* Height of page in points */
  int	input_color_model;	/* Color model for this device */
  int	output_color_model;	/* Color model for this device */
  void  *lut;			/* Look-up table */
  void  *driver_data;		/* Private data of the driver */
  unsigned char *cmap;		/* Color map */
  void (*outfunc)(void *data, const char *buffer, size_t bytes);
  void *outdata;
  void (*errfunc)(void *data, const char *buffer, size_t bytes);
  void *errdata;
  stp_internal_option_t *options;
  int verified;			/* Ensure that params are OK! */
} stp_internal_vars_t;

typedef struct stp_internal_printer
{
  const char	*long_name,	/* Long name for UI */
		*driver;	/* Short name for printrc file */
  int	model;			/* Model number */
  const stp_printfuncs_t *printfuncs;
  stp_internal_vars_t printvars;
} stp_internal_printer_t;
