/*
 * "$Id: dither-impl.h,v 1.5 2003/05/05 00:36:03 rlk Exp $"
 *
 *   Internal implementation of dither algorithms
 *
 *   Copyright 1997-2003 Michael Sweet (mike@easysw.com) and
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
 *
 * Revision History:
 *
 *   See ChangeLog
 */

/*
 * This file must include only standard C header files.  The core code must
 * compile on generic platforms that don't support glib, gimp, gtk, etc.
 */

#ifndef GIMP_PRINT_INTERNAL_DITHER_IMPL_H
#define GIMP_PRINT_INTERNAL_DITHER_IMPL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <limits.h>

#ifdef __GNUC__
#define inline __inline__
#endif

#define D_FLOYD_HYBRID 0
#define D_ADAPTIVE_BASE 4
#define D_ADAPTIVE_HYBRID (D_ADAPTIVE_BASE | D_FLOYD_HYBRID)
#define D_ORDERED_BASE 8
#define D_ORDERED (D_ORDERED_BASE)
#define D_FAST_BASE 16
#define D_FAST (D_FAST_BASE)
#define D_VERY_FAST (D_FAST_BASE + 1)
#define D_EVENTONE 32

#define DITHER_FAST_STEPS (6)

typedef struct
{
  const char *name;
  const char *text;
  int id;
} stpi_dither_algorithm_t;

#define ERROR_ROWS 2

#define MAX_SPREAD 32

typedef void stpi_ditherfunc_t(stp_vars_t, int, const unsigned short *, int, int);

/*
 * An end of a dither segment, describing one ink
 */

typedef struct ink_defn
{
  unsigned range;
  unsigned value;
  unsigned xvalue;
  unsigned bits;
  unsigned dot_size;
  int subchannel;
} stpi_ink_defn_t;

/*
 * A segment of the entire 0-65535 intensity range.
 */

typedef struct dither_segment
{
  stpi_ink_defn_t *lower;
  stpi_ink_defn_t *upper;
  unsigned range_span;
  unsigned value_span;
  int is_same_ink;
  int is_equal;
} stpi_dither_segment_t;

typedef struct
{
  unsigned subchannel_count;
  unsigned char **c;
} stpi_dither_channel_data_t;

typedef struct
{
  unsigned channel_count;
  stpi_dither_channel_data_t *c;
} stpi_dither_data_t;

typedef struct
{
  int dx;
  int dy;
  int r_sq;
} stpi_dis_t;

typedef struct shade_segment
{
  int subchannel;
  unsigned lower;
  unsigned trans;
  unsigned density;
  unsigned div1, div2;

  stpi_dis_t dis;
  stpi_dis_t *et_dis;

  int numdotsizes;
  stpi_ink_defn_t *dotsizes;

  int *errs;
  int value;
  int base;
} stpi_shade_segment_t;

typedef struct dither_channel
{
  unsigned randomizer;		/* With Floyd-Steinberg dithering, control */
				/* how much randomness is applied to the */
				/* threshold values (0-65535).  With ordered */
				/* dithering, how much randomness is added */
				/* to the matrix value. */
  int k_level;			/* Amount of each ink (in 64ths) required */
				/* to create equivalent black */
  int nlevels;
  unsigned bit_max;
  unsigned signif_bits;
  unsigned density;
  float sqrt_density_adjustment;
  float density_adjustment;

  int v;
  int o;
  int b;
  int very_fast;
  int subchannels;

  int maxdot;			/* Maximum dot size */

  stpi_ink_defn_t *ink_list;

  stpi_shade_segment_t *shades;
  int numshades;

  stpi_dither_segment_t *ranges;
  int **errs;
  unsigned short *vals;

  dither_matrix_t pick;
  dither_matrix_t dithermat;
  int *row_ends[2];
  unsigned char **ptrs;
} stpi_dither_channel_t;

typedef struct dither
{
  int src_width;		/* Input width */
  int dst_width;		/* Output width */

  float fdensity;

  int spread;			/* With Floyd-Steinberg, how widely the */
  int spread_mask;		/* error is distributed.  This should be */
				/* between 12 (very broad distribution) and */
				/* 19 (very narrow) */

  int stpi_dither_type;

  int adaptive_limit;

  int x_aspect;			/* Aspect ratio numerator */
  int y_aspect;			/* Aspect ratio denominator */

  double transition;		/* Exponential scaling for transition region */

  int *offset0_table;
  int *offset1_table;

  int d_cutoff;

  int oversampling;
  int last_line_was_empty;
  int ptr_offset;
  int n_channels;
  int n_input_channels;
  int n_ghost_channels;		/* FIXME: For composite grayscale
  				 * This is a really ugly way of doing this.
  				 * Of course, ultimately we'll eliminate
  				 * the CMYK special casing, which will
  				 * get rid of this. */
  int error_rows;

  int dither_class;		/* mono, black, or CMYK */

  dither_matrix_t dither_matrix;
  dither_matrix_t transition_matrix;
  stpi_dither_channel_t *channel;
  stpi_dither_data_t dt;

  unsigned short virtual_dot_scale[65536];
  stpi_ditherfunc_t *ditherfunc;
  void *aux_data;
  void (*aux_freefunc)(struct dither *);
} stpi_dither_t;

#define CHANNEL(d, c) ((d)->channel[(c) + (d)->n_ghost_channels])
#define PHYSICAL_CHANNEL(d, c) ((d)->channel[(c)])
#define CHANNEL_COUNT(d) ((d)->n_channels - (d)->n_ghost_channels)
#define PHYSICAL_CHANNEL_COUNT(d) ((d)->n_channels)

#define USMIN(a, b) ((a) < (b) ? (a) : (b))


extern stpi_ditherfunc_t stpi_dither_fast;
extern stpi_ditherfunc_t stpi_dither_very_fast;
extern stpi_ditherfunc_t stpi_dither_ordered;
extern stpi_ditherfunc_t stpi_dither_ed;
extern stpi_ditherfunc_t stpi_dither_et;

extern void stpi_dither_reverse_row_ends(stpi_dither_t *d);


#define ADVANCE_UNIDIRECTIONAL(d, bit, input, width, xerror, xstep, xmod) \
do									  \
{									  \
  bit >>= 1;								  \
  if (bit == 0)								  \
    {									  \
      d->ptr_offset++;							  \
      bit = 128;							  \
    }									  \
  input += xstep;							  \
  if (xmod)								  \
    {									  \
      xerror += xmod;							  \
      if (xerror >= d->dst_width)					  \
	{								  \
	  xerror -= d->dst_width;					  \
	  input += (width);						  \
	}								  \
    }									  \
} while (0)

#define ADVANCE_REVERSE(d, bit, input, width, xerror, xstep, xmod)	\
do									\
{									\
  if (bit == 128)							\
    {									\
      d->ptr_offset--;							\
      bit = 1;								\
    }									\
  else									\
    bit <<= 1;								\
  input -= xstep;							\
  if (xmod)								\
    {									\
      xerror -= xmod;							\
      if (xerror < 0)							\
	{								\
	  xerror += d->dst_width;					\
	  input -= (width);						\
	}								\
    }									\
} while (0)

#define ADVANCE_BIDIRECTIONAL(d,bit,in,dir,width,xer,xstep,xmod,err,N,S) \
do									 \
{									 \
  int ii;								 \
  int jj;								 \
  for (ii = 0; ii < N; ii++)						 \
    for (jj = 0; jj < S; jj++)						 \
      err[ii][jj] += dir;						 \
  if (dir == 1)								 \
    ADVANCE_UNIDIRECTIONAL(d, bit, in, width, xer, xstep, xmod);	 \
  else									 \
    ADVANCE_REVERSE(d, bit, in, width, xer, xstep, xmod);		 \
} while (0)

#ifdef __cplusplus
  }
#endif

#endif /* GIMP_PRINT_INTERNAL_DITHER_IMPL_H */
/*
 * End of "$Id: dither-impl.h,v 1.5 2003/05/05 00:36:03 rlk Exp $".
 */
