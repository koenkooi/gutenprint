/*
 * "$Id: dither-main.c,v 1.38 2003/11/14 23:47:13 rlk Exp $"
 *
 *   Dither routine entrypoints
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <gimp-print/gimp-print.h>
#include "gimp-print-internal.h"
#include <gimp-print/gimp-print-intl-internal.h>
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#include <math.h>
#include <string.h>
#include "dither-impl.h"
#include "generic-options.h"

static const stpi_dither_algorithm_t dither_algos[] =
{
  /* Note to translators: "EvenTone" is the proper name, rather than a */
  /* descriptive name, of this algorithm. */
  { "None",           N_ ("Default"),                -1 },
  { "EvenTone",       N_ ("EvenTone"),               D_EVENTONE },
  { "HybridEvenTone", N_ ("Hybrid EvenTone"),        D_HYBRID_EVENTONE },
  { "UniTone",        N_ ("UniTone"),                D_UNITONE },
  { "HybridUniTone",  N_ ("Hybrid UniTone"),         D_HYBRID_UNITONE },
  { "Adaptive",	      N_ ("Adaptive Hybrid"),        D_ADAPTIVE_HYBRID },
  { "Ordered",	      N_ ("Ordered"),                D_ORDERED },
  { "Fast",	      N_ ("Fast"),                   D_FAST },
  { "VeryFast",	      N_ ("Very Fast"),              D_VERY_FAST },
  { "Floyd",	      N_ ("Hybrid Floyd-Steinberg"), D_FLOYD_HYBRID }
};

static const int num_dither_algos = sizeof(dither_algos)/sizeof(stpi_dither_algorithm_t);


/*
 * Bayer's dither matrix using Judice, Jarvis, and Ninke recurrence relation
 * http://www.cs.rit.edu/~sxc7922/Project/CRT.htm
 */

static const unsigned sq2[] =
{
  0, 2,
  3, 1
};

static const stp_parameter_t dither_parameters[] =
{
  {
    "Density", N_("Density"), N_("Output Level Adjustment"),
    N_("Adjust the density (amount of ink) of the print. "
       "Reduce the density if the ink bleeds through the "
       "paper or smears; increase the density if black "
       "regions are not solid."),
    STP_PARAMETER_TYPE_DOUBLE, STP_PARAMETER_CLASS_OUTPUT,
    STP_PARAMETER_LEVEL_ADVANCED, 0, 1, -1, 1
  },
  {
    "DitherAlgorithm", N_("Dither Algorithm"), N_("Screening Adjustment"),
    N_("Choose the dither algorithm to be used.\n"
       "Adaptive Hybrid usually produces the best all-around quality.\n"
       "EvenTone is a new, experimental algorithm that often produces excellent results.\n"
       "Ordered is faster and produces almost as good quality on photographs.\n"
       "Fast and Very Fast are considerably faster, and work well for text and line art.\n"
       "Hybrid Floyd-Steinberg generally produces inferior output."),
    STP_PARAMETER_TYPE_STRING_LIST, STP_PARAMETER_CLASS_OUTPUT,
    STP_PARAMETER_LEVEL_ADVANCED2, 1, 1, -1, 1
  },
};

static const int dither_parameter_count =
sizeof(dither_parameters) / sizeof(const stp_parameter_t);

stp_parameter_list_t
stpi_dither_list_parameters(stp_const_vars_t v)
{
  stp_parameter_list_t *ret = stp_parameter_list_create();
  int i;
  for (i = 0; i < dither_parameter_count; i++)
    stp_parameter_list_add_param(ret, &(dither_parameters[i]));
  return ret;
}

void
stpi_dither_describe_parameter(stp_const_vars_t v, const char *name,
			       stp_parameter_t *description)
{
  int i;
  description->p_type = STP_PARAMETER_TYPE_INVALID;
  if (name == NULL)
    return;
  description->deflt.str = NULL;
  if (strcmp(name, "Density") == 0)
    {
      stpi_fill_parameter_settings(description, &(dither_parameters[0]));
      description->bounds.dbl.upper = 8.0;
      description->bounds.dbl.lower = 0.1;
      description->deflt.dbl = 1.0;
    }
  else if (strcmp(name, "DitherAlgorithm") == 0)
    {
      if (stp_check_string_parameter(v, "Quality", STP_PARAMETER_ACTIVE) &&
	  stpi_get_quality_by_name(stp_get_string_parameter(v, "Quality")))
	description->is_active = 0;
      else
	{
	  stpi_fill_parameter_settings(description, &(dither_parameters[1]));
	  description->bounds.str = stp_string_list_create();
	  for (i = 0; i < num_dither_algos; i++)
	    {
	      const stpi_dither_algorithm_t *dt = &dither_algos[i];
	      stp_string_list_add_string(description->bounds.str,
					 dt->name, dt->text);
	    }
	  description->deflt.str =
	    stp_string_list_param(description->bounds.str, 0)->name;
	}
    }
  else
    return;
  if (stp_check_string_parameter(v, "Quality", STP_PARAMETER_ACTIVE) &&
      stpi_get_quality_by_name(stp_get_string_parameter(v, "Quality")))
    description->is_active = 0;
  else if (stp_check_string_parameter(v, "ImageType", STP_PARAMETER_ACTIVE) &&
	   strcmp(stp_get_string_parameter(v, "ImageType"), "None") != 0 &&
	   description->p_level > STP_PARAMETER_LEVEL_BASIC)
    description->is_active = 0;
}

#define RETURN_DITHERFUNC(func, v)					\
do									\
{									\
  stpi_dprintf(STPI_DBG_COLORFUNC, v, "ditherfunc %s\n", #func);	\
  return (func);							\
} while (0)

static stpi_ditherfunc_t *
stpi_set_dither_function(stp_vars_t v, int image_bpp)
{
  const stpi_quality_t *quality = NULL;
  const char *image_type = stp_get_string_parameter(v, "ImageType");
  stpi_dither_t *d = (stpi_dither_t *) stpi_get_component_data(v, "Dither");
  int i;
  const char *algorithm = stp_get_string_parameter(v, "DitherAlgorithm");
  d->stpi_dither_type = -1;
  if (stp_check_string_parameter(v, "Quality", STP_PARAMETER_ACTIVE))
    quality = stpi_get_quality_by_name(stp_get_string_parameter(v, "Quality"));
  
  if (image_type)
    {
      if (strcmp(image_type, "Text") == 0)
	d->stpi_dither_type = D_VERY_FAST;
    }
  if (quality && d->stpi_dither_type == -1)
    {
      switch (quality->quality_level)
	{
	case 0:
	case 1:
	  d->stpi_dither_type = D_VERY_FAST;
	  break;
	case 2:
	case 3:
	  if (image_type && strcmp(image_type, "LineArt") == 0)
	    d->stpi_dither_type = D_VERY_FAST;
	  else
	    d->stpi_dither_type = D_FAST;
	  break;
	case 4:
	  if (image_type &&
	      (strcmp(image_type, "LineArt") == 0 ||
	       strcmp(image_type, "TextGraphics") == 0))
	    d->stpi_dither_type = D_ADAPTIVE_HYBRID;
	  else
	    d->stpi_dither_type = D_ORDERED;
	  break;
	case 5:
	  if (image_type &&
	      (strcmp(image_type, "LineArt") == 0 ||
	       strcmp(image_type, "TextGraphics") == 0))
	    d->stpi_dither_type = D_HYBRID_EVENTONE;
	  else if (image_type && (strcmp(image_type, "Photo") == 0))
	    d->stpi_dither_type = D_EVENTONE;
	  else
	    d->stpi_dither_type = D_ORDERED;
	  break;
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	default:
	  if (image_type &&
	      (strcmp(image_type, "LineArt") == 0 ||
	       strcmp(image_type, "TextGraphics") == 0))
	    d->stpi_dither_type = D_HYBRID_EVENTONE;
	  else
	    d->stpi_dither_type = D_EVENTONE;
	  break;
	}
      /* EvenTone performs poorly if the aspect ratio is greater than 2 */
      if ((d->stpi_dither_type & (D_EVENTONE | D_UNITONE)) &&
	  (d->x_aspect > 2 || d->y_aspect > 2))
	d->stpi_dither_type = D_ADAPTIVE_HYBRID;
    }
  else if (algorithm)
    {
      for (i = 0; i < num_dither_algos; i++)
	{
	  if (!strcmp(algorithm, _(dither_algos[i].name)))
	    {
	      d->stpi_dither_type = dither_algos[i].id;
	      break;
	    }
	}
      if (d->stpi_dither_type == -1)
	{
	  d->stpi_dither_type = D_EVENTONE;
	  /* EvenTone performs poorly if the aspect ratio is greater than 2 */
	  if ((d->stpi_dither_type & (D_EVENTONE | D_UNITONE)) &&
	      (d->x_aspect > 2 || d->y_aspect > 2))
	    d->stpi_dither_type = D_ADAPTIVE_HYBRID;
	}	
    }
  switch (d->stpi_dither_type)
    {
    case D_VERY_FAST:
      RETURN_DITHERFUNC(stpi_dither_very_fast, v);
    case D_ORDERED:
    case D_FAST:
      RETURN_DITHERFUNC(stpi_dither_ordered, v);
    case D_HYBRID_EVENTONE:
    case D_EVENTONE:
      RETURN_DITHERFUNC(stpi_dither_et, v);
    case D_HYBRID_UNITONE:
    case D_UNITONE:
      RETURN_DITHERFUNC(stpi_dither_ut, v);
    default:
      RETURN_DITHERFUNC(stpi_dither_ed, v);
    }
}

void
stpi_dither_set_adaptive_limit(stp_vars_t v, double limit)
{
  stpi_dither_t *d = (stpi_dither_t *) stpi_get_component_data(v, "Dither");
  d->adaptive_limit = limit;
}

void
stpi_dither_set_ink_spread(stp_vars_t v, int spread)
{
  stpi_dither_t *d = (stpi_dither_t *) stpi_get_component_data(v, "Dither");
  SAFE_FREE(d->offset0_table);
  SAFE_FREE(d->offset1_table);
  if (spread >= 16)
    {
      d->spread = 16;
    }
  else
    {
      int max_offset;
      int i;
      d->spread = spread;
      max_offset = (1 << (16 - spread)) + 1;
      d->offset0_table = stpi_malloc(sizeof(int) * max_offset);
      d->offset1_table = stpi_malloc(sizeof(int) * max_offset);
      for (i = 0; i < max_offset; i++)
	{
	  d->offset0_table[i] = (i + 1) * (i + 1);
	  d->offset1_table[i] = ((i + 1) * i) / 2;
	}
    }
  d->spread_mask = (1 << d->spread) - 1;
}

void
stpi_dither_set_randomizer(stp_vars_t v, int i, double val)
{
  stpi_dither_t *d = (stpi_dither_t *) stpi_get_component_data(v, "Dither");
  if (i < 0 || i >= CHANNEL_COUNT(d))
    return;
  CHANNEL(d, i).randomizer = val * 65535;
}

static void
stpi_dither_free(void *vd)
{
  stpi_dither_t *d = (stpi_dither_t *) vd;
  int j;
  if (d->aux_freefunc)
    (d->aux_freefunc)(d);
  for (j = 0; j < CHANNEL_COUNT(d); j++)
    stpi_dither_channel_destroy(&(CHANNEL(d, j)));
  SAFE_FREE(d->offset0_table);
  SAFE_FREE(d->offset1_table);
  stpi_dither_matrix_destroy(&(d->dither_matrix));
  stpi_dither_matrix_destroy(&(d->transition_matrix));
  stpi_free(d->channel);
  stpi_free(d->channel_index);
  stpi_free(d->subchannel_count);
  stpi_free(d);
}

void
stpi_dither_init(stp_vars_t v, stp_image_t *image, int out_width,
		 int xdpi, int ydpi)
{
  int in_width = stpi_image_width(image);
  int image_bpp = stpi_image_bpp(image);
  stpi_dither_t *d = stpi_zalloc(sizeof(stpi_dither_t));

  stpi_allocate_component_data(v, "Dither", NULL, stpi_dither_free, d);

  d->finalized = 0;
  d->error_rows = ERROR_ROWS;
  d->d_cutoff = 4096;

  d->offset0_table = NULL;
  d->offset1_table = NULL;
  if (xdpi > ydpi)
    {
      d->x_aspect = 1;
      d->y_aspect = xdpi / ydpi;
    }
  else
    {
      d->x_aspect = ydpi / xdpi;
      d->y_aspect = 1;
    }
  d->ditherfunc = stpi_set_dither_function(v, image_bpp);
  d->transition = 1.0;
  d->adaptive_limit = .75 * 65535;

  /*
   * For hybrid EvenTone we want to use the good matrix.  For regular
   * EvenTone, we don't need to pay the cost.
   */
  
  if (d->stpi_dither_type == D_VERY_FAST || d->stpi_dither_type == D_FAST ||
      d->stpi_dither_type == D_EVENTONE)
    {
      if (stp_check_int_parameter(v, "DitherVeryFastSteps",
				  STP_PARAMETER_ACTIVE))
	stpi_dither_set_iterated_matrix
	  (v, 2, stp_get_int_parameter(v, "DitherVeryFastSteps"), sq2, 0, 2,4);
      else
	stpi_dither_set_iterated_matrix(v, 2, DITHER_FAST_STEPS, sq2, 0, 2, 4);
    }
  else if (stp_check_array_parameter(v, "DitherMatrix",
				     STP_PARAMETER_ACTIVE) &&
	   (stpi_dither_matrix_validate_array
	    (stp_get_array_parameter(v, "DitherMatrix"))))
    {
      stpi_dither_set_matrix_from_dither_array
	(v, stp_get_array_parameter(v, "DitherMatrix"), 0);
    }
  else
    {
      stp_array_t array;
      int transposed;
	array = stpi_find_standard_dither_array(d->y_aspect, d->x_aspect);
      transposed = d->y_aspect < d->x_aspect ? 1 : 0;
      if (array)
	{
	  stpi_dither_set_matrix_from_dither_array(v, array, transposed);
	  stp_array_destroy(array);
	}
      else
	{
	  stpi_eprintf(v, "Cannot find dither matrix file!  Aborting.\n");
	  stpi_abort();
	}
    }
  stpi_dither_set_transition(v, 0.7);

  d->src_width = in_width;
  d->dst_width = out_width;

  stpi_dither_set_ink_spread(v, 13);
  d->channel_count = 0;
}

void
stpi_dither_reverse_row_ends(stpi_dither_t *d)
{
  int i;
  for (i = 0; i < CHANNEL_COUNT(d); i++)
    {
      int tmp = CHANNEL(d, i).row_ends[0];
      CHANNEL(d, i).row_ends[0] =
	CHANNEL(d, i).row_ends[1];
      CHANNEL(d, i).row_ends[1] = tmp;
    }
}

int
stpi_dither_get_first_position(stp_vars_t v, int color, int subchannel)
{
  stpi_dither_t *d = (stpi_dither_t *) stpi_get_component_data(v, "Dither");
  int channel = stpi_dither_translate_channel(v, color, subchannel);
  if (channel < 0)
    return -1;
  return CHANNEL(d, channel).row_ends[0];
}

int
stpi_dither_get_last_position(stp_vars_t v, int color, int subchannel)
{
  stpi_dither_t *d = (stpi_dither_t *) stpi_get_component_data(v, "Dither");
  int channel = stpi_dither_translate_channel(v, color, subchannel);
  if (channel < 0)
    return -1;
  return CHANNEL(d, channel).row_ends[1];
}

int *
stpi_dither_get_errline(stpi_dither_t *d, int row, int color)
{
  stpi_dither_channel_t *dc;
  if (row < 0 || color < 0 || color >= CHANNEL_COUNT(d))
    return NULL;
  dc = &(CHANNEL(d, color));
  if (!dc->errs)
    dc->errs = stpi_zalloc(d->error_rows * sizeof(int *));
  if (!dc->errs[row % dc->error_rows])
    {
      int size = 2 * MAX_SPREAD + (16 * ((d->dst_width + 7) / 8));
      dc->errs[row % dc->error_rows] = stpi_zalloc(size * sizeof(int));
    }
  return dc->errs[row % dc->error_rows] + MAX_SPREAD;
}

void
stpi_dither_internal(stp_vars_t v, int row, const unsigned short *input,
		     int duplicate_line, int zero_mask,
		     const unsigned char *mask)
{
  int i;
  stpi_dither_t *d = (stpi_dither_t *) stpi_get_component_data(v, "Dither");
  stpi_dither_finalize(v);
  stpi_dither_matrix_set_row(&(d->dither_matrix), row);
  stpi_dither_matrix_set_row(&(d->transition_matrix), row);
  for (i = 0; i < CHANNEL_COUNT(d); i++)
    {
      CHANNEL(d, i).ptr = CHANNEL(d, i).ptr;
      if (CHANNEL(d, i).ptr)
	  memset(CHANNEL(d, i).ptr, 0,
		 (d->dst_width + 7) / 8 * CHANNEL(d, i).signif_bits);
      CHANNEL(d, i).row_ends[0] = -1;
      CHANNEL(d, i).row_ends[1] = -1;

      stpi_dither_matrix_set_row(&(CHANNEL(d, i).dithermat), row);
      stpi_dither_matrix_set_row(&(CHANNEL(d, i).pick), row);
    }
  d->ptr_offset = 0;
  (d->ditherfunc)(v, row, input, duplicate_line, zero_mask, mask);
}

void
stpi_dither(stp_vars_t v, int row, int duplicate_line, int zero_mask,
	    const unsigned char *mask)
{
  const unsigned short *input = stpi_channel_get_output(v);
  stpi_dither_internal(v, row, input, duplicate_line, zero_mask, mask);
}