/*
 *   Print plug-in CANON BJL driver for the GIMP.
 *
 *   Copyright 1997-2000 Michael Sweet (mike@easysw.com),
 *	Robert Krawitz (rlk@alum.mit.edu) and
 *      Andy Thaller (thaller@ph.tum.de)
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

/* This file contains definitions for the various inks
*/

#ifndef GUTENPRINT_INTERNAL_CANON_INKS_H
#define GUTENPRINT_INTERNAL_CANON_INKS_H

/* NOTE  NOTE  NOTE  NOTE  NOTE  NOTE  NOTE  NOTE  NOTE  NOTE  NOTE  NOTE
 *
 * The following dither ranges were taken from print-escp2.c and do NOT
 * represent the requirements of canon inks. Feel free to play with them
 * accoring to the escp2 part of doc/README.new-printer and send me a patch
 * if you get better results. Please send mail to thaller@ph.tum.de
 */

#define DECLARE_INK(name, density)		\
static const canon_variable_ink_t name##_ink =	\
{						\
  density,					\
  name##_shades,				\
  sizeof(name##_shades) / sizeof(stp_shade_t)	\
}

#define SHADE(density, name)				\
{ density, sizeof(name)/sizeof(stp_dotsize_t), name }

/*
 * Dither ranges specifically for Cyan/LightCyan (see NOTE above)
 *
 */

static const stp_dotsize_t single_dotsize[] =
{
  { 0x1, 1.0 }
};

static const stp_shade_t canon_Cc_1bit_shades[] =
{
  SHADE(1.0, single_dotsize),
  SHADE(0.25, single_dotsize),
};

DECLARE_INK(canon_Cc_1bit, 0.75);

/*
 * Dither ranges specifically for Magenta/LightMagenta (see NOTE above)
 *
 */

static const stp_shade_t canon_Mm_1bit_shades[] =
{
  SHADE(1.0, single_dotsize),
  SHADE(0.26, single_dotsize),
};

DECLARE_INK(canon_Mm_1bit, 0.75);

/*
 * Dither ranges specifically for any Color and 2bit/pixel (see NOTE above)
 *
 */
static const stp_dotsize_t two_bit_dotsize[] =
{
  { 0x1, 0.45 },
  { 0x2, 0.68 },
  { 0x3, 1.0 }
};

static const stp_shade_t canon_X_2bit_shades[] =
{
  SHADE(1.0, two_bit_dotsize)
};

DECLARE_INK(canon_X_2bit, 1.0);

static const stp_dotsize_t two_bit_3level_dotsize[] =
{
  { 0x1, 0.5  },
  { 0x2, 1.0  }
};

static const stp_shade_t canon_X_2bit_3level_shades[] =
{
  SHADE(1.0, two_bit_3level_dotsize)
};
DECLARE_INK(canon_X_2bit_3level,0.75);

/*
 * Dither ranges for black 1bit/pixel (even though photo black
 * is not used parameters for it have to be set in the t) command
 */

static const stp_shade_t canon_K_1bit_pixma_shades[] =
{
  SHADE(1.0, single_dotsize),
  SHADE(0.0, two_bit_3level_dotsize),
};
DECLARE_INK(canon_K_1bit_pixma,1.0);

static const stp_dotsize_t two_bit_4level_dotsize[] =
{
  { 0x1, 0.25 },
  { 0x2, 0.50 },
  { 0x3, 1.00 }
};

static const stp_shade_t canon_X_2bit_4level_shades[] =
{
  SHADE(1.0, two_bit_4level_dotsize)
};
DECLARE_INK(canon_X_2bit_4level,0.75);

/*
 * Dither ranges specifically for any Color/LightColor and 2bit/pixel
 * (see NOTE above)
 */
static const stp_shade_t canon_Xx_2bit_shades[] =
{
  SHADE(1.0, two_bit_dotsize),
  SHADE(0.33, two_bit_dotsize),
};

DECLARE_INK(canon_Xx_2bit, 1.0);

/*
 * Dither ranges specifically for any Color and 3bit/pixel
 * (see NOTE above)
 *
 * BIG NOTE: The bjc8200 has this kind of ink. One Byte seems to hold
 *           drop sizes for 3 pixels in a 3/2/2 bit fashion.
 *           Size values for 3bit-sized pixels range from 1 to 7,
 *           size values for 2bit-sized picels from 1 to 3 (kill msb).
 *
 *
 */
static const stp_dotsize_t three_bit_dotsize[] =
{
  { 0x1, 0.45 },
  { 0x2, 0.55 },
  { 0x3, 0.66 },
  { 0x4, 0.77 },
  { 0x5, 0.88 },
  { 0x6, 1.0 }
};

static const stp_shade_t canon_X_3bit_shades[] =
{
  SHADE(1.0, three_bit_dotsize)
};

DECLARE_INK(canon_X_3bit, 1.0);

/*
 * Dither ranges specifically for any Color/LightColor and 3bit/pixel
 * (see NOTE above)
 */
static const stp_shade_t canon_Xx_3bit_shades[] =
{
  SHADE(1.0, three_bit_dotsize),
  SHADE(0.33, three_bit_dotsize),
};

DECLARE_INK(canon_Xx_3bit, 1.0);


#endif

