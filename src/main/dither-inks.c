/*
 * "$Id: dither-inks.c,v 1.7.2.11 2003/05/24 22:37:36 rlk Exp $"
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

int
stpi_dither_translate_channel(stp_vars_t v, unsigned channel,
			      unsigned subchannel)
{
  stpi_dither_t *d = (stpi_dither_t *) stpi_get_component_data(v, "Dither");
  unsigned chan_idx;
  if (channel >= d->channel_count)
    return -1;
  if (subchannel >= d->subchannel_count[channel])
    return -1;
  chan_idx = d->channel_index[channel];
  return chan_idx + subchannel;
}

unsigned char *
stpi_dither_get_channel(stp_vars_t v, unsigned channel, unsigned subchannel)
{
  stpi_dither_t *d = (stpi_dither_t *) stpi_get_component_data(v, "Dither");
  int place = stpi_dither_translate_channel(v, channel, subchannel);
  if (place >= 0)
    return d->channel[place].ptr;
  else
    return NULL;
}

static void
insert_channel(stp_vars_t v, stpi_dither_t *d, int channel)
{
  unsigned oc = d->channel_count;
  int i;
  d->channel_index =
    stpi_realloc (d->channel_index, sizeof(unsigned) * (channel + 1));
  d->subchannel_count =
    stpi_realloc (d->subchannel_count, sizeof(unsigned) * (channel + 1));
  for (i = oc; i < channel + 1; i++)
    {
      if (oc == 0)
	d->channel_index[i] = 0;
      else
	d->channel_index[i] =
	  d->channel_index[oc - 1] + d->subchannel_count[oc - 1];
      d->subchannel_count[i] = 0;
    }
  d->channel_count = channel + 1;
}

void
stpi_dither_channel_destroy(stpi_dither_channel_t *channel)
{
  int i;
  SAFE_FREE(channel->vals);
  SAFE_FREE(channel->ink_list);
  if (channel->errs)
    {
      for (i = 0; i < channel->error_rows; i++)
	SAFE_FREE(channel->errs[i]);
      SAFE_FREE(channel->errs);
    }
  SAFE_FREE(channel->errs);
  SAFE_FREE(channel->ranges);
  if (channel->shades)
    {
      for (i = 0; i < channel->numshades; i++)
	{
	  SAFE_FREE(channel->shades[i].dotsizes);
	  SAFE_FREE(channel->shades[i].errs);
	  SAFE_FREE(channel->shades[i].et_dis);
	}
      SAFE_FREE(channel->shades);
    }
  stpi_dither_matrix_destroy(&(channel->pick));
  stpi_dither_matrix_destroy(&(channel->dithermat));
}  

static void
initialize_channel(stp_vars_t v, int channel, int subchannel)
{
  stpi_dither_t *d = (stpi_dither_t *) stpi_get_component_data(v, "Dither");
  int idx = stpi_dither_translate_channel(v, channel, subchannel);
  stpi_dither_channel_t *chan = &(CHANNEL(d, idx));
  stpi_shade_t shade;
  stpi_dotsize_t dot;
  memset(chan, 0, sizeof(stpi_dither_channel_t));
  stpi_dither_matrix_clone(&(d->dither_matrix), &(chan->dithermat), 0, 0);
  stpi_dither_matrix_clone(&(d->transition_matrix), &(chan->pick), 0, 0);
  shade.dot_sizes = &dot;
  shade.value = 65535.0;
  shade.numsizes = 1;
  dot.bit_pattern = 1;
  dot.value = 1.0;
  stpi_dither_set_inks(v, channel, 1, &shade, 1.0);
}

static void
insert_subchannel(stp_vars_t v, stpi_dither_t *d, int channel, int subchannel)
{
  int i;
  unsigned oc = d->subchannel_count[channel];
  unsigned increment = subchannel - oc + 1;
  unsigned old_place = d->channel_index[channel] + oc;
  stpi_dither_channel_t *nc =
    stpi_malloc(sizeof(stpi_dither_channel_t) *
		(d->total_channel_count + increment));
      
  if (d->channel)
    {
      /*
       * Copy the old channels, including all subchannels of the current
       * channel that already existed.
       */
      memcpy(nc, d->channel, sizeof(stpi_dither_channel_t) * old_place);
      if (old_place < d->total_channel_count)
	/*
	 * If we're inserting a new subchannel in the middle somewhere,
	 * we need to move everything else up
	 */
	memcpy(nc + old_place + increment,
	       d->channel + old_place,
	       (sizeof(stpi_dither_channel_t) *
		(d->total_channel_count - old_place)));
      stpi_free(d->channel);
    }
  d->channel = nc;
  if (channel < d->channel_count - 1)
    /* Now fix up the subchannel offsets */
    for (i = channel + 1; i < d->channel_count; i++)
      d->channel_index[i] += increment;
  d->subchannel_count[channel] = subchannel + 1;
  d->total_channel_count += increment;
  for (i = oc; i < oc + increment; i++)
    initialize_channel(v, channel, i);
}

void
stpi_dither_finalize(stp_vars_t v)
{
  stpi_dither_t *d = (stpi_dither_t *) stpi_get_component_data(v, "Dither");
  if (!d->finalized)
    {
      int i;
      unsigned rc = 1 + (unsigned) ceil(sqrt(CHANNEL_COUNT(d)));
      unsigned x_n = d->dither_matrix.x_size / rc;
      unsigned y_n = d->dither_matrix.y_size / rc;
      for (i = 0; i < CHANNEL_COUNT(d); i++)
	{
	  stpi_dither_channel_t *dc = &(CHANNEL(d, i));
	  stpi_dither_matrix_clone(&(d->dither_matrix), &(dc->dithermat),
				   x_n * (i % rc), y_n * (i / rc));
	  stpi_dither_matrix_clone(&(d->dither_matrix), &(dc->pick),
				   x_n * (i % rc), y_n * (i / rc));
	}
      d->finalized = 1;
    }
}

void
stpi_dither_add_channel(stp_vars_t v, unsigned char *data,
			unsigned channel, unsigned subchannel)
{
  stpi_dither_t *d = (stpi_dither_t *) stpi_get_component_data(v, "Dither");
  int idx;
  if (channel >= d->channel_count)
    insert_channel(v, d, channel);
  if (subchannel >= d->subchannel_count[channel])
    insert_subchannel(v, d, channel, subchannel);
  idx = stpi_dither_translate_channel(v, channel, subchannel);
  d->channel[idx].ptr = data;
}

static void
stpi_dither_finalize_ranges(stp_vars_t v, stpi_dither_channel_t *s)
{
  stpi_dither_t *d = (stpi_dither_t *) stpi_get_component_data(v, "Dither");
  int i;
  unsigned lbit = s->bit_max;
  s->signif_bits = 0;
  while (lbit > 0)
    {
      s->signif_bits++;
      lbit >>= 1;
    }

  s->maxdot = 0;

  for (i = 0; i < s->nlevels; i++)
    {
      if (s->ranges[i].lower->dot_size == s->ranges[i].upper->dot_size)
	s->ranges[i].is_same_ink = 1;
      else
	s->ranges[i].is_same_ink = 0;
      if (s->ranges[i].range_span > 0 && s->ranges[i].value_span > 0)
	s->ranges[i].is_equal = 0;
      else
	s->ranges[i].is_equal = 1;

      if (s->ranges[i].lower->dot_size > s->maxdot)
	s->maxdot = s->ranges[i].lower->dot_size;
      if (s->ranges[i].upper->dot_size > s->maxdot)
	s->maxdot = s->ranges[i].upper->dot_size;

      stpi_dprintf(STPI_DBG_INK, v,
		   "    level %d value[0] %d value[1] %d range[0] %d range[1] %d\n",
		   i, s->ranges[i].lower->value, s->ranges[i].upper->value,
		   s->ranges[i].lower->range, s->ranges[i].upper->range);
      stpi_dprintf(STPI_DBG_INK, v,
		   "    xvalue[0] %d xvalue[1] %d\n",
		   s->ranges[i].lower->xvalue, s->ranges[i].upper->xvalue);
      stpi_dprintf(STPI_DBG_INK, v,
		   "       bits[0] %d bits[1] %d",
		   s->ranges[i].lower->bits, s->ranges[i].upper->bits);
      stpi_dprintf(STPI_DBG_INK, v,
		   "       rangespan %d valuespan %d same_ink %d equal %d\n",
		   s->ranges[i].range_span, s->ranges[i].value_span,
		   s->ranges[i].is_same_ink, s->ranges[i].is_equal);
      if (i > 0 && s->ranges[i].lower->range >= d->adaptive_limit)
	{
	  d->adaptive_limit = s->ranges[i].lower->range + 1;
	  if (d->adaptive_limit > 65535)
	    d->adaptive_limit = 65535;
	  stpi_dprintf(STPI_DBG_INK, v, "Setting adaptive limit to %d\n",
		       d->adaptive_limit);
	}
    }
  if (s->nlevels == 1 && s->ranges[0].upper->bits == 1)
    s->very_fast = 1;
  else
    s->very_fast = 0;

  stpi_dprintf(STPI_DBG_INK, v,
	       "  bit_max %d signif_bits %d\n", s->bit_max, s->signif_bits);
}

static void
stpi_dither_set_ranges(stp_vars_t v, stpi_dither_channel_t *s,
		       const stpi_shade_t *shade, double density)
{
  double sdensity = s->density_adjustment;
  const stpi_dotsize_t *ranges = shade->dot_sizes;
  int nlevels = shade->numsizes;
  int i;
  SAFE_FREE(s->ranges);
  SAFE_FREE(s->ink_list);

  s->nlevels = nlevels > 1 ? nlevels + 1 : nlevels;
  s->ranges = (stpi_dither_segment_t *)
    stpi_zalloc(s->nlevels * sizeof(stpi_dither_segment_t));
  s->ink_list = (stpi_ink_defn_t *)
    stpi_zalloc((s->nlevels + 1) * sizeof(stpi_ink_defn_t));
  s->bit_max = 0;
/*  density *= sdensity; */
  s->density = density * 65535;
  stpi_init_debug_messages(v);
  stpi_dprintf(STPI_DBG_INK, v,
	      "stpi_dither_set_generic_ranges nlevels %d density %f\n",
	      nlevels, density);
  for (i = 0; i < nlevels; i++)
    stpi_dprintf(STPI_DBG_INK, v,
		"  level %d value %f pattern %x", i,
		ranges[i].value, ranges[i].bit_pattern);
  s->ranges[0].lower = &s->ink_list[0];
  s->ranges[0].upper = &s->ink_list[1];
  s->ink_list[0].range = 0;
  s->ink_list[0].value = ranges[0].value * 65535.0;
  s->ink_list[0].xvalue = ranges[0].value * 65535.0 * sdensity;
  s->ink_list[0].bits = ranges[0].bit_pattern;
  if (nlevels == 1)
    s->ink_list[1].range = 65535;
  else
    s->ink_list[1].range = ranges[0].value * 65535.0 * density;
  if (s->ink_list[1].range > 65535)
    s->ink_list[1].range = 65535;
  s->ink_list[1].value = ranges[0].value * 65535.0;
  if (s->ink_list[1].value > 65535)
    s->ink_list[1].value = 65535;
  s->ink_list[1].xvalue = ranges[0].value * 65535.0 * sdensity;
  s->ink_list[1].bits = ranges[0].bit_pattern;
  if (ranges[0].bit_pattern > s->bit_max)
    s->bit_max = ranges[0].bit_pattern;
  s->ranges[0].range_span = s->ranges[0].upper->range;
  s->ranges[0].value_span = 0;
  if (s->nlevels > 1)
    {
      for (i = 1; i < nlevels; i++)
	{
	  int l = i + 1;
	  s->ranges[i].lower = &s->ink_list[i];
	  s->ranges[i].upper = &s->ink_list[l];

	  s->ink_list[l].range =
	    (ranges[i].value + ranges[i].value) * 32768.0 * density;
	  if (s->ink_list[l].range > 65535)
	    s->ink_list[l].range = 65535;
	  s->ink_list[l].value = ranges[i].value * 65535.0;
	  if (s->ink_list[l].value > 65535)
	    s->ink_list[l].value = 65535;
	  s->ink_list[l].xvalue = ranges[i].value * 65535.0 * sdensity;
	  s->ink_list[l].bits = ranges[i].bit_pattern;
	  if (ranges[i].bit_pattern > s->bit_max)
	    s->bit_max = ranges[i].bit_pattern;
	  s->ranges[i].range_span =
	    s->ink_list[l].range - s->ink_list[i].range;
	  s->ranges[i].value_span =
	    s->ink_list[l].value - s->ink_list[i].value;
	}
      s->ranges[i].lower = &s->ink_list[i];
      s->ranges[i].upper = &s->ink_list[i+1];
      s->ink_list[i+1] = s->ink_list[i];
      s->ink_list[i+1].range = 65535;
      s->ranges[i].range_span = s->ink_list[i+1].range - s->ink_list[i].range;
      s->ranges[i].value_span = s->ink_list[i+1].value - s->ink_list[i].value;
    }
  stpi_dither_finalize_ranges(v, s);
  stpi_flush_debug_messages(v);
}

void
stpi_dither_set_inks_simple(stp_vars_t v, int color, int nlevels,
			    const double *levels, double density)
{
  stpi_shade_t s;
  stpi_dotsize_t *d = stpi_malloc(nlevels * sizeof(stpi_dotsize_t));
  int i;
  s.dot_sizes = d;
  s.value = 65535.0;
  s.numsizes = nlevels;

  for (i = 0; i < nlevels; i++)
    {
      d[i].bit_pattern = i + 1;
      d[i].value = levels[i];
    }
  stpi_dither_set_inks(v, color, 1, &s, density);
  stpi_free(d);
}

void
stpi_dither_set_inks(stp_vars_t v, int color, int nshades,
		     const stpi_shade_t *shades, double density)
{
  int i, j;

  /* Setting ink_gamma to different values changes the amount
     of photo ink used (or other lighter inks). Set to 0 it uses
     the maximum amount of ink possible without soaking the paper.
     Set to 1.0 it is very conservative.
     0.5 is probably a good compromise
  */

  const double ink_gamma = 0.5;

  stpi_dither_t *d = (stpi_dither_t *) stpi_get_component_data(v, "Dither");

  stpi_channel_reset_channel(v, color);

  for (i=0; i < nshades; i++)
    {
      int idx = stpi_dither_translate_channel(v, color, i);
      stpi_dither_channel_t *dc = &(CHANNEL(d, idx));
      stpi_shade_segment_t *sp;

      if (dc->shades)
	{
	  for (j = 0; j < dc->numshades; j++)
	    {
	      SAFE_FREE(dc->shades[j].dotsizes);
	      SAFE_FREE(dc->shades[j].errs);
	    }
	  SAFE_FREE(dc->shades);
	}

      dc->numshades = 1;
      dc->shades = stpi_zalloc(dc->numshades * sizeof(stpi_shade_segment_t));
      dc->density_adjustment = stp_get_float_parameter(v, "Density");
      dc->sqrt_density_adjustment = sqrt(density);

      sp = &dc->shades[0];
      sp->value = 1.0;
      stpi_channel_add(v, color, i, shades[i].value);
      sp->density = 65536.0;
      if (i == 0)
	{
	  sp->lower = 0;
	  sp->trans = 0;
	}
      else
	{
	  double k;
	  k = 65536.0 * density * pow(shades[i-1].value, ink_gamma);
	  sp->lower = k * shades[i-1].value + 0.5;
	  sp->trans = k * shades[i].value + 0.5;

	  /* Precompute some values */
	  sp->div1 = (sp->density * (sp->trans - sp->lower)) / sp->trans;
	  sp->div2 = (sp[-1].density * (sp->trans - sp->lower)) / sp->lower;
	}

      sp->numdotsizes = shades[i].numsizes;
      sp->dotsizes = stpi_zalloc(sp->numdotsizes * sizeof(stpi_ink_defn_t));
      if (idx >= 0)
	stpi_dither_set_ranges(v, &(CHANNEL(d, idx)), &shades[i], density);
      for (j=0; j < sp->numdotsizes; j++)
	{
	  stpi_ink_defn_t *ip = &sp->dotsizes[j];
	  const stpi_dotsize_t *dp = &shades[i].dot_sizes[j];
	  ip->value = dp->value * sp->density + 0.5;
	  ip->range = density * ip->value;
	  ip->bits = dp->bit_pattern;
	  ip->dot_size = dp->value * 65536.0 + 0.5;
	}
    }
}
