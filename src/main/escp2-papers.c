/*
 * "$Id: escp2-papers.c,v 1.68.4.1 2005/06/13 02:08:41 rlk Exp $"
 *
 *   Print plug-in EPSON ESC/P2 driver for the GIMP.
 *
 *   Copyright 1997-2000 Michael Sweet (mike@easysw.com) and
 *	Robert Krawitz (rlk@alum.mit.edu)
 *
 *   This program is free software; you can redistribute it and/or modify it
 *   under the terms of the GNU eral Public License as published by the Free
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
#include <gutenprint/gutenprint.h>
#include "gutenprint-internal.h"
#include <gutenprint/gutenprint-intl-internal.h>
#include "print-escp2.h"

static const char standard_sat_adj[] =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<gutenprint>\n"
"<curve wrap=\"wrap\" type=\"linear\" gamma=\"0\">\n"
"<sequence count=\"48\" lower-bound=\"0\" upper-bound=\"4\">\n"
/* C */  "1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 "  /* B */
/* B */  "1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 "  /* M */
/* M */  "1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 "  /* R */
/* R */  "1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 "  /* Y */
/* Y */  "1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 "  /* G */
/* G */  "1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 "  /* C */
"</sequence>\n"
"</curve>\n"
"</gutenprint>\n";

static const char standard_lum_adj[] =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<gutenprint>\n"
"<curve wrap=\"wrap\" type=\"linear\" gamma=\"0\">\n"
"<sequence count=\"48\" lower-bound=\"0\" upper-bound=\"4\">\n"
/* C */  "0.56 0.58 0.62 0.68 0.73 0.78 0.82 0.85 "  /* B */
/* B */  "0.85 0.82 0.78 0.78 0.79 0.80 0.82 0.85 "  /* M */
/* M */  "0.87 0.90 0.94 0.97 1.00 1.00 1.00 1.00 "  /* R */
/* R */  "1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 "  /* Y */
/* Y */  "1.00 1.00 1.00 0.99 0.98 0.97 0.95 0.93 "  /* G */
/* G */  "0.90 0.76 0.65 0.58 0.58 0.57 0.56 0.56 "  /* C */
"</sequence>\n"
"</curve>\n"
"</gutenprint>\n";

static const char standard_hue_adj[] =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<gutenprint>\n"
"<curve wrap=\"wrap\" type=\"linear\" gamma=\"0\">\n"
"<sequence count=\"48\" lower-bound=\"-6\" upper-bound=\"6\">\n"
/* C */  "0.00 0.00 0.00 -.02 -.06 -.12 -.18 -.24 "  /* B */
/* B */  "-.30 -.28 -.28 -.26 -.24 -.22 -.20 -.20 "  /* M */
/* M */  "-.22 -.28 -.34 -.40 -.50 -.45 -.40 -.30 "  /* R */
/* R */  "-.12 -.07 -.04 -.02 0.00 0.00 0.00 0.00 "  /* Y */
/* Y */  "0.00 -.00 -.06 -.12 -.18 -.26 -.34 -.42 "  /* G */
/* G */  "-.50 -.44 -.38 -.31 -.25 -.20 -.13 -.06 "  /* C */
"</sequence>\n"
"</curve>\n"
"</gutenprint>\n";


static const char photo2_sat_adj[] =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<gutenprint>\n"
"<curve wrap=\"wrap\" type=\"linear\" gamma=\"0\">\n"
"<sequence count=\"48\" lower-bound=\"0\" upper-bound=\"4\">\n"
/* C */  "1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 "  /* B */
/* B */  "1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 "  /* M */
/* M */  "1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 "  /* R */
/* R */  "1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 "  /* Y */
/* Y */  "1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 "  /* G */
/* G */  "1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 "  /* C */
"</sequence>\n"
"</curve>\n"
"</gutenprint>\n";

static const char photo2_lum_adj[] =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<gutenprint>\n"
"<curve wrap=\"wrap\" type=\"linear\" gamma=\"0\">\n"
"<sequence count=\"48\" lower-bound=\"0\" upper-bound=\"4\">\n"
/* C */  "0.56 0.58 0.62 0.68 0.73 0.78 0.82 0.85 "  /* B */
/* B */  "0.85 0.82 0.78 0.78 0.79 0.80 0.82 0.85 "  /* M */
/* M */  "0.87 0.90 0.94 0.97 1.00 1.00 1.00 1.00 "  /* R */
/* R */  "1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 "  /* Y */
/* Y */  "1.00 1.00 1.00 0.99 0.98 0.97 0.95 0.93 "  /* G */
/* G */  "0.90 0.76 0.65 0.58 0.58 0.57 0.56 0.56 "  /* C */
"</sequence>\n"
"</curve>\n"
"</gutenprint>\n";

static const char photo2_hue_adj[] =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<gutenprint>\n"
"<curve wrap=\"wrap\" type=\"linear\" gamma=\"0\">\n"
"<sequence count=\"48\" lower-bound=\"-6\" upper-bound=\"6\">\n"
/* C */  "0.00 0.00 0.00 -.02 -.06 -.12 -.18 -.24 "  /* B */
/* B */  "-.30 -.28 -.28 -.26 -.24 -.22 -.20 -.20 "  /* M */
/* M */  "-.22 -.28 -.34 -.40 -.50 -.45 -.40 -.30 "  /* R */
/* R */  "-.12 -.07 -.04 -.02 0.00 0.00 0.00 0.00 "  /* Y */
/* Y */  "0.00 -.00 -.06 -.12 -.18 -.26 -.34 -.42 "  /* G */
/* G */  "-.50 -.44 -.38 -.31 -.25 -.20 -.13 -.06 "  /* C */
"</sequence>\n"
"</curve>\n"
"</gutenprint>\n";


static const char sp960_sat_adj[] =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<gutenprint>\n"
"<curve wrap=\"wrap\" type=\"linear\" gamma=\"0\">\n"
"<sequence count=\"48\" lower-bound=\"0\" upper-bound=\"4\">\n"
/* C */  "1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 "  /* B */
/* B */  "1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 "  /* M */
/* M */  "1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 "  /* R */
/* R */  "1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 "  /* Y */
/* Y */  "1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 "  /* G */
/* G */  "1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 "  /* C */
"</sequence>\n"
"</curve>\n"
"</gutenprint>\n";

static const char sp960_lum_adj[] =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<gutenprint>\n"
"<curve wrap=\"wrap\" type=\"linear\" gamma=\"0\">\n"
"<sequence count=\"48\" lower-bound=\"0\" upper-bound=\"4\">\n"
/* C */  "0.58 0.60 0.65 0.69 0.74 0.79 0.82 0.84 "  /* B */
/* B */  "0.86 0.81 0.76 0.76 0.78 0.79 0.83 0.86 "  /* M */
/* M */  "0.93 0.95 0.97 0.98 1.00 1.00 1.00 1.00 "  /* R */
/* R */  "1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 "  /* Y */
/* Y */  "1.00 0.99 0.98 0.97 0.96 0.94 0.93 0.89 "  /* G */
/* G */  "0.86 0.73 0.65 0.58 0.59 0.59 0.58 0.58 "  /* C */
"</sequence>\n"
"</curve>\n"
"</gutenprint>\n";

static const char sp960_hue_adj[] =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<gutenprint>\n"
"<curve wrap=\"wrap\" type=\"linear\" gamma=\"0\">\n"
"<sequence count=\"48\" lower-bound=\"-6\" upper-bound=\"6\">\n"
/* C */  "0.00 0.06 0.10 0.10 0.06 -.01 -.09 -.17 "  /* B */
/* B */  "-.25 -.28 -.28 -.26 -.24 -.22 -.20 -.20 "  /* M */
/* M */  "-.22 -.28 -.34 -.40 -.50 -.45 -.40 -.30 "  /* R */
/* R */  "-.22 -.13 -.04 -.02 0.00 0.00 0.00 0.00 "  /* Y */
/* Y */  "0.00 -.00 -.06 -.14 -.22 -.30 -.38 -.44 "  /* G */
/* G */  "-.50 -.44 -.38 -.31 -.25 -.20 -.13 -.06 "  /* C */
"</sequence>\n"
"</curve>\n"
"</gutenprint>\n";

static const char sp960_matte_sat_adj[] =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<gutenprint>\n"
"<curve wrap=\"wrap\" type=\"linear\" gamma=\"0\">\n"
"<sequence count=\"48\" lower-bound=\"0\" upper-bound=\"4\">\n"
/* C */  "1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 "  /* B */
/* B */  "1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 "  /* M */
/* M */  "1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 "  /* R */
/* R */  "1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 "  /* Y */
/* Y */  "1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 "  /* G */
/* G */  "1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 "  /* C */
"</sequence>\n"
"</curve>\n"
"</gutenprint>\n";

static const char sp960_matte_lum_adj[] =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<gutenprint>\n"
"<curve wrap=\"wrap\" type=\"linear\" gamma=\"0\">\n"
"<sequence count=\"48\" lower-bound=\"0\" upper-bound=\"4\">\n"
/* C */  "0.58 0.63 0.70 0.75 0.80 0.86 0.88 0.90 "  /* B */
/* B */  "0.90 0.83 0.78 0.78 0.78 0.79 0.83 0.86 "  /* M */
/* M */  "0.93 0.95 0.97 0.98 1.00 1.00 1.00 1.00 "  /* R */
/* R */  "1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 "  /* Y */
/* Y */  "1.00 0.99 0.98 0.97 0.96 0.94 0.93 0.89 "  /* G */
/* G */  "0.86 0.73 0.65 0.58 0.59 0.59 0.58 0.58 "  /* C */
"</sequence>\n"
"</curve>\n"
"</gutenprint>\n";

static const char sp960_matte_hue_adj[] =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<gutenprint>\n"
"<curve wrap=\"wrap\" type=\"linear\" gamma=\"0\">\n"
"<sequence count=\"48\" lower-bound=\"-6\" upper-bound=\"6\">\n"
/* C */  "0.00 -.02 -.04 -.06 -.12 -.18 -.25 -.30 "  /* B */
/* B */  "-.30 -.28 -.28 -.26 -.24 -.22 -.20 -.20 "  /* M */
/* M */  "-.22 -.28 -.34 -.40 -.50 -.45 -.40 -.30 "  /* R */
/* R */  "-.22 -.13 -.04 -.02 0.00 0.00 0.00 0.00 "  /* Y */
/* Y */  "0.00 -.00 -.06 -.14 -.22 -.30 -.38 -.44 "  /* G */
/* G */  "-.50 -.44 -.38 -.31 -.25 -.20 -.13 -.06 "  /* C */
"</sequence>\n"
"</curve>\n"
"</gutenprint>\n";


static const char ultra_matte_sat_adj[] =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<gutenprint>\n"
"<curve wrap=\"wrap\" type=\"linear\" gamma=\"0\">\n"
"<sequence count=\"48\" lower-bound=\"0\" upper-bound=\"4\">\n"
/* C */  "1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 "  /* B */
/* B */  "1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 "  /* M */
/* M */  "1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 "  /* R */
/* R */  "1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 "  /* Y */
/* Y */  "1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 "  /* G */
/* G */  "1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 "  /* C */
"</sequence>\n"
"</curve>\n"
"</gutenprint>\n";

static const char ultra_matte_lum_adj[] =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<gutenprint>\n"
"<curve wrap=\"wrap\" type=\"linear\" gamma=\"0\">\n"
"<sequence count=\"48\" lower-bound=\"0\" upper-bound=\"4\">\n"
/* C */  "0.49 0.51 0.55 0.61 0.67 0.71 0.76 0.79 "  /* B */
/* B */  "0.83 0.80 0.76 0.76 0.78 0.79 0.83 0.86 "  /* M */
/* M */  "0.93 0.95 0.97 0.97 0.97 0.97 0.96 0.96 "  /* R */
/* R */  "0.96 0.97 0.97 0.98 0.99 1.00 1.00 1.00 "  /* Y */
/* Y */  "1.00 0.98 0.97 0.95 0.94 0.93 0.90 0.86 "  /* G */
/* G */  "0.82 0.69 0.60 0.54 0.52 0.51 0.50 0.49 "  /* C */
"</sequence>\n"
"</curve>\n"
"</gutenprint>\n";

static const char ultra_matte_hue_adj[] =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<gutenprint>\n"
"<curve wrap=\"wrap\" type=\"linear\" gamma=\"0\">\n"
"<sequence count=\"48\" lower-bound=\"-6\" upper-bound=\"6\">\n"
/* C */  "0.00 0.06 0.10 0.10 0.06 -.01 -.09 -.17 "  /* B */
/* B */  "-.25 -.28 -.28 -.26 -.24 -.22 -.20 -.20 "  /* M */
/* M */  "-.22 -.28 -.34 -.40 -.50 -.40 -.30 -.20 "  /* R */
/* R */  "-.12 -.07 -.04 -.02 0.00 0.00 0.00 0.00 "  /* Y */
/* Y */  "0.00 -.00 -.06 -.14 -.22 -.30 -.38 -.44 "  /* G */
/* G */  "-.50 -.44 -.38 -.31 -.25 -.20 -.13 -.06 "  /* C */
"</sequence>\n"
"</curve>\n"
"</gutenprint>\n";

static const char ultra_glossy_sat_adj[] =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<gutenprint>\n"
"<curve wrap=\"wrap\" type=\"linear\" gamma=\"0\">\n"
"<sequence count=\"48\" lower-bound=\"0\" upper-bound=\"4\">\n"
/* C */  "1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 "  /* B */
/* B */  "1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 "  /* M */
/* M */  "1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 "  /* R */
/* R */  "1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 "  /* Y */
/* Y */  "1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 "  /* G */
/* G */  "1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 "  /* C */
"</sequence>\n"
"</curve>\n"
"</gutenprint>\n";

static const char ultra_glossy_lum_adj[] =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<gutenprint>\n"
"<curve wrap=\"wrap\" type=\"linear\" gamma=\"0\">\n"
"<sequence count=\"48\" lower-bound=\"0\" upper-bound=\"4\">\n"
/* C */  "0.49 0.53 0.60 0.64 0.69 0.73 0.77 0.80 "  /* B */
/* B */  "0.84 0.81 0.77 0.77 0.78 0.80 0.84 0.87 "  /* M */
/* M */  "0.93 0.95 0.97 0.98 0.98 0.97 0.96 0.96 "  /* R */
/* R */  "0.96 0.97 0.98 0.98 0.99 1.00 1.00 1.00 "  /* Y */
/* Y */  "1.00 0.98 0.97 0.96 0.95 0.93 0.90 0.87 "  /* G */
/* G */  "0.83 0.69 0.61 0.55 0.53 0.52 0.50 0.49 "  /* C */
"</sequence>\n"
"</curve>\n"
"</gutenprint>\n";

static const char ultra_glossy_hue_adj[] =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<gutenprint>\n"
"<curve wrap=\"wrap\" type=\"linear\" gamma=\"0\">\n"
"<sequence count=\"48\" lower-bound=\"-6\" upper-bound=\"6\">\n"
/* C */  "0.00 0.06 0.10 0.10 0.06 -.01 -.09 -.17 "  /* B */
/* B */  "-.25 -.28 -.28 -.26 -.24 -.22 -.20 -.20 "  /* M */
/* M */  "-.22 -.28 -.34 -.40 -.50 -.40 -.30 -.20 "  /* R */
/* R */  "-.12 -.07 -.04 -.02 0.00 0.00 0.00 0.00 "  /* Y */
/* Y */  "0.00 -.00 -.06 -.14 -.22 -.30 -.38 -.44 "  /* G */
/* G */  "-.50 -.44 -.38 -.31 -.25 -.20 -.13 -.06 "  /* C */
"</sequence>\n"
"</curve>\n"
"</gutenprint>\n";


static const char r800_matte_sat_adj[] =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<gutenprint>\n"
"<curve wrap=\"wrap\" type=\"linear\" gamma=\"0\">\n"
"<sequence count=\"48\" lower-bound=\"0\" upper-bound=\"4\">\n"
/* C */  "1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 "  /* B */
/* B */  "1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 "  /* M */
/* M */  "1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 "  /* R */
/* R */  "1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 "  /* Y */
/* Y */  "1.00 1.03 1.06 1.09 1.12 1.15 1.18 1.20 "  /* G */
/* G */  "1.20 1.15 1.10 1.05 1.00 1.00 1.00 1.00 "  /* C */
"</sequence>\n"
"</curve>\n"
"</gutenprint>\n";

static const char r800_matte_lum_adj[] =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<gutenprint>\n"
"<curve wrap=\"wrap\" type=\"linear\" gamma=\"0\">\n"
"<sequence count=\"48\" lower-bound=\"0\" upper-bound=\"4\">\n"
/* C */  "0.63 0.66 0.71 0.78 0.82 0.89 0.95 0.98 "  /* B */
/* B */  "0.98 0.95 0.88 0.79 0.69 0.69 0.69 0.69 "  /* M */
/* M */  "0.75 0.88 0.97 1.00 1.00 0.98 0.96 0.96 "  /* R */
/* R */  "0.96 0.97 0.98 0.98 0.99 1.00 1.00 1.00 "  /* Y */
/* Y */  "1.00 0.98 0.96 0.94 0.92 0.89 0.85 0.80 "  /* G */
/* G */  "0.75 0.74 0.72 0.71 0.69 0.65 0.63 0.63 "  /* C */
"</sequence>\n"
"</curve>\n"
"</gutenprint>\n";

static const char r800_matte_hue_adj[] =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<gutenprint>\n"
"<curve wrap=\"wrap\" type=\"linear\" gamma=\"0\">\n"
"<sequence count=\"48\" lower-bound=\"-6\" upper-bound=\"6\">\n"
/* C */  "0.00 -.07 -.10 -.15 -.19 -.25 -.30 -.37 "  /* B */
/* B */  "-.42 -.50 -.58 -.58 -.50 -.40 -.35 -.30 "  /* M */
/* M */  "-.27 -.25 -.22 -.19 -.14 -.10 -.05 0.00 "  /* R */
/* R */  "0.04 0.08 0.12 0.12 0.12 0.11 0.10 0.05 "  /* Y */
/* Y */  "0.00 0.02 0.04 0.06 0.06 0.04 0.02 0.00 "  /* G */
/* G */  "-.00 -.05 -.10 -.15 -.15 -.10 -.05 -.00 "  /* C */
"</sequence>\n"
"</curve>\n"
"</gutenprint>\n";

static const char r800_glossy_sat_adj[] =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<gutenprint>\n"
"<curve wrap=\"wrap\" type=\"linear\" gamma=\"0\">\n"
"<sequence count=\"48\" lower-bound=\"0\" upper-bound=\"4\">\n"
/* C */  "1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 "  /* B */
/* B */  "1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 "  /* M */
/* M */  "1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 "  /* R */
/* R */  "1.00 1.00 1.00 1.00 1.00 1.00 1.00 1.00 "  /* Y */
/* Y */  "1.00 1.03 1.06 1.09 1.12 1.15 1.18 1.20 "  /* G */
/* G */  "1.20 1.15 1.10 1.05 1.00 1.00 1.00 1.00 "  /* C */
"</sequence>\n"
"</curve>\n"
"</gutenprint>\n";


static const char r800_glossy_lum_adj[] =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<gutenprint>\n"
"<curve wrap=\"wrap\" type=\"linear\" gamma=\"0\">\n"
"<sequence count=\"48\" lower-bound=\"0\" upper-bound=\"4\">\n"
/* C */  "0.63 0.66 0.69 0.71 0.74 0.77 0.79 0.79 "  /* B */
/* B */  "0.79 0.77 0.76 0.73 0.69 0.65 0.62 0.59 "  /* M */
/* M */  "0.57 0.57 0.60 0.64 0.64 0.64 0.78 0.84 "  /* R */
/* R */  "0.96 0.97 0.98 0.98 0.99 1.00 1.00 1.00 "  /* Y */
/* Y */  "1.00 0.98 0.96 0.94 0.92 0.89 0.85 0.80 "  /* G */
/* G */  "0.75 0.74 0.72 0.71 0.69 0.65 0.63 0.63 "  /* C */
"</sequence>\n"
"</curve>\n"
"</gutenprint>\n";

static const char r800_glossy_hue_adj[] =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<gutenprint>\n"
"<curve wrap=\"wrap\" type=\"linear\" gamma=\"0\">\n"
"<sequence count=\"48\" lower-bound=\"-6\" upper-bound=\"6\">\n"
/* C */  "0.00 0.00 0.00 0.00 0.00 -.02 -.04 -.06 "  /* B */
/* B */  "-.08 -.12 -.15 -.15 -.08 0.02 0.12 0.23 "  /* M */
/* M */  "0.35 0.35 0.32 0.29 0.26 0.23 0.19 0.12 "  /* R */
/* R */  "0.00 0.04 0.08 0.12 0.12 0.11 0.10 0.05 "  /* Y */
/* Y */  "0.00 0.02 0.04 0.06 0.06 0.04 0.02 0.00 "  /* G */
/* G */  "-.00 -.05 -.10 -.15 -.15 -.10 -.05 -.00 "  /* C */
"</sequence>\n"
"</curve>\n"
"</gutenprint>\n";

#define DECLARE_PAPERS(name)				\
const paperlist_t stpi_escp2_##name##_paper_list =	\
{							\
  #name,						\
  sizeof(name##_papers) / sizeof(paper_t),		\
  name##_papers						\
}

#define DECLARE_PAPER_ADJUSTMENTS(name)					  \
const paper_adjustment_list_t stpi_escp2_##name##_paper_adjustment_list = \
{									  \
  #name,								  \
  sizeof(name##_adjustments) / sizeof(paper_adjustment_t),		  \
  name##_adjustments							  \
}

static const paper_adjustment_t standard_adjustments[] =
{
  { "Plain", 0.615, .5, 1, .075, .9, 1, 1, 1, 1, 1, 1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "PlainFast", 0.615, .5, 1, .075, .9, 1, 1, 1, 1, 1, 1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "Postcard", 0.83, .5, 1, .075, .9, 1, 1, 1, 1, 1, 1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "GlossyFilm", 1.00, 1.0, 1, .15, .999, 1, 1, 1, 1, 1, 1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "Transparency", 1.00, .75, 1, .15, .999, 1, 1, 1, 1, 1, 1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "Envelope", 0.615, .5, 1, .075, .9, 1, 1, 1, 1, 1, 1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "BackFilm", 1.00, .75, 1, .15, .999, 1, 1, 1, 1, 1, 1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "Matte", 0.85, .8, 1.0, .15, .999, 1, 1, 1, 1, 1, 1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "MatteHeavy", 1.0, 1.0, 1, .15, .999, 1, 1, 1, 1, 1, 1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "Inkjet", 0.85, .5, 1, .10, .9, 1, 1, 1, 1, 1, 1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "Coated", 1.10, 1.0, 1, .15, .999, 1, 1, 1, 1, 1, 1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "Photo", 1.00, 1.0, 1, .15, .999, 1, 1, 1, 1, 1, 1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "GlossyPhoto", 1.10, 1.0, 1, .15, .999, 1, 1, 1, 1, 1, 1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "Semigloss", 1.00, 1.0, 1, .15, .999, 1, 1, 1, 1, 1, 1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "Luster", 1.00, 1.0, 1, .15, .999, 1, 1, 1, 1, 1, 1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "GlossyPaper", 1.00, 1.0, 1, .15, .999, 1, 1, 1, 1, 1, 1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "Ilford", 1.0, 1.0, 1, .15, 1.35, 1, 1, 1, 1, 1, 1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj  },
  { "ColorLife", 1.00, 1.0, 1, .15, .9, 1, 1, 1, 1, 1, 1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "Other", 0.615, .5, 1, .075, .9, 1, 1, 1, 1, 1, 1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
};

DECLARE_PAPER_ADJUSTMENTS(standard);

static const paper_adjustment_t photo_adjustments[] =
{
  { "Plain", 0.615, .25, 1, .15, .9, 1, 1, 1, 1, 1, 1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "PlainFast", 0.615, .25, 1, .15, .9, 1, 1, 1, 1, 1, 1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "Postcard", 0.83, .25, 1, .15, .9, 1, 1, 1, 1, 1, 1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "GlossyFilm", 1.00, 1.0, 1, .2, .999, 1, 1, 1, 1, 1, 1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "Transparency", 1.00, .75, 1, .2, .999, 1, 1, 1, 1, 1, 1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "Envelope", 0.615, .25, 1, .15, .9, 1, 1, 1, 1, 1, 1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "BackFilm", 1.00, .75, 1, .2, .999, 1, 1, 1, 1, 1, 1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "Matte", 0.85, .8, 1.0, .2, .999, 1, 1, 1, 1, 1, 1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "MatteHeavy", 1.0, 1.0, 1, .35, .999, 1, 1, 1, 1, 1, 1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "Inkjet", 0.85, .375, 1, .2, .9, 1, 1, 1, 1, 1, 1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "Coated", 1.10, 1.0, 1, .35, .999, 1, 1, 1, 1, 1, 1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "Photo", 1.00, 1.00, 1, .35, .999, 1, 1, 1, 1, 1, 1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "GlossyPhoto", 1.10, 1.0, 1, .35, .999, 1, 1, 1, 1, 1, 1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "Semigloss", 1.00, 1.0, 1, .35, .999, 1, 1, 1, 1, 1, 1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "Luster", 1.00, 1.0, 1, .35, .999, 1, 1, 1, 1, 1, 1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "GlossyPaper", 1.00, 1.0, 1, .35, .999, 1, 1, 1, 1, 1, 1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "Ilford", 1.0, 1.0, 1, .35, 1.35, 1, 1, 1, 1, 1, 1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj  },
  { "ColorLife", 1.00, 1.0, 1, .35, .9, 1, 1, 1, 1, 1, 1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "Other", 0.615, .25, 1, .15, .9, 1, 1, 1, 1, 1, 1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
};

DECLARE_PAPER_ADJUSTMENTS(photo);

static const paper_adjustment_t photo2_adjustments[] =
{
  { "Plain", 0.738, .5, 0.5, .1, .9, 1, 1, 1, 1, 1, 1.2,
    photo2_hue_adj, photo2_lum_adj, photo2_sat_adj },
  { "PlainFast", 0.738, .5, 0.5, .1, .9, 1, 1, 1, 1, 1, 1.2,
    photo2_hue_adj, photo2_lum_adj, photo2_sat_adj },
  { "Postcard", 0.83, .5, 0.5, .1, .9, 1, 1, 1, 1, 1, 1.2,
    photo2_hue_adj, photo2_lum_adj, photo2_sat_adj },
  { "GlossyFilm", 1.00, .5, 0.5, .2, .999, 1, 1, 1, 1, 1, 1,
    photo2_hue_adj, photo2_lum_adj, photo2_sat_adj },
  { "Transparency", 1.00, .5, 0.25, .2, .999, 1, 1, 1, 1, 1, 1,
    photo2_hue_adj, photo2_lum_adj, photo2_sat_adj },
  { "Envelope", 0.738, .5, 0.5, .1, .9, 1, 1, 1, 1, 1, 1.2,
    photo2_hue_adj, photo2_lum_adj, photo2_sat_adj },
  { "BackFilm", 1.00, .5, 0.25, .2, .999, 1, 1, 1, 1, 1, 1,
    photo2_hue_adj, photo2_lum_adj, photo2_sat_adj },
  { "Matte", 0.85, .5, 0.4, .3, .999, 1, 1, 1, 1, 1, 1.1,
    photo2_hue_adj, photo2_lum_adj, photo2_sat_adj },
  { "MatteHeavy", 0.85, .5, .3, .2, .999, 1, 1, 1, 1, 1, 1.1,
    photo2_hue_adj, photo2_lum_adj, photo2_sat_adj },
  { "Inkjet", 0.85, .5, 0.5, .15, .9, 1, 1, 1, 1, 1, 1.1,
    photo2_hue_adj, photo2_lum_adj, photo2_sat_adj },
  { "Coated", 1.2, .5, .25, .15, .999, .89, 1, 1, .9, 1, 1.2,
    photo2_hue_adj, photo2_lum_adj, photo2_sat_adj },
  { "Photo", 1.00, .5, 0.25, .2, .999, 1, 1, 1, 1, 1, 1.1,
    photo2_hue_adj, photo2_lum_adj, photo2_sat_adj },
  { "GlossyPhoto", 1.0, .5, 0.5, .3, .999, .9, .98, 1, .9, 1, 1.0,
    photo2_hue_adj, photo2_lum_adj, photo2_sat_adj },
  { "Semigloss", 1.0, .5, 0.5, .3, .999, .9, .98, 1, .9, 1, 1.0,
    photo2_hue_adj, photo2_lum_adj, photo2_sat_adj },
  { "Luster", 1.0, .5, 0.5, .3, .999, .9, .98, 1, .9, 1, 1.0,
    photo2_hue_adj, photo2_lum_adj, photo2_sat_adj },
  { "GlossyPaper", 1.00, .5, 0.25, .2, .999, 1, 1, 1, 1, 1, 1,
    photo2_hue_adj, photo2_lum_adj, photo2_sat_adj },
  { "Ilford", .85, .5, 0.25, .2, .999, 1, 1, 1, 1, 1, 1,
    photo2_hue_adj, photo2_lum_adj, photo2_sat_adj  },
  { "ColorLife", 1.00, .5, 0.25, .2, .9, 1, 1, 1, 1, 1, 1.1,
    photo2_hue_adj, photo2_lum_adj, photo2_sat_adj },
  { "Other", 0.738, .5, 0.5, .1, .9, 1, 1, 1, 1, 1, 1.2,
    photo2_hue_adj, photo2_lum_adj, photo2_sat_adj },
};

DECLARE_PAPER_ADJUSTMENTS(photo2);

static const paper_adjustment_t photo3_adjustments[] =
{
  { "Plain", 0.738, .5, 0.75, .1, .9, 1, 1, 1, 1, 1, 1.2,
    photo2_hue_adj, photo2_lum_adj, photo2_sat_adj },
  { "PlainFast", 0.738, .5, 0.75, .1, .9, 1, 1, 1, 1, 1, 1.2,
    photo2_hue_adj, photo2_lum_adj, photo2_sat_adj },
  { "Postcard", 0.83, .5, 0.75, .1, .9, 1, 1, 1, 1, 1, 1.2,
    photo2_hue_adj, photo2_lum_adj, photo2_sat_adj },
  { "GlossyFilm", 1.00, .5, 0.75, .2, .999, 1, 1, 1, 1, 1, 1,
    photo2_hue_adj, photo2_lum_adj, photo2_sat_adj },
  { "Transparency", 1.00, .5, 0.75, .2, .999, 1, 1, 1, 1, 1, 1,
    photo2_hue_adj, photo2_lum_adj, photo2_sat_adj },
  { "Envelope", 0.738, .5, 0.75, .1, .9, 1, 1, 1, 1, 1, 1.2,
    photo2_hue_adj, photo2_lum_adj, photo2_sat_adj },
  { "BackFilm", 1.00, .5, 0.75, .2, .999, 1, 1, 1, 1, 1, 1,
    photo2_hue_adj, photo2_lum_adj, photo2_sat_adj },
  { "Matte", 0.85, .75, 0.75, .3, .999, 1, 1, 1, 1, 1, 1.1,
    photo2_hue_adj, photo2_lum_adj, photo2_sat_adj },
  { "MatteHeavy", 0.85, .75, .3, .2, .999, 1, 1, 1, 1, 1, 1.1,
    photo2_hue_adj, photo2_lum_adj, photo2_sat_adj },
  { "Inkjet", 0.85, .5, 0.75, .15, .9, 1, 1, 1, 1, 1, 1.1,
    photo2_hue_adj, photo2_lum_adj, photo2_sat_adj },
  { "Coated", 1.2, .5, .75, .15, .999, 1, 1, 1, 1, 1, 1.2,
    photo2_hue_adj, photo2_lum_adj, photo2_sat_adj },
  { "Photo", 1.00, .5, 0.75, .2, .999, 1, 1, 1, 1, 1, 1.1,
    photo2_hue_adj, photo2_lum_adj, photo2_sat_adj },
  { "GlossyPhoto", 1.0, .25, 0.5, .3, .999, 1, 1, 1, .9, 1, 1.0,
    photo2_hue_adj, photo2_lum_adj, photo2_sat_adj },
  { "Semigloss", 1.0, .25, 0.5, .3, .999, 1, 1, 1, .9, 1, 1.0,
    photo2_hue_adj, photo2_lum_adj, photo2_sat_adj },
  { "Luster", 1.0, .25, 0.5, .3, .999, 1, 1, 1, .9, 1, 1.0,
    photo2_hue_adj, photo2_lum_adj, photo2_sat_adj },
  { "GlossyPaper", 1.00, .5, 0.75, .2, .999, 1, 1, 1, 1, 1, 1,
    photo2_hue_adj, photo2_lum_adj, photo2_sat_adj },
  { "Ilford", .85, .5, 0.75, .2, .999, 1, 1, 1, 1, 1, 1,
    photo2_hue_adj, photo2_lum_adj, photo2_sat_adj  },
  { "ColorLife", 1.00, .5, 0.75, .2, .9, 1, 1, 1, 1, 1, 1.1,
    photo2_hue_adj, photo2_lum_adj, photo2_sat_adj },
  { "Other", 0.738, .5, 0.75, .1, .9, 1, 1, 1, 1, 1, 1.2,
    photo2_hue_adj, photo2_lum_adj, photo2_sat_adj },
};

DECLARE_PAPER_ADJUSTMENTS(photo3);

static const paper_adjustment_t sp960_adjustments[] =
{
  { "Plain",        0.86, .2,  0.4, .1,   .9,   .9, 1, 1, 1, 1, 1,
    sp960_matte_hue_adj, sp960_matte_lum_adj, sp960_matte_sat_adj },
  { "PlainFast",    0.86, .2,  0.4, .1,   .9,   1, 1, 1, 1, 1, 1,
    sp960_matte_hue_adj, sp960_matte_lum_adj, sp960_matte_sat_adj },
  { "Postcard",     0.90, .2,  0.4, .1,   .9,   .9, 1, 1, 1, 1, 1,
    sp960_matte_hue_adj, sp960_matte_lum_adj, sp960_matte_sat_adj },
  { "GlossyFilm",   0.9,  .3,  0.4, .2,   .999, 1, 1, 1, 1, 1, 1,
    sp960_matte_hue_adj, sp960_matte_lum_adj, sp960_matte_sat_adj },
  { "Transparency", 0.9,  .2,  0.4, .1,   .9,   1, 1, 1, 1, 1, 1,
    sp960_matte_hue_adj, sp960_matte_lum_adj, sp960_matte_sat_adj },
  { "Envelope",     0.86, .2,  0.4, .1,   .9,   1, 1, 1, 1, 1, 1,
    sp960_matte_hue_adj, sp960_matte_lum_adj, sp960_matte_sat_adj },
  { "BackFilm",     0.9,  .2,  0.4, .1,   .9,   1, 1, 1, 1, 1, 1,
    sp960_matte_hue_adj, sp960_matte_lum_adj, sp960_matte_sat_adj },
  { "Matte",        0.9,  .25, 0.4, .2,   .9,   1, 1, 1, 1, 1, 1,
    sp960_matte_hue_adj, sp960_matte_lum_adj, sp960_matte_sat_adj },
  { "MatteHeavy",   0.9,  .3,  0.4, .2,   .999, 1, 1, 1, 1, 1, 1,
    sp960_matte_hue_adj, sp960_matte_lum_adj, sp960_matte_sat_adj },
  { "Inkjet",       0.9,  .2,  0.4, .15,  .9,   1, 1, 1, 1, 1, 1,
    sp960_matte_hue_adj, sp960_matte_lum_adj, sp960_matte_sat_adj },
  { "Coated",       0.9,  .3,  0.4, .2,   .999, 1, 1, 1, 1, 1, 1,
    sp960_matte_hue_adj, sp960_matte_lum_adj, sp960_matte_sat_adj },
  { "Photo",        0.9,  .3,  0.4, .2,   .999, 1, 1, 1, 1, 1, 1,
    sp960_matte_hue_adj, sp960_matte_lum_adj, sp960_matte_sat_adj },
  { "GlossyPhoto",  0.9,  .3,  0.4, .2,   .999, 1, 1, 1, 1, 1, 1,
    sp960_hue_adj, sp960_lum_adj, sp960_sat_adj },
  { "Semigloss",    0.9,  .3,  0.4, .2,   .999, 1, 1, 1, 1, 1, 1,
    sp960_hue_adj, sp960_lum_adj, sp960_sat_adj },
  { "Luster",       0.9,  .3,  0.4, .2,   .999, 1, 1, 1, 1, 1, 1,
    sp960_hue_adj, sp960_lum_adj, sp960_sat_adj },
  { "GlossyPaper",  0.9,  .3,  0.4, .15,  .9,   1, 1, 1, 1, 1, 1,
    sp960_matte_hue_adj, sp960_matte_lum_adj, sp960_matte_sat_adj },
  { "Ilford",       0.85, .3,  0.4, .15, 1.35,  1, 1, 1, 1, 1, 1,
    sp960_matte_hue_adj, sp960_matte_lum_adj, sp960_matte_sat_adj  },
  { "ColorLife",    0.9,  .3,  0.4, .15,  .9,   1, 1, 1, 1, 1, 1,
    sp960_matte_hue_adj, sp960_matte_lum_adj, sp960_matte_sat_adj },
  { "Other",        0.86, .2,  0.4, .1,   .9,   1, 1, 1, 1, 1, 1,
    sp960_matte_hue_adj, sp960_matte_lum_adj, sp960_matte_sat_adj },
};

DECLARE_PAPER_ADJUSTMENTS(sp960);

static const paper_adjustment_t ultrachrome_photo_adjustments[] =
{
  { "Plain", 0.72, .1, 1, .01, 1.5, 1, 1, 1, 1, 1, 1.2,
    ultra_matte_hue_adj, ultra_matte_lum_adj, ultra_matte_sat_adj },
  { "PlainFast", 0.72, .1, 1, .01, 1.5, 1, 1, 1, 1, 1, 1.2,
    ultra_matte_hue_adj, ultra_matte_lum_adj, ultra_matte_sat_adj },
  { "Postcard", 0.72, .1, 1, .01, 1.5, 1, 1, 1, 1, 1, 1.2,
    ultra_matte_hue_adj, ultra_matte_lum_adj, ultra_matte_sat_adj },
  { "GlossyFilm", 0.83, 1.0, 1, .01, 1.5, 1, 1, 1, 1, 1, 1.2,
    ultra_matte_hue_adj, ultra_matte_lum_adj, ultra_matte_sat_adj },
  { "Transparency", 0.83, .75, 1, .01, 1.5, 1, 1, 1, 1, 1, 1.2,
    ultra_matte_hue_adj, ultra_matte_lum_adj, ultra_matte_sat_adj },
  { "Envelope", 0.72, .1, 1, .01, 1.5, 1, 1, 1, 1, 1, 1.2,
    ultra_matte_hue_adj, ultra_matte_lum_adj, ultra_matte_sat_adj },
  { "BackFilm", 0.83, .75, 1, .01, 1.5, 1, 1, 1, 1, 1, 1.2,
    ultra_matte_hue_adj, ultra_matte_lum_adj, ultra_matte_sat_adj },
  { "Matte", 0.92, 0.4, 1, .01, 1.5, 1, 1, 1, 1, 1, 1.2,
    ultra_matte_hue_adj, ultra_matte_lum_adj, ultra_matte_sat_adj },
  { "MatteHeavy", 0.92, 0.4, 1, .01, 1.5, 1, 1, 1, 1, 1, 1.2,
    ultra_matte_hue_adj, ultra_matte_lum_adj, ultra_matte_sat_adj },
  { "Inkjet", 0.72, .5, 1, .01, 1.5, 1, 1, 1, 1, 1, 1.2,
    ultra_matte_hue_adj, ultra_matte_lum_adj, ultra_matte_sat_adj },
  { "Coated", 0.83, .5, 1, .01, 1.5, 1, 1, 1, 1, 1, 1.2,
    ultra_matte_hue_adj, ultra_matte_lum_adj, ultra_matte_sat_adj },
  { "Photo", 1.0, .75, 1, .01, 1.5, 1, 1, 1, 1, 1, 1.2,
    ultra_matte_hue_adj, ultra_matte_lum_adj, ultra_matte_sat_adj },
  { "GlossyPhoto", 0.72, 1, 1, .01, 1.8, 1, 1, 1, 1, 1, .9,
    ultra_glossy_hue_adj, ultra_glossy_lum_adj, ultra_glossy_sat_adj },
  { "Semigloss", 0.72, .8, 1, .01, 1.8, 1, 1, 1, 1, 1, .9,
    ultra_glossy_hue_adj, ultra_glossy_lum_adj, ultra_glossy_sat_adj },
  { "Luster", 0.72, .8, 1, .01, 1.8, 1, 1, 1, 1, 1, .9,
    ultra_glossy_hue_adj, ultra_glossy_lum_adj, ultra_glossy_sat_adj },
  { "ArchivalMatte", 0.92, .4, 1, .01, 1.5, 1, 1, 1, 1, 1, 1.2,
    ultra_matte_hue_adj, ultra_matte_lum_adj, ultra_matte_sat_adj },
  { "WaterColorRadiant", 0.92, .4, 1, .01, 1.5, 1, 1, 1, 1, 1, 1.2,
    ultra_matte_hue_adj, ultra_matte_lum_adj, ultra_matte_sat_adj },
  { "GlossyPaper", 0.83, 1.0, 1, .01, 1.5, 1, 1, 1, 1, 1, 1.2,
    ultra_matte_hue_adj, ultra_matte_lum_adj, ultra_matte_sat_adj },
  { "Ilford", 0.83, 1.0, 1, .01, 1.5, 1, 1, 1, 1, 1, 1.2,
    ultra_matte_hue_adj, ultra_matte_lum_adj, ultra_matte_sat_adj  },
  { "ColorLife", 0.83, 1.0, 1, .01, 1.5, 1, 1, 1, 1, 1, 1.2,
    ultra_matte_hue_adj, ultra_matte_lum_adj, ultra_matte_sat_adj },
  { "Other", 0.72, .1, 1, .01, 1.5, 1, 1, 1, 1, 1, 1.2,
    ultra_matte_hue_adj, ultra_matte_lum_adj, ultra_matte_sat_adj },
};

DECLARE_PAPER_ADJUSTMENTS(ultrachrome_photo);

static const paper_adjustment_t ultrachrome_matte_adjustments[] =
{
  { "Plain", 0.72, .1, 1, 0, .999, 1, 1, 1, 1, 1, 1.2,
    ultra_matte_hue_adj, ultra_matte_lum_adj, ultra_matte_sat_adj },
  { "PlainFast", 0.72, .1, 1, 0, .999, 1, 1, 1, 1, 1, 1.2,
    ultra_matte_hue_adj, ultra_matte_lum_adj, ultra_matte_sat_adj },
  { "Postcard", 0.72, .1, 1, 0, .999, 1, 1, 1, 1, 1, 1.2,
    ultra_matte_hue_adj, ultra_matte_lum_adj, ultra_matte_sat_adj },
  { "GlossyFilm", 0.83, .5, 1, 0.01, 1.25, 1, 1, 1, 1, 1, 1.2,
    ultra_matte_hue_adj, ultra_matte_lum_adj, ultra_matte_sat_adj },
  { "Transparency", 0.83, .5, 1, 0.01, 1.25, 1, 1, 1, 1, 1, 1,
    ultra_matte_hue_adj, ultra_matte_lum_adj, ultra_matte_sat_adj },
  { "Envelope", 0.72, .1, 1, 0, .999, 1, 1, 1, 1, 1, 1.2,
    ultra_matte_hue_adj, ultra_matte_lum_adj, ultra_matte_sat_adj },
  { "BackFilm", 0.83, .5, 1, 0.01, 1.25, 1, 1, 1, 1, 1, 1.2,
    ultra_matte_hue_adj, ultra_matte_lum_adj, ultra_matte_sat_adj },
  { "Matte", 0.92, 0.4, 1, 0.01, 1.25, 1, 1, 1, 1, 1, 1.2,
    ultra_matte_hue_adj, ultra_matte_lum_adj, ultra_matte_sat_adj },
  { "MatteHeavy", 0.92, 0.4, .4, .01, 0.999, 1, 1, 1, 1.75, 1, 1.2,
    ultra_matte_hue_adj, ultra_matte_lum_adj, ultra_matte_sat_adj },
  { "Inkjet", 0.72, .3, 1, .01, .999, 1, 1, 1, 1, 1, 1.2,
    ultra_matte_hue_adj, ultra_matte_lum_adj, ultra_matte_sat_adj },
  { "Coated", 0.83, .4, 1, .01, 1.25, 1, 1, 1, 1, 1, 1.2,
    ultra_matte_hue_adj, ultra_matte_lum_adj, ultra_matte_sat_adj },
  { "Photo", 1.0, 0.5, 1, 0.01, 1.25, 1, 1, 1, 1, 1, 1.2,
    ultra_matte_hue_adj, ultra_matte_lum_adj, ultra_matte_sat_adj },
  { "GlossyPhoto", 0.72, 1, 1, .01, 1.25, 1, 1, 1, 1, 1, .9,
    ultra_glossy_hue_adj, ultra_glossy_lum_adj, ultra_glossy_sat_adj },
  { "Semigloss", 0.72, .8, 1, .01, 1.25, 1, 1, 1, 1, 1, .9,
    ultra_glossy_hue_adj, ultra_glossy_lum_adj, ultra_glossy_sat_adj },
  { "Luster", 0.72, .8, 1, .01, 1.25, 1, 1, 1, 1, 1, .9,
    ultra_glossy_hue_adj, ultra_glossy_lum_adj, ultra_glossy_sat_adj },
  { "WaterColorRadiant", 0.92, 0.4, 1, .01, 1.25, 1, 1, 1, 1, 1, 1.2,
    ultra_matte_hue_adj, ultra_matte_lum_adj, ultra_matte_sat_adj },
  { "GlossyPaper", 0.83, 0.5, 1, 0.01, 1.25, 1, 1, 1, 1, 1, 1,
    ultra_matte_hue_adj, ultra_matte_lum_adj, ultra_matte_sat_adj },
  { "Ilford", 0.83, 0.5, 1, 0.01, 1.25, 1, 1, 1, 1, 1, 1,
    ultra_matte_hue_adj, ultra_matte_lum_adj, ultra_matte_sat_adj  },
  { "ColorLife", 0.83, 0.5, 1, 0.01, 1.25, 1, 1, 1, 1, 1, 1,
    ultra_matte_hue_adj, ultra_matte_lum_adj, ultra_matte_sat_adj },
  { "Other", 0.72, .1, .4, 0, .999, 1, 1, 1, 1.75, 1, 1.2,
    ultra_matte_hue_adj, ultra_matte_lum_adj, ultra_matte_sat_adj },
};

DECLARE_PAPER_ADJUSTMENTS(ultrachrome_matte);

static const paper_adjustment_t r800_photo_adjustments[] =
{
  { "Plain", 0.72, .1, 1, .1, 0.5, .882, 1, .300, 1, 1, 1.2,
    r800_matte_hue_adj, r800_matte_lum_adj, r800_matte_sat_adj },
  { "PlainFast", 0.72, .1, 1, .1, 0.5, .882, 1, .300, 1, 1, 1.2,
    r800_matte_hue_adj, r800_matte_lum_adj, r800_matte_sat_adj },
  { "Postcard", 0.72, .1, 1, .1, 0.5, .882, 1, .300, 1, 1, 1.2,
    r800_matte_hue_adj, r800_matte_lum_adj, r800_matte_sat_adj },
  { "GlossyFilm", 0.83, 1.0, 1, .1, 0.5, .882, 1, .300, 1, 1, 1.2,
    r800_matte_hue_adj, r800_matte_lum_adj, r800_matte_sat_adj },
  { "Transparency", 0.83, .75, 1, .1, 0.5, .882, 1, .300, 1, 1, 1.2,
    r800_matte_hue_adj, r800_matte_lum_adj, r800_matte_sat_adj },
  { "Envelope", 0.72, .1, 1, .1, 0.5, .882, 1, .300, 1, 1, 1.2,
    r800_matte_hue_adj, r800_matte_lum_adj, r800_matte_sat_adj },
  { "BackFilm", 0.83, .75, 1, .1, 0.5, .882, 1, .300, 1, 1, 1.2,
    r800_matte_hue_adj, r800_matte_lum_adj, r800_matte_sat_adj },
  { "Matte", 0.92, 0.4, 1, .1, 0.5, .882, 1, .300, 1, 1, 1.2,
    r800_matte_hue_adj, r800_matte_lum_adj, r800_matte_sat_adj },
  { "MatteHeavy", 0.92, 0.4, 1, .1, 0.5, .882, 1, .300, 1, 1, 1.2,
    r800_matte_hue_adj, r800_matte_lum_adj, r800_matte_sat_adj },
  { "Inkjet", 0.72, .5, 1, .1, 0.5, .882, 1, .300, 1, 1, 1.2,
    r800_matte_hue_adj, r800_matte_lum_adj, r800_matte_sat_adj },
  { "Coated", 0.83, .5, 1, .1, 0.5, .882, 1, .300, 1, 1, 1.2,
    r800_matte_hue_adj, r800_matte_lum_adj, r800_matte_sat_adj },
  { "Photo", 1.0, .75, 1, .1, 0.5, .882, 1, .300, 1, 1, 1.2,
    r800_matte_hue_adj, r800_matte_lum_adj, r800_matte_sat_adj },
  { "GlossyPhoto", 0.72, 1, 1, .1, 0.5, .882, 1, .300, 1, 1, 0.9,
    r800_glossy_hue_adj, r800_glossy_lum_adj, r800_glossy_sat_adj },
  { "Semigloss", 0.72, .8, 1, .1, 0.5, .882, 1, .300, 1, 1, 0.9,
    r800_glossy_hue_adj, r800_glossy_lum_adj, r800_glossy_sat_adj },
  { "Luster", 0.72, .8, 1, .1, .5, .882, 1, .300, 1, 1, 0.9,
    r800_glossy_hue_adj, r800_glossy_lum_adj, r800_glossy_sat_adj },
  { "ArchivalMatte", 0.92, .4, 1, .1, 0.5, .882, 1, .300, 1, 1, 1.2,
    r800_matte_hue_adj, r800_matte_lum_adj, r800_matte_sat_adj },
  { "WaterColorRadiant", 0.92, .4, 1, .1, 0.5, .882, 1, .300, 1, 1, 1.2,
    r800_matte_hue_adj, r800_matte_lum_adj, r800_matte_sat_adj },
  { "GlossyPaper", 0.83, 1.0, 1, .1, 0.5, .882, 1, .300, 1, 1, 1.2,
    r800_matte_hue_adj, r800_matte_lum_adj, r800_matte_sat_adj },
  { "Ilford", 0.83, 1.0, 1, .1, 0.5, .882, 1, .300, 1, 1, 1.2,
    r800_matte_hue_adj, r800_matte_lum_adj, r800_matte_sat_adj  },
  { "ColorLife", 0.83, 1.0, 1, .1, 0.5, .882, 1, .300, 1, 1, 1.2,
    r800_matte_hue_adj, r800_matte_lum_adj, r800_matte_sat_adj },
  { "Other", 0.72, .1, 1, .1, 0.5, .882, 1, .300, 1, 1, 1.2,
    r800_matte_hue_adj, r800_matte_lum_adj, r800_matte_sat_adj },
};

DECLARE_PAPER_ADJUSTMENTS(r800_photo);

static const paper_adjustment_t r800_matte_adjustments[] =
{
  { "Plain", 0.72, .1, .5, .025, .5, .882, 1, .300, .8, 1, 1.2,
    r800_matte_hue_adj, r800_matte_lum_adj, r800_matte_sat_adj },
  { "PlainFast", 0.72, .1, .5, .025, .5, .882, 1, .300, .8, 1, 1.2,
    r800_matte_hue_adj, r800_matte_lum_adj, r800_matte_sat_adj },
  { "Postcard", 0.72, .1, .5, .025, .5, .882, 1, .300, .8, 1, 1.2,
    r800_matte_hue_adj, r800_matte_lum_adj, r800_matte_sat_adj },
  { "GlossyFilm", 0.83, .5, .5, .025, .5, .882, 1, .300, .8, 1, 1.2,
    r800_matte_hue_adj, r800_matte_lum_adj, r800_matte_sat_adj },
  { "Transparency", 0.83, .5, .5, .025, .5, .882, 1, .300, .8, 1, 1,
    r800_matte_hue_adj, r800_matte_lum_adj, r800_matte_sat_adj },
  { "Envelope", 0.72, .1, .5, .025, .5, .882, 1, .300, .8, 1, 1.2,
    r800_matte_hue_adj, r800_matte_lum_adj, r800_matte_sat_adj },
  { "BackFilm", 0.83, .5, .5, .025, .5, .882, 1, .300, .8, 1, 1.2,
    r800_matte_hue_adj, r800_matte_lum_adj, r800_matte_sat_adj },
  { "Matte", 0.92, 0.4, .5, .025, .5, .882, 1, .300, .8, 1, 1.2,
    r800_matte_hue_adj, r800_matte_lum_adj, r800_matte_sat_adj },
  { "MatteHeavy", 0.92, 0.4, .4, .01, .5, .882, 1, .300, .8, 1, 1.2,
    r800_matte_hue_adj, r800_matte_lum_adj, r800_matte_sat_adj },
  { "Inkjet", 0.72, .3, .5, .025, .5, .882, 1, .300, .8, 1, 1.2,
    r800_matte_hue_adj, r800_matte_lum_adj, r800_matte_sat_adj },
  { "Coated", 0.83, .4, .5, .025, .5, .882, 1, .300, .8, 1, 1.2,
    r800_matte_hue_adj, r800_matte_lum_adj, r800_matte_sat_adj },
  { "Photo", 1.0, 0.5, .5, .025, .5, .882, 1, .300, .8, 1, 1.2,
    r800_matte_hue_adj, r800_matte_lum_adj, r800_matte_sat_adj },
  { "GlossyPhoto", 0.72, 1, .5, .025, .5, .882, 1, .300, .8, 1, .9,
    r800_glossy_hue_adj, r800_glossy_lum_adj, r800_glossy_sat_adj },
  { "Semigloss", 0.72, .8, .5, .025, .5, .882, 1, .300, .8, 1, .9,
    r800_glossy_hue_adj, r800_glossy_lum_adj, r800_glossy_sat_adj },
  { "Luster", 0.72, .8, .5, .025, .5, .882, 1, .300, .8, 1, .9,
    r800_glossy_hue_adj, r800_glossy_lum_adj, r800_glossy_sat_adj },
  { "WaterColorRadiant", 0.92, 0.4, .5, .025, .5, .882, 1, .300, .8, 1, 1.2,
    r800_matte_hue_adj, r800_matte_lum_adj, r800_matte_sat_adj },
  { "GlossyPaper", 0.83, 0.5, .5, .025, .5, .882, 1, .300, .8, 1, 1,
    r800_matte_hue_adj, r800_matte_lum_adj, r800_matte_sat_adj },
  { "Ilford", 0.83, 0.5, .5, .025, .5, .882, 1, .300, .8, 1, 1,
    r800_matte_hue_adj, r800_matte_lum_adj, r800_matte_sat_adj  },
  { "ColorLife", 0.83, 0.5, .5, .025, .5, .882, 1, .300, .8, 1, 1,
    r800_matte_hue_adj, r800_matte_lum_adj, r800_matte_sat_adj },
  { "Other", 0.72, .1, .5, .025, .5, .882, 1, .300, .8, 1, 1.2,
    r800_matte_hue_adj, r800_matte_lum_adj, r800_matte_sat_adj },
};

DECLARE_PAPER_ADJUSTMENTS(r800_matte);

static const paper_adjustment_t durabrite_adjustments[] =
{
  { "Plain", 1.0, .5, .5, .05, .9, 1, 1, 1, 1, 1, 1.2,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "PlainFast", 1.0, .5, .5, .05, .9, 1, 1, 1, 1, 1, 1.2,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "Postcard", 1.0, .5, 1, .05, .9, 1, 1, 1, 1, 1, 1.2,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "GlossyFilm", 0.8, 1.0, 1, .05, .999, 1, 1, 1, 1, 1, 1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "Transparency", 0.8, .75, 1, .05, .999, 1, 1, 1, 1, 1, 1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "Envelope", 1.0, .5, 1, .05, .9, 1, 1, 1, 1, 1, 1.2,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "BackFilm", 0.8, .75, 1, .05, .999, 1, 1, 1, 1, 1, 1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "Matte", 0.9, .5, .5, .075, .999, 1, .975, .975, 1, 1, 1.1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "MatteHeavy", 0.9, .5, .5, .075, .999, 1, .975, .975, 1, 1, 1.1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "Inkjet", 1.0, .5, .5, .05, .9, 1, 1, 1, 1, 1, 1.2,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "Coated", 1.0, .5, .5, .075, .999, 1, 1, 1, 1, 1, 1.1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "Photo", .833, .5, .5, .075, .999, 1, 1, 1, 1, 1, 1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "GlossyPhoto", .833, 1.0, 1, .15, .999, 1, 1, 1, 1, 1, .9,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "Semigloss", .833, 1.0, 1, .15, .999, 1, 1, 1, 1, 1, .9,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "Luster", .833, 1.0, 1, .15, .999, 1, 1, 1, 1, 1, .9,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "GlossyPaper", .833, 1.0, 1, .15, .999, 1, 1, 1, 1, 1, .9,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "Ilford", .833, 1.0, 1, .15, 1.35, 1, 1, 1, 1, 1, 1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj  },
  { "ColorLife", .833, 1.0, 1, .15, .9, 1, 1, 1, 1, 1, 1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "Other", 1.0, .5, 1, .05, .9, 1, 1, 1, 1, 1, 1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
};

DECLARE_PAPER_ADJUSTMENTS(durabrite);

static const paper_adjustment_t durabrite2_adjustments[] =
{
  { "Plain", 1.0, .5, .5, .05, .9, 1, 1, 1, 1, 1, 1.2,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "PlainFast", 1.0, .5, .5, .05, .9, 1, 1, 1, 1, 1, 1.2,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "Postcard", 1.0, .5, 1, .05, .9, 1, 1, 1, 1, 1, 1.2,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "GlossyFilm", 0.8, 1.0, 1, .05, .999, 1, 1, 1, 1, 1, 1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "Transparency", 0.8, .75, 1, .05, .999, 1, 1, 1, 1, 1, 1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "Envelope", 1.0, .5, 1, .05, .9, 1, 1, 1, 1, 1, 1.2,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "BackFilm", 0.8, .75, 1, .05, .999, 1, 1, 1, 1, 1, 1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "Matte", 0.9, .5, .5, .075, .999, 1, .975, .975, 1, 1, 1.1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "MatteHeavy", 0.9, .5, .5, .075, .999, 1, .975, .975, 1, 1, 1.1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "Inkjet", 1.0, .5, .5, .05, .9, 1, 1, 1, 1, 1, 1.2,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "Coated", 1.0, .5, .5, .075, .999, 1, 1, 1, 1, 1, 1.1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "Photo", .833, .5, .5, .075, .999, 1, 1, 1, 1, 1, 1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "GlossyPhoto", .833, 1.0, 1, .15, .999, 1, 1, 1, 1, 1, .9,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "Semigloss", .833, 1.0, 1, .15, .999, 1, 1, 1, 1, 1, .9,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "Luster", .833, 1.0, 1, .15, .999, 1, 1, 1, 1, 1, .9,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "GlossyPaper", .833, 1.0, 1, .15, .999, 1, 1, 1, 1, 1, .9,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "Ilford", .833, 1.0, 1, .15, 1.35, 1, 1, 1, 1, 1, 1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj  },
  { "ColorLife", .833, 1.0, 1, .15, .9, 1, 1, 1, 1, 1, 1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
  { "Other", 1.0, .5, 1, .05, .9, 1, 1, 1, 1, 1, 1,
    standard_hue_adj, standard_lum_adj, standard_sat_adj },
};

DECLARE_PAPER_ADJUSTMENTS(durabrite2);

static const paper_t standard_papers[] =
{
  { "Plain", N_("Plain Paper"), PAPER_PLAIN,
    1, 0, 0x6b, 0x1a, 0x01, NULL, NULL },
  { "PlainFast", N_("Plain Paper Fast Load"), PAPER_PLAIN,
    5, 0, 0x6b, 0x1a, 0x01, NULL, NULL },
  { "Postcard", N_("Postcard"), PAPER_PLAIN,
    2, 0, 0x00, 0x00, 0x02, NULL, NULL },
  { "GlossyFilm", N_("Glossy Film"), PAPER_PHOTO,
    3, 0, 0x6d, 0x00, 0x01, NULL, NULL },
  { "Transparency", N_("Transparencies"), PAPER_TRANSPARENCY,
    3, 0, 0x6d, 0x00, 0x02, NULL, NULL },
  { "Envelope", N_("Envelopes"), PAPER_PLAIN,
    4, 0, 0x6b, 0x1a, 0x01, NULL, NULL },
  { "BackFilm", N_("Back Light Film"), PAPER_TRANSPARENCY,
    6, 0, 0x6d, 0x00, 0x01, NULL, NULL },
  { "Matte", N_("Matte Paper"), PAPER_GOOD,
    7, 0, 0x00, 0x00, 0x02, NULL, NULL },
  { "MatteHeavy", N_("Matte Paper Heavyweight"), PAPER_GOOD,
    7, 0, 0x00, 0x00, 0x02, NULL, NULL },
  { "Inkjet", N_("Inkjet Paper"), PAPER_GOOD,
    7, 0, 0x6b, 0x1a, 0x01, NULL, NULL },
  { "Coated", N_("Photo Quality Inkjet Paper"), PAPER_GOOD,
    7, 0, 0x6b, 0x1a, 0x01, NULL, NULL },
  { "Photo", N_("Photo Paper"), PAPER_PHOTO,
    8, 0, 0x67, 0x00, 0x02, NULL, NULL },
  { "GlossyPhoto", N_("Premium Glossy Photo Paper"), PAPER_PREMIUM_PHOTO,
    8, 0, 0x80, 0x00, 0x02, NULL, NULL },
  { "Semigloss", N_("Premium Semigloss Photo Paper"), PAPER_PREMIUM_PHOTO,
    8, 0, 0x80, 0x00, 0x02, NULL, NULL },
  { "Luster", N_("Premium Luster Photo Paper"), PAPER_PREMIUM_PHOTO,
    8, 0, 0x80, 0x00, 0x02, NULL, NULL },
  { "GlossyPaper", N_("Photo Quality Glossy Paper"), PAPER_PREMIUM_PHOTO,
    6, 0, 0x6b, 0x1a, 0x01, NULL, NULL },
  { "Ilford", N_("Ilford Heavy Paper"), PAPER_PREMIUM_PHOTO,
    8, 0, 0x80, 0x00, 0x02, NULL, NULL },
  { "ColorLife", N_("ColorLife Paper"), PAPER_PREMIUM_PHOTO,
    8, 0, 0x67, 0x00, 0x02, NULL, NULL },
  { "Other", N_("Other"), PAPER_PLAIN,
    0, 0, 0x6b, 0x1a, 0x01, NULL, NULL },
};

DECLARE_PAPERS(standard);

static const paper_t durabrite_papers[] =
{
  { "Plain", N_("Plain Paper"), PAPER_PLAIN,
    1, 0, 0x6b, 0x1a, 0x01, NULL, NULL },
  { "PlainFast", N_("Plain Paper Fast Load"), PAPER_PLAIN,
    5, 0, 0x6b, 0x1a, 0x01, NULL, NULL },
  { "Postcard", N_("Postcard"), PAPER_PLAIN,
    2, 0, 0x00, 0x00, 0x02, NULL, NULL },
  { "GlossyFilm", N_("Glossy Film"), PAPER_PHOTO,
    3, 0, 0x6d, 0x00, 0x01, NULL, NULL },
  { "Transparency", N_("Transparencies"), PAPER_TRANSPARENCY,
    3, 0, 0x6d, 0x00, 0x02, NULL, NULL },
  { "Envelope", N_("Envelopes"), PAPER_PLAIN,
    4, 0, 0x6b, 0x1a, 0x01, NULL, NULL },
  { "BackFilm", N_("Back Light Film"), PAPER_TRANSPARENCY,
    6, 0, 0x6d, 0x00, 0x01, NULL, NULL },
  { "Matte", N_("Matte Paper"), PAPER_GOOD,
    7, 0, 0x00, 0x00, 0x02, NULL, NULL },
  { "MatteHeavy", N_("Matte Paper Heavyweight"), PAPER_GOOD,
    7, 0, 0x00, 0x00, 0x02, NULL, NULL },
  { "Inkjet", N_("Inkjet Paper"), PAPER_GOOD,
    7, 0, 0x6b, 0x1a, 0x01, NULL, NULL },
  { "Coated", N_("Photo Quality Inkjet Paper"), PAPER_GOOD,
    7, 0, 0x6b, 0x1a, 0x01, NULL, NULL },
  { "Photo", N_("Photo Paper"), PAPER_PHOTO,
    8, 0, 0x67, 0x00, 0x02, "RGB", NULL },
  { "GlossyPhoto", N_("Premium Glossy Photo Paper"), PAPER_PREMIUM_PHOTO,
    8, 0, 0x80, 0x00, 0x02, "RGB", NULL },
  { "Semigloss", N_("Premium Semigloss Photo Paper"), PAPER_PREMIUM_PHOTO,
    8, 0, 0x80, 0x00, 0x02, "RGB", NULL },
  { "Luster", N_("Premium Luster Photo Paper"), PAPER_PREMIUM_PHOTO,
    8, 0, 0x80, 0x00, 0x02, "RGB", NULL },
  { "GlossyPaper", N_("Photo Quality Glossy Paper"), PAPER_PHOTO,
    6, 0, 0x6b, 0x1a, 0x01, "RGB", NULL },
  { "Ilford", N_("Ilford Heavy Paper"), PAPER_PREMIUM_PHOTO,
    8, 0, 0x80, 0x00, 0x02, NULL, NULL },
  { "ColorLife", N_("ColorLife Paper"), PAPER_PREMIUM_PHOTO,
    8, 0, 0x67, 0x00, 0x02, NULL, NULL },
  { "Other", N_("Other"), PAPER_PLAIN,
    0, 0, 0x6b, 0x1a, 0x01, NULL, NULL },
};

DECLARE_PAPERS(durabrite);

static const paper_t ultrachrome_papers[] =
{
  { "Plain", N_("Plain Paper"), PAPER_PLAIN,
    1, 0, 0x6b, 0x1a, 0x01, NULL, "UltraMatte" },
  { "PlainFast", N_("Plain Paper Fast Load"), PAPER_PLAIN,
    5, 0, 0x6b, 0x1a, 0x01, NULL, "UltraMatte" },
  { "Postcard", N_("Postcard"), PAPER_PLAIN,
    2, 0, 0x00, 0x00, 0x02, NULL, "UltraMatte" },
  { "GlossyFilm", N_("Glossy Film"), PAPER_PHOTO,
    3, 0, 0x6d, 0x00, 0x01, NULL, "UltraPhoto" },
  { "Transparency", N_("Transparencies"), PAPER_TRANSPARENCY,
    3, 0, 0x6d, 0x00, 0x02, NULL, "UltraPhoto" },
  { "Envelope", N_("Envelopes"), PAPER_PLAIN,
    4, 0, 0x6b, 0x1a, 0x01, NULL, "UltraMatte" },
  { "BackFilm", N_("Back Light Film"), PAPER_TRANSPARENCY,
    6, 0, 0x6d, 0x00, 0x01, NULL, "UltraPhoto" },
  { "Matte", N_("Matte Paper"), PAPER_GOOD,
    7, 0, 0x00, 0x00, 0x02, NULL, "UltraMatte" },
  { "MatteHeavy", N_("Matte Paper Heavyweight"), PAPER_GOOD,
    7, 0, 0x00, 0x00, 0x02, NULL, "UltraMatte" },
  { "Inkjet", N_("Inkjet Paper"), PAPER_GOOD,
    7, 0, 0x6b, 0x1a, 0x01, NULL, "UltraMatte" },
  { "Coated", N_("Photo Quality Inkjet Paper"), PAPER_GOOD,
    7, 0, 0x6b, 0x1a, 0x01, NULL, "UltraPhoto" },
  { "Photo", N_("Photo Paper"), PAPER_PHOTO,
    8, 0, 0x67, 0x00, 0x02, NULL, "UltraPhoto" },
  { "GlossyPhoto", N_("Premium Glossy Photo Paper"), PAPER_PREMIUM_PHOTO,
    8, 0, 0x80, 0x00, 0x02, NULL, "UltraPhoto" },
  { "Semigloss", N_("Premium Semigloss Photo Paper"), PAPER_PREMIUM_PHOTO,
    8, 0, 0x80, 0x00, 0x02, NULL, "UltraPhoto" },
  { "Luster", N_("Premium Luster Photo Paper"), PAPER_PREMIUM_PHOTO,
    8, 0, 0x80, 0x00, 0x02, NULL, "UltraPhoto" },
  { "ArchivalMatte", N_("Archival Matte Paper"), PAPER_PREMIUM_PHOTO,
    7, 0, 0x00, 0x00, 0x02, NULL, "UltraMatte" },
  { "WaterColorRadiant", N_("Watercolor Paper - Radiant White"), PAPER_PREMIUM_PHOTO,
    7, 0, 0x00, 0x00, 0x02, NULL, "UltraMatte" },
  { "GlossyPaper", N_("Photo Quality Glossy Paper"), PAPER_PHOTO,
    6, 0, 0x6b, 0x1a, 0x01, NULL, "UltraPhoto" },
  { "Ilford", N_("Ilford Heavy Paper"), PAPER_PREMIUM_PHOTO,
    8, 0, 0x80, 0x00, 0x02, NULL, "UltraMatte" },
  { "ColorLife", N_("ColorLife Paper"), PAPER_PREMIUM_PHOTO,
    8, 0, 0x67, 0x00, 0x02, NULL, "UltraPhoto" },
  { "Other", N_("Other"), PAPER_PLAIN,
    0, 0, 0x6b, 0x1a, 0x01, NULL, "UltraMatte" },
};

DECLARE_PAPERS(ultrachrome);

static const paper_t durabrite2_papers[] =
{
  { "Plain", N_("Plain Paper"), PAPER_PLAIN,
    1, 0, 0x6b, 0x1a, 0x01, NULL, NULL },
  { "PlainFast", N_("Plain Paper Fast Load"), PAPER_PLAIN,
    5, 0, 0x6b, 0x1a, 0x01, NULL, NULL },
  { "Postcard", N_("Postcard"), PAPER_PLAIN,
    2, 0, 0x00, 0x00, 0x02, NULL, NULL },
  { "GlossyFilm", N_("Glossy Film"), PAPER_PHOTO,
    3, 0, 0x6d, 0x00, 0x01, NULL, NULL },
  { "Transparency", N_("Transparencies"), PAPER_TRANSPARENCY,
    3, 0, 0x6d, 0x00, 0x02, NULL, NULL },
  { "Envelope", N_("Envelopes"), PAPER_PLAIN,
    4, 0, 0x6b, 0x1a, 0x01, NULL, NULL },
  { "BackFilm", N_("Back Light Film"), PAPER_TRANSPARENCY,
    6, 0, 0x6d, 0x00, 0x01, NULL, NULL },
  { "Matte", N_("Matte Paper"), PAPER_GOOD,
    7, 0, 0x00, 0x00, 0x02, NULL, NULL },
  { "MatteHeavy", N_("Matte Paper Heavyweight"), PAPER_GOOD,
    7, 0, 0x00, 0x00, 0x02, NULL, NULL },
  { "Inkjet", N_("Inkjet Paper"), PAPER_GOOD,
    7, 0, 0x6b, 0x1a, 0x01, NULL, NULL },
  { "Coated", N_("Photo Quality Inkjet Paper"), PAPER_GOOD,
    7, 0, 0x6b, 0x1a, 0x01, NULL, NULL },
  { "Photo", N_("Photo Paper"), PAPER_PHOTO,
    8, 0, 0x67, 0x00, 0x02, NULL, NULL },
  { "GlossyPhoto", N_("Premium Glossy Photo Paper"), PAPER_PREMIUM_PHOTO,
    8, 0, 0x80, 0x00, 0x02, NULL, NULL },
  { "Semigloss", N_("Premium Semigloss Photo Paper"), PAPER_PREMIUM_PHOTO,
    8, 0, 0x80, 0x00, 0x02, NULL, NULL },
  { "Luster", N_("Premium Luster Photo Paper"), PAPER_PREMIUM_PHOTO,
    8, 0, 0x80, 0x00, 0x02, NULL, NULL },
  { "GlossyPaper", N_("Photo Quality Glossy Paper"), PAPER_PHOTO,
    6, 0, 0x6b, 0x1a, 0x01, NULL, NULL },
  { "Ilford", N_("Ilford Heavy Paper"), PAPER_PREMIUM_PHOTO,
    8, 0, 0x80, 0x00, 0x02, NULL, NULL },
  { "ColorLife", N_("ColorLife Paper"), PAPER_PREMIUM_PHOTO,
    8, 0, 0x67, 0x00, 0x02, NULL, NULL },
  { "Other", N_("Other"), PAPER_PLAIN,
    0, 0, 0x6b, 0x1a, 0x01, NULL, NULL },
};

DECLARE_PAPERS(durabrite2);

static const paper_t r800_papers[] =
{
  { "Plain", N_("Plain Paper"), PAPER_PLAIN,
    1, 0, 0x6b, 0x1a, 0x01, NULL, "r800Matte" },
  { "PlainFast", N_("Plain Paper Fast Load"), PAPER_PLAIN,
    5, 0, 0x6b, 0x1a, 0x01, NULL, "r800Matte" },
  { "Postcard", N_("Postcard"), PAPER_PLAIN,
    2, 0, 0x00, 0x00, 0x02, NULL, "r800Matte" },
  { "GlossyFilm", N_("Glossy Film"), PAPER_PHOTO,
    3, 0, 0x6d, 0x00, 0x01, NULL, "r800Photo" },
  { "Transparency", N_("Transparencies"), PAPER_TRANSPARENCY,
    3, 0, 0x6d, 0x00, 0x02, NULL, "r800Photo" },
  { "Envelope", N_("Envelopes"), PAPER_PLAIN,
    4, 0, 0x6b, 0x1a, 0x01, NULL, "r800Matte" },
  { "BackFilm", N_("Back Light Film"), PAPER_TRANSPARENCY,
    6, 0, 0x6d, 0x00, 0x01, NULL, "r800Photo" },
  { "Matte", N_("Matte Paper"), PAPER_GOOD,
    7, 0, 0x00, 0x00, 0x02, NULL, "r800Matte" },
  { "MatteHeavy", N_("Matte Paper Heavyweight"), PAPER_GOOD,
    7, 0, 0x00, 0x00, 0x02, NULL, "r800Matte" },
  { "Inkjet", N_("Inkjet Paper"), PAPER_GOOD,
    7, 0, 0x6b, 0x1a, 0x01, NULL, "r800Matte" },
  { "Coated", N_("Photo Quality Inkjet Paper"), PAPER_GOOD,
    7, 0, 0x6b, 0x1a, 0x01, NULL, "r800Photo" },
  { "Photo", N_("Photo Paper"), PAPER_PHOTO,
    8, 0, 0x67, 0x00, 0x02, NULL, "r800Photo" },
  { "GlossyPhoto", N_("Premium Glossy Photo Paper"), PAPER_PREMIUM_PHOTO,
    8, 0, 0x80, 0x00, 0x02, NULL, "r800Photo" },
  { "Semigloss", N_("Premium Semigloss Photo Paper"), PAPER_PREMIUM_PHOTO,
    8, 0, 0x80, 0x00, 0x02, NULL, "r800Photo" },
  { "Luster", N_("Premium Luster Photo Paper"), PAPER_PREMIUM_PHOTO,
    8, 0, 0x80, 0x00, 0x02, NULL, "r800Photo" },
  { "ArchivalMatte", N_("Archival Matte Paper"), PAPER_PREMIUM_PHOTO,
    7, 0, 0x00, 0x00, 0x02, NULL, "r800Matte" },
  { "WaterColorRadiant", N_("Watercolor Paper - Radiant White"), PAPER_PREMIUM_PHOTO,
    7, 0, 0x00, 0x00, 0x02, NULL, "r800Matte" },
  { "GlossyPaper", N_("Photo Quality Glossy Paper"), PAPER_PHOTO,
    6, 0, 0x6b, 0x1a, 0x01, NULL, "r800Photo" },
  { "Ilford", N_("Ilford Heavy Paper"), PAPER_PREMIUM_PHOTO,
    8, 0, 0x80, 0x00, 0x02, NULL, "r800Matte" },
  { "ColorLife", N_("ColorLife Paper"), PAPER_PREMIUM_PHOTO,
    8, 0, 0x67, 0x00, 0x02, NULL, "r800Photo" },
  { "Other", N_("Other"), PAPER_PLAIN,
    0, 0, 0x6b, 0x1a, 0x01, NULL, "r800Matte" },
};

DECLARE_PAPERS(r800);
