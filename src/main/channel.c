/*
 * "$Id: channel.c,v 1.1.2.2 2003/05/16 01:31:22 rlk Exp $"
 *
 *   Dither routine entrypoints
 *
 *   Copyright 2003 Robert Krawitz (rlk@alum.mit.edu)
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

typedef struct
{
  double value;
  double lower;
  double upper;
} stpi_subchannel_t;

typedef struct
{
  unsigned subchannel_count;
  stpi_subchannel_t *sc;
  unsigned short *lut;
} stpi_channel_t;

typedef struct
{
  unsigned channel_count;
  unsigned total_channels;
  unsigned input_channels;
  stpi_channel_t *c;
  size_t width;
  unsigned short *input_data;
  unsigned short *data;
} stpi_channel_group_t;


static void
stpi_channel_clear(void *vc)
{
  stpi_channel_group_t *cg = (stpi_channel_group_t *) vc;
  int i;
  if (cg->channel_count > 0)
    {
      for (i = 0; i < cg->channel_count; i++)
	{
	  SAFE_FREE(cg->c[i].sc);
	  SAFE_FREE(cg->c[i].lut);
	}
    }
  if (cg->data != cg->input_data)
    SAFE_FREE(cg->data);
  SAFE_FREE(cg->input_data);
  SAFE_FREE(cg->c);
}

void
stpi_channel_reset(stp_vars_t v)
{
  stpi_channel_group_t *cg =
    ((stpi_channel_group_t *) stpi_get_component_data(v, "Channel"));
  if (cg)
    stpi_channel_clear(cg);
}

static void
stpi_channel_free(void *vc)
{
  stpi_channel_clear(vc);
  stpi_free(vc);
}

void
stpi_channel_add(stp_vars_t v, unsigned channel, unsigned subchannel,
		 double value)
{
  stpi_channel_group_t *cg =
    ((stpi_channel_group_t *) stpi_get_component_data(v, "Channel"));
  stpi_channel_t *chan;
  if (!cg)
    {
      cg = stpi_zalloc(sizeof(stpi_channel_group_t));
      stpi_allocate_component_data(v, "Channel", NULL, stpi_channel_free, cg);
    }				   
  if (channel >= cg->channel_count)
    {
      unsigned oc = cg->channel_count;
      cg->c = stpi_realloc(cg->c, sizeof(stpi_channel_t) * (channel + 1));
      memset(cg->c + oc, 0, sizeof(stpi_channel_t) * (channel + 1 - oc));
      if (channel >= cg->channel_count)
	cg->channel_count = channel + 1;
    }
  chan = cg->c + channel;
  if (subchannel >= chan->subchannel_count)
    {
      unsigned oc = chan->subchannel_count;
      chan->sc =
	stpi_realloc(chan->sc, sizeof(stpi_subchannel_t) * (subchannel + 1));
      (void) memset
	(chan->sc + oc, 0, sizeof(stpi_subchannel_t) * (subchannel + 1 - oc));
      chan->sc[subchannel].value = value;
      if (subchannel >= chan->subchannel_count)
	chan->subchannel_count = subchannel + 1;
    }
  chan->sc[subchannel].value = value;
}

static int
input_needs_splitting(stp_const_vars_t v)
{
  const stpi_channel_group_t *cg =
    ((const stpi_channel_group_t *) stpi_get_component_data(v, "Channel"));
#if 1
  return cg->total_channels != cg->input_channels;
#else
  int i;
  if (!cg || cg->channel_count <= 0)
    return 0;
  for (i = 0; i < cg->channel_count; i++)
    {
      if (cg->c[i].subchannel_count > 1)
	return 1;
    }
  return 0;
#endif
}

void
stpi_channel_initialize(stp_vars_t v, stp_image_t *image,
			int input_channel_count)
{
  stpi_channel_group_t *cg =
    ((stpi_channel_group_t *) stpi_get_component_data(v, "Channel"));
  int width = stpi_image_width(image);
  int i;
  if (!cg)
    {
      cg = stpi_zalloc(sizeof(stpi_channel_group_t));
      stpi_allocate_component_data(v, "Channel", NULL, stpi_channel_free, cg);
    }				   
  cg->data = stpi_malloc(sizeof(unsigned short) * cg->total_channels * width);
  if (!input_needs_splitting(v))
    {
      cg->data = cg->input_data;
      return;
    }
  cg->input_data =
    stpi_malloc(sizeof(unsigned short) * input_channel_count * width);
  cg->input_channels = input_channel_count;
  for (i = 0; i < cg->channel_count; i++)
    {
      stpi_channel_t *c = &(cg->c[i]);
      int sc = c->subchannel_count;
      if (sc > 1)
	{
	  int k;
	  int val = 65535;
	  c->lut = stpi_zalloc(sizeof(unsigned short) * sc * 65536);
	  for (k = 0; k < sc - 1; k++)
	    {
	      int next_breakpoint = c->sc[k + 1].value * 65536 / 2;
	      int range = val - next_breakpoint;
	      double upper = 1.0 / c->sc[k].value;
	      double lower = 1.0 / c->sc[k + 1].value;
	      for (; val >= next_breakpoint; val--)
		{
		  double where =
		    ((double) val - next_breakpoint) / (double) range;
		  c->lut[val * sc + k] = val * where * upper;
		  c->lut[val * sc + k + 1] = val * (1.0 - where) * lower;
		}
	    }
	  for (; val >= 0; val--)
	    {
	      c->lut[val * sc + (sc - 1)] = val / c->sc[sc - 1].value;
	    }
	}     
      cg->total_channels += c->subchannel_count;
    }
  cg->data = stpi_malloc(sizeof(unsigned short) * cg->total_channels * width);
  cg->width = width;
}

void
stpi_channel_convert(stp_const_vars_t v)
{
  stpi_channel_group_t *cg =
    ((stpi_channel_group_t *) stpi_get_component_data(v, "Channel"));
  const unsigned short *input;
  unsigned short *output;
  int i;
  if (!input_needs_splitting(v))
    return;
  input = cg->input_data;
  output = cg->data;
  for (i = 0; i < cg->width; i++)
    {
      int j;
      for (j = 0; j < cg->channel_count; j++)
	{
	  stpi_channel_t *c = &(cg->c[j]);
	  int s_count = c->subchannel_count;
	  if (s_count == 1)
	    *(output++) = *(input++);
	  else if (s_count > 1)
	    {
	      int k;
	      for (k = 0; k < s_count; k++)
		*(output++) = c->lut[*input * s_count + k];
	      input++;
	    }
	}
    }
}

unsigned short *
stpi_channel_get_input(stp_const_vars_t v)
{
  stpi_channel_group_t *cg =
    ((stpi_channel_group_t *) stpi_get_component_data(v, "Channel"));
  return (unsigned short *) cg->input_data;
}

unsigned short *
stpi_channel_get_output(stp_const_vars_t v)
{
  stpi_channel_group_t *cg =
    ((stpi_channel_group_t *) stpi_get_component_data(v, "Channel"));
  return cg->data;
}
