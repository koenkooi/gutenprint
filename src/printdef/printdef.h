/*
 * "$Id: printdef.h,v 1.6.2.1 2002/11/17 02:02:58 rlk Exp $"
 *
 *   I18N header file for the gimp-print plugin.
 *
 *   Copyright 1997-2000 Michael Sweet (mike@easysw.com),
 *	Robert Krawitz (rlk@alum.mit.edu) and Michael Natterer (mitch@gimp.org)
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

#define OUTPUT_GRAY		0	/* Grayscale output */
#define OUTPUT_COLOR		1	/* Color output */

typedef struct stp_printer
{
  char	long_name[256];
  char  short_name[64];
  int   family;
  int	model;
  int	output_type;		/* Color or grayscale output */
  float	brightness;		/* Output brightness */
  float gamma;                  /* Gamma */
  float contrast;		/* Output Contrast */
  float cyan;			/* Output red level */
  float magenta;		/* Output green level */
  float yellow;			/* Output blue level */
  float	saturation;		/* Output saturation */
  float	density;		/* Maximum output density */
} stp_printer_t;

typedef union yylv {
  int ival;
  double dval;
  char *sval;
} YYSTYPE;

extern YYSTYPE yylval;
extern stp_printer_t thePrinter;

#include "printdefy.h"

