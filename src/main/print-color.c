/*
 * "$Id: print-color.c,v 1.7.2.4 2001/03/24 01:47:27 rlk Exp $"
 *
 *   Print plug-in color management for the GIMP.
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

/* #define PRINT_DEBUG */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <gimp-print.h>
#include <gimp-print-internal.h>
#include <gimp-print-intl-internal.h>
#include <math.h>
#include <limits.h>

typedef struct
{
  unsigned steps;
  unsigned short *composite;
  unsigned short *red;
  unsigned short *green;
  unsigned short *blue;
  unsigned shiftval;
  unsigned bin_size;
  unsigned bin_shift;
} lut_t;

/*
 * RGB to grayscale luminance constants...
 */

#define LUM_RED		31
#define LUM_GREEN	61
#define LUM_BLUE	8

/* rgb/hsl conversions taken from Gimp common/autostretch_hsv.c */

#define FMAX(a, b) ((a) > (b) ? (a) : (b))
#define FMIN(a, b) ((a) < (b) ? (a) : (b))

static inline void
calc_cmy_to_hsl(stp_color_output_t *cmy, int i, double *hue, double *sat,
		double *lightness)
{
  double cyan, magenta, yellow;
  double h, s, l;
  double min, max;
  double delta;
  int maxval;

  cyan   = cmy->c[i] / 65535.0;
  magenta = cmy->m[i] / 65535.0;
  yellow  = cmy->y[i] / 65535.0;

  if (cyan > magenta)
    {
      if (cyan > yellow)
	{
	  max = cyan;
	  maxval = 0;
	}
      else
	{
	  max = yellow;
	  maxval = 2;
	}
      min = FMIN(magenta, yellow);
    }
  else
    {
      if (magenta > yellow)
	{
	  max = magenta;
	  maxval = 1;
	}
      else
	{
	  max = yellow;
	  maxval = 2;
	}
      min = FMIN(cyan, yellow);
    }

  l = (max + min) / 2.0;
  delta = max - min;

  if (delta < .000001)	/* Suggested by Eugene Anikin <eugene@anikin.com> */
    {
      s = 0.0;
      h = 0.0;
    }
  else
    {
      if (l <= .5)
	s = delta / (max + min);
      else
	s = delta / (2 - max - min);

      if (maxval == 0)
	h = (magenta - yellow) / delta;
      else if (maxval == 1)
	h = 2 + (yellow - cyan) / delta;
      else
	h = 4 + (cyan - magenta) / delta;

      if (h < 0.0)
	h += 6.0;
      else if (h > 6.0)
	h -= 6.0;
    }

  *hue = h;
  *sat = s;
  *lightness = l;
}

static inline double
hsl_value(double n1, double n2, double hue)
{
  if (hue < 0)
    hue += 6.0;
  else if (hue > 6)
    hue -= 6.0;
  if (hue < 1.0)
    return (n1 + (n2 - n1) * hue);
  else if (hue < 3.0)
    return (n2);
  else if (hue < 4.0)
    return (n1 + (n2 - n1) * (4.0 - hue));
  else
    return (n1);
}

static inline void
calc_hsl_to_cmy(stp_color_output_t *cmy, int i, double h, double s, double l)
{
  if (s < .0000001)
    {
      if (l > 1)
	l = 1;
      else if (l < 0)
	l = 0;
      cmy->c[i] = l * 65535;
      cmy->m[i] = l * 65535;
      cmy->y[i] = l * 65535;
    }
  else
    {
      double m1, m2;
      double h1, h2;
      h1 = h + 2;
      h2 = h - 2;

      if (l < .5)
	m2 = l * (1 + s);
      else
	m2 = l + s - (l * s);
      m1 = (l * 2) - m2;
      cmy->c[i] = 65535 * hsl_value(m1, m2, h1);
      cmy->m[i] = 65535 * hsl_value(m1, m2, h);
      cmy->y[i] = 65535 * hsl_value(m1, m2, h2);
    }
}

static inline void
update_cmyk(unsigned short *cmy, int i)
{
  int c = cmy->c[i];
  int m = cmy->m[i];
  int y = cmy->y[i];
  int nc, nm, ny;
  int k;
  if (c == m && c == y)
    return;
  k = FMIN(FMIN(c, m), y);

  /*
   * This is an attempt to achieve better color balance.  The goal
   * is to weaken the pure cyan, magenta, and yellow and strengthen
   * pure red, green, and blue.
   *
   * We also don't want S=1 V=1 cyan to be 100% cyan; it's simply
   * too dark.
   */

  nc = (c * 3 + FMIN(c, FMAX(m, y)) * 4 + FMAX(m, y) * 0 + k) / 8;
  nm = (m * 3 + FMIN(m, FMAX(c, y)) * 4 + FMAX(c, y) * 0 + k) / 8;
  ny = (y * 3 + FMIN(y, FMAX(c, m)) * 4 + FMAX(c, m) * 0 + k) / 8;

  /*
   * Make sure we didn't go overboard.  We don't want to go too
   * close to white unnecessarily.
   */
  nc = c + (nc - c) / 3;
  nm = m + (nm - m) / 3;
  ny = y + (ny - y) / 3;

  if (nc > 65535)
    nc = 65535;
  if (nm > 65535)
    nm = 65535;
  if (ny > 65535)
    ny = 65535;

  cmy->c[i] = nc;
  cmy->m[i] = nm;
  cmy->y[i] = ny;
}

/*
 * A lot of this stuff needs to be factored out of here
 */
static inline unsigned short
lookup_value(unsigned short value, int lut_size, const unsigned short *lut,
	     unsigned shiftval, unsigned bin_size, unsigned bin_shift)
{
  unsigned subrange;
  unsigned remainder;
  unsigned below;
  unsigned above;
  if (lut_size == 65536)
    return lut[value];
  subrange = value >> bin_shift;
  remainder = value & (bin_size - 1);
  below = lut[subrange];
  if (remainder == 0)
    return below;
  if (subrange == (bin_size - 1))
    above = lut[subrange];
  else
    above = lut[subrange + 1];
  if (above == below)
    return above;
  else
    return below + (((above - below) * remainder) >> bin_shift);
}

/*
 * 'gray_to_gray()' - Convert grayscale image data to grayscale (brightness
 *                    adjusted).
 */

static void
gray_to_gray(const unsigned char *grayin,
	     stp_color_output_t *out,
	     int    	width,
	     int    	bpp,
	     const unsigned char *cmap,
	     const stp_vars_t	vars,
	     const double *hue_map,
	     const double *lum_map,
	     const double *sat_map)
{
  int i0 = -1;
  int i1 = -1;
  int use_previous = 0;
  int o0 = 0;
  lut_t *lut = (lut_t *)(stp_get_lut(vars));
  unsigned short *gray = out->k;
  double density = stp_get_density(vars);
  while (width > 0)
    {
      if (bpp == 1)
	{
	  /*
	   * No alpha in image...
	   */
	  if (i0 == grayin[0])
	    use_previous = 1;
	  else
	    {
	      use_previous = 0;
	      i0 = grayin[0];
	      gray[0] = lut->composite[grayin[0]];
	    }
	}
      else
	{
	  if (i0 == grayin[0] && i1 == grayin[1])
	    use_previous = 1;
	  else
	    {
	      use_previous = 0;
	      i0 = grayin[0];
	      i1 = grayin[1];
	      gray[0] = lut->composite[grayin[0] * grayin[1] / 255];
	    }
	}
      if (use_previous)
	{
	  gray[0] = o0;
	}
      else
	{
	  if (density != 1.0)
	    gray[0] = .5 + gray[0] * density;
	  o0 = gray[0];
	}
      grayin += bpp;
      gray ++;
      width --;
    }
}

static void
gray_to_monochrome(const unsigned char *grayin,
		   stp_color_output_t *out,
		   unsigned short *gray,
		   int    	width,
		   int    	bpp,
		   const unsigned char *cmap,
		   const stp_vars_t	vars,
		   const double *hue_map,
		   const double *lum_map,
		   const double *sat_map)
{
  int i0 = -1;
  int i1 = -1;
  int use_previous = 0;
  int o0 = 0;
  lut_t *lut = (lut_t *)(stp_get_lut(vars));
  unsigned short *gray = out->k;
  while (width > 0)
    {
      if (bpp == 1)
	{
	  /*
	   * No alpha in image...
	   */
	  if (i0 == grayin[0])
	    use_previous = 1;
	  else
	    {
	      use_previous = 0;
	      i0 = grayin[0];
	      gray[0] = lut->composite[grayin[0]];
	    }
	}
      else
	{
	  if (i0 == grayin[0] && i1 == grayin[1])
	    use_previous = 1;
	  else
	    {
	      use_previous = 0;
	      i0 = grayin[0];
	      i1 = grayin[1];
	      gray[0] = lut->composite[grayin[0] * grayin[1] / 255];
	    }
	}
      if (use_previous)
	{
	  gray[0] = o0;
	}
      else
	{
	  if (gray[0] < 32768)
	    gray[0] = 0;
	  else
	    gray[0] = 65535;
	  o0 = gray[0];
	}
      grayin += bpp;
      gray ++;
      width --;
    }
}

/*
 * 'indexed_to_gray()' - Convert indexed image data to grayscale.
 */

static void
indexed_to_gray(const unsigned char *indexed,
		stp_color_output_t *out,
		int    width,
		int    bpp,
		const unsigned char *cmap,
		const stp_vars_t   vars,
		const double *hue_map,
		const double *lum_map,
		const double *sat_map)
{
  int i0 = -1;
  int i1 = -1;
  int o0 = 0;
  int use_previous = 0;
  int i;
  lut_t *lut = (lut_t *)(stp_get_lut(vars));
  unsigned char	gray_cmap[256];		/* Grayscale colormap */
  unsigned short *gray = out->k;
  double density = stp_get_density(vars);

  /* Really should precompute this silly thing... */
  for (i = 0; i < 256; i ++, cmap += 3)
    gray_cmap[i] = (cmap[0] * LUM_RED +
		    cmap[1] * LUM_GREEN +
		    cmap[2] * LUM_BLUE) / 100;

  while (width > 0)
    {
      if (bpp == 1)
	{
	  /*
	   * No alpha in image...
	   */
	  if (i0 == indexed[0])
	    use_previous = 1;
	  else
	    {
	      use_previous = 0;
	      i0 = indexed[0];
	      gray[0] = lut->composite[gray_cmap[i0]];
	    }
	}
      else
	{
	  if (i0 == indexed[0] && i1 == indexed[1])
	    use_previous = 1;
	  else
	    {
	      use_previous = 0;
	      i0 = indexed[0];
	      i1 = indexed[1];
	      gray[0] = lut->composite[gray_cmap[i0] * i1 / 255];
	    }
	}
      if (use_previous)
	{
	  gray[0] = o0;
	}
      else
	{
	  if (density != 1.0)
	    gray[0] = .5 + gray[0] * density;
	  o0 = gray[0];
	}
      indexed += bpp;
      gray ++;
      width --;
    }
}

static void
indexed_to_monochrome(const unsigned char *indexed,
		      stp_color_output_t *out,
		      unsigned short *gray,
		      int    width,
		      int    bpp,
		      const unsigned char *cmap,
		      const stp_vars_t   vars,
		      const double *hue_map,
		      const double *lum_map,
		      const double *sat_map)
{
  int i0 = -1;
  int i1 = -1;
  int o0 = 0;
  int use_previous = 0;
  int i;
  lut_t *lut = (lut_t *)(stp_get_lut(vars));
  unsigned char	gray_cmap[256];		/* Grayscale colormap */
  unsigned short *gray = out->k;

  /* Really should precompute this silly thing... */
  for (i = 0; i < 256; i ++, cmap += 3)
    gray_cmap[i] = (cmap[0] * LUM_RED +
		    cmap[1] * LUM_GREEN +
		    cmap[2] * LUM_BLUE) / 100;

  while (width > 0)
    {
      if (bpp == 1)
	{
	  /*
	   * No alpha in image...
	   */
	  if (i0 == indexed[0])
	    use_previous = 1;
	  else
	    {
	      use_previous = 0;
	      i0 = indexed[0];
	      gray[0] = lut->composite[gray_cmap[i0]];
	    }
	}
      else
	{
	  if (i0 == indexed[0] && i1 == indexed[1])
	    use_previous = 1;
	  else
	    {
	      use_previous = 0;
	      i0 = indexed[0];
	      i1 = indexed[1];
	      gray[0] = lut->composite[gray_cmap[i0] * i1 / 255];
	    }
	}
      if (use_previous)
	{
	  gray[0] = o0;
	}
      else
	{
	  if (gray[0] < 32768)
	    gray[0] = 0;
	  else
	    gray[0] = 65535;
	  o0 = gray[0];
	}
      indexed += bpp;
      gray ++;
      width --;
    }
}

/*
 * 'rgb_to_gray()' - Convert RGB image data to grayscale.
 */

static void
rgb_to_gray(const unsigned char *rgb,
	    stp_color_output_t *out,
	    int    width,
	    int    bpp,
	    const unsigned char *cmap,
	    const stp_vars_t   vars,
	    const double *hue_map,
	    const double *lum_map,
	    const double *sat_map)
{
  int i0 = -1;
  int i1 = -1;
  int i2 = -1;
  int i3 = -1;
  int o0 = 0;
  int use_previous = 0;
  lut_t *lut = (lut_t *)(stp_get_lut(vars));
  unsigned short *gray = out->k;
  double density = stp_get_density(vars);
  while (width > 0)
    {
      if (bpp == 3)
	{
	  /*
	   * No alpha in image...
	   */
	  if (i0 == rgb[0] && i1 == rgb[1] && i2 == rgb[2])
	    use_previous = 1;
	  else
	    {
	      use_previous = 0;
	      i0 = rgb[0];
	      i1 = rgb[1];
	      i2 = rgb[2];
	      gray[0] = lut->composite[(rgb[0] * LUM_RED +
					rgb[1] * LUM_GREEN +
					rgb[2] * LUM_BLUE) / 100];
	    }
	}
      else
	{
	  if (i0 == rgb[0] && i1 == rgb[1] && i2 == rgb[2] && i3 == rgb[3])
	    use_previous = 1;
	  else
	    {
	      use_previous = 0;
	      i0 = rgb[0];
	      i1 = rgb[1];
	      i2 = rgb[2];
	      i3 = rgb[3];

	      gray[0] = lut->composite[((rgb[0] * LUM_RED +
					 rgb[1] * LUM_GREEN +
					 rgb[2] * LUM_BLUE) *
					rgb[3] / 25500)];
	    }
	}
      if (use_previous)
	{
	  gray[0] = o0;
	}
      else
	{
	  if (density != 1.0)
	    gray[0] = .5 + gray[0] * density;
	  o0 = gray[0];
	}
      rgb += bpp;
      gray ++;
      width --;
    }
}

static void
rgb_to_monochrome(const unsigned char *rgb,
		  stp_color_output_t *out,
		  int    width,
		  int    bpp,
		  const unsigned char *cmap,
		  const stp_vars_t   vars,
		  const double *hue_map,
		  const double *lum_map,
		  const double *sat_map)
{
  int i0 = -1;
  int i1 = -1;
  int i2 = -1;
  int i3 = -1;
  int o0 = 0;
  int use_previous = 0;
  lut_t *lut = (lut_t *)(stp_get_lut(vars));
  unsigned short *gray = out->k;
  while (width > 0)
    {
      if (bpp == 3)
	{
	  /*
	   * No alpha in image...
	   */
	  if (i0 == rgb[0] && i1 == rgb[1] && i2 == rgb[2])
	    use_previous = 1;
	  else
	    {
	      use_previous = 0;
	      i0 = rgb[0];
	      i1 = rgb[1];
	      i2 = rgb[2];
	      gray[0] = lut->composite[(rgb[0] * LUM_RED +
					rgb[1] * LUM_GREEN +
					rgb[2] * LUM_BLUE) / 100];
	    }
	}
      else
	{
	  if (i0 == rgb[0] && i1 == rgb[1] && i2 == rgb[2] && i3 == rgb[3])
	    use_previous = 1;
	  else
	    {
	      use_previous = 0;
	      i0 = rgb[0];
	      i1 = rgb[1];
	      i2 = rgb[2];
	      i3 = rgb[3];

	      gray[0] = lut->composite[((rgb[0] * LUM_RED +
					 rgb[1] * LUM_GREEN +
					 rgb[2] * LUM_BLUE) *
					rgb[3] / 25500)];
	    }
	}
      if (use_previous)
	{
	  gray[0] = o0;
	}
      else
	{
	  if (gray[0] < 32768)
	    gray[0] = 0;
	  else
	    gray[0] = 65535;
	  o0 = gray[0];
	}
      rgb += bpp;
      gray ++;
      width --;
    }
}

/*
 * 'rgb_to_cmy()' - Convert rgb image data to RGB.
 */

static void
rgb_to_cmy(const unsigned char	*rgbin,
	   stp_color_output_t *out,
	   int    		width,
	   int    		bpp,
	   const unsigned char 	*cmap,
	   const stp_vars_t  	vars,
	   const double *hue_map,
	   const double *lum_map,
	   const double *sat_map)
{
  double density = stp_get_density(vars);
  unsigned ld = density * 65536;
  double isat = 1.0;
  double ssat = stp_get_saturation(vars);
  int i0 = -1;
  int i1 = -1;
  int i2 = -1;
  int i3 = -1;
  int o0 = 0;
  int o1 = 0;
  int o2 = 0;
  int use_previous = 0;
  lut_t *lut = (lut_t *)(stp_get_lut(vars));
  int compute_saturation = ssat <= .99999 || ssat >= 1.00001;
  int split_saturation = ssat > 1.4;
  int i = 0;
  if (split_saturation)
    ssat = sqrt(ssat);
  if (ssat > 1)
    isat = 1.0 / ssat;
  while (width > 0)
    {
      double h, s, v;
      switch (bpp)
	{
	case 1:
	  /*
	   * No alpha in image, using colormap...
	   */
	  if (i0 == rgbin[0])
	    use_previous = 1;
	  else
	    {
	      int idx = rgbin[0] * 3;
	      use_previous = 0;
	      i0 = rgbin[0];
	      out->c[i] = 65535 - cmap[idx + 0] * 257;
	      out->m[i] = 65535 - cmap[idx + 1] * 257;
	      out->y[i] = 65535 - cmap[idx + 2] * 257;
	    }
	  break;
	case 2:
	  if (i0 == rgbin[0] && i1 == rgbin[1])
	    use_previous = 1;
	  else
	    {
	      int idx = rgbin[0] * 3;
	      use_previous = 0;
	      i0 = rgbin[0];
	      i1 = rgbin[1];
	      out->c[i] = 65535 - (cmap[idx + 0] * rgbin[1]) * 257 / 255;
	      out->m[i] = 65535 - (cmap[idx + 1] * rgbin[1]) * 257 / 255;
	      out->y[i] = 65535 - (cmap[idx + 2] * rgbin[1]) * 257 / 255;
	    }
	  break;
	case 3:
	  /*
	   * No alpha in image...
	   */
	  if (i0 == rgbin[0] && i1 == rgbin[1] && i2 == rgbin[2])
	    use_previous = 1;
	  else
	    {
	      use_previous = 0;
	      i0 = rgbin[0];
	      i1 = rgbin[1];
	      i2 = rgbin[2];
	      out->c[i] = rgbin[0] * 257;
	      out->m[i] = rgbin[1] * 257;
	      out->y[i] = rgbin[2] * 257;
	    }
	  break;
	case 4:
	  if (i0 == rgbin[0] && i1 == rgbin[1] && i2 == rgbin[2] &&
	      i3 == rgbin[3])
	    use_previous = 1;
	  else
	    {
	      use_previous = 0;
	      i0 = rgbin[0];
	      i1 = rgbin[1];
	      i2 = rgbin[2];
	      i3 = rgbin[3];
	      out->c[i] = (rgbin[0] * rgbin[3]) * 257 / 255;
	      out->m[i] = (rgbin[1] * rgbin[3]) * 257 / 255;
	      out->y[i] = (rgbin[2] * rgbin[3]) * 257 / 255;
	    }
	  break;
	}
      if (use_previous)
	{
	  out->c[i] = o0;
	  out->m[i] = o1;
	  out->y[i] = o2;
	}
      else
	{
	  if ((compute_saturation) &&
	      (out->c[i] != out->m[i] || out->c[i] != out->y[i]))
	    {
	      calc_cmy_to_hsl(out, i, &h, &s, &v);
	      if (ssat < 1)
		s *= ssat;
	      else
		{
		  double s1 = s * ssat;
		  double s2 = 1.0 - ((1.0 - s) * isat);
		  s = FMIN(s1, s2);
		}
	      if (s > 1)
		s = 1.0;
	      calc_hsl_to_cmy(out, i, h, s, v);
	    }
	  update_cmyk(out, i);	/* Fiddle with the INPUT */
	  out->c[i] = lookup_value(out->c[i], lut->steps,
				   lut->red, lut->shiftval,
				   lut->bin_size, lut->bin_shift);
	  out->m[i] = lookup_value(out->m[i], lut->steps,
				   lut->green, lut->shiftval,
				   lut->bin_size, lut->bin_shift);
	  out->y[i] = lookup_value(out->y[i], lut->steps,
				   lut->blue, lut->shiftval,
				   lut->bin_size, lut->bin_shift);
	  if ((split_saturation || hue_map || lum_map || sat_map) &&
	      (out->c[i] != out->m[i] || out->c[i] != out->y[i]))
	    {
	      calc_rgb_to_hsl(out, i, &h, &s, &v);
	      if (split_saturation)
		{
		  if (ssat < 1)
		    s *= ssat;
		  else
		    {
		      double s1 = s * ssat;
		      double s2 = 1.0 - ((1.0 - s) * isat);
		      s = FMIN(s1, s2);
		    }
		}
	      if (s > 1)
		s = 1.0;
	      if (hue_map || lum_map || sat_map)
		{
		  if (hue_map)
		    {
		      int ih;
		      double eh;
		      double nh = h * 8;
		      ih = (int) nh;
		      eh = nh - (double) ih;
		      h = hue_map[ih] + eh * (hue_map[ih + 1] - hue_map[ih]);
		      if (h < 0.0)
			h += 6.0;
		      else if (h >= 6.0)
			h -= 6.0;
		    }
		  if (lum_map && v > .0001 && v < .9999)
		    {
		      int ih;
		      double eh;
		      double nh = h * 8;
		      ih = (int) nh;
		      eh = nh - (double) ih;
		      if (lum_map[ih] != 1.0 || lum_map[ih + 1] != 1.0)
			{
			  double ev = lum_map[ih] +
			    eh * (lum_map[ih + 1] - lum_map[ih]);
			  ev = 1.0 + (s * (ev - 1.0));
			  if (v > .5)
			    ev = 1.0 + ((2.0 * (1.0 - v)) * (ev - 1.0));
			  v = 1.0 - pow(1.0 - v, ev);
			}
		    }
		  if (sat_map)
		    {
		      int ih;
		      double eh;
		      double nh = h * 8;
		      ih = (int) nh;
		      eh = nh - (double) ih;
		      if (sat_map[ih] != 1.0 || sat_map[ih + 1] != 1.0)
			{
			  double es = sat_map[ih] +
			    eh * (sat_map[ih + 1] - sat_map[ih]);
			  s = 1.0 - pow(1.0 - s, es);
			}
		    }
		}
	      calc_hsl_to_cmy(out, i, h, s, v);
	    }
	  if (ld < 65536)
	    {
	      out->c[i] = .5 + out->c[i] * density;
	      out->m[i] = .5 + out->m[i] * density;
	      out->y[i] = .5 + out->y[i] * density;
	    }
	  o0 = out->c[i];
	  o1 = out->m[i];
	  o2 = out->y[i];
	}
      rgbin += bpp;
      i++;
      width --;
    }
}

static void
indexed_to_cmy(const unsigned char *indexed,
	       stp_color_output_t *out,
	       int    width,
	       int    bpp,
	       const unsigned char *cmap,
	       const stp_vars_t   vars,
	       const double *hue_map,
	       const double *lum_map,
	       const double *sat_map)
{
  rgb_to_cmy(indexed, rgb, width, bpp, cmap, vars, hue_map, lum_map, sat_map);
}

/*
 * 'gray_to_cmy()' - Convert gray image data to RGB.
 */

static void
gray_to_cmy(const unsigned char	*grayin,
	    stp_color_output_t *out,
	    int    		width,
	    int    		bpp,
	    const unsigned char *cmap,
	    const stp_vars_t  	vars,
	    const double *hue_map,
	    const double *lum_map,
	    const double *sat_map)
{
  double density = stp_get_density(vars);
  int use_previous = 0;
  int i0 = -1;
  int i1 = -1;
  int o0 = 0;
  int o1 = 0;
  int o2 = 0;
  lut_t *lut = (lut_t *)(stp_get_lut(vars));
  int i = 0;
  while (width > 0)
    {
      unsigned short trgb[3];
      if (bpp == 1)
	{
	  /*
	   * No alpha in image...
	   */
	  if (i0 == grayin[0])
	    use_previous = 1;
	  else
	    {
	      use_previous = 0;
	      i0 = grayin[0];
	      trgb[0] = grayin[0] * 257;
	      trgb[1] = grayin[0] * 257;
	      trgb[2] = grayin[0] * 257;
	    }
	}
      else
	{
	  if (i0 == grayin[0] && i1 == grayin[1])
	    use_previous = 1;
	  else
	    {
	      int lookup = grayin[0] * grayin[1] * 257 / 255;
	      use_previous = 0;
	      i0 = grayin[0];
	      i1 = grayin[1];
	      trgb[0] = lookup;
	      trgb[1] = lookup;
	      trgb[2] = lookup;
	    }
	}
      if (use_previous)
	{
	  out->c[i] = o0;
	  out->m[i] = o1;
	  out->y[i] = o2;
	}
      else
	{
	  update_cmyk(trgb);
	  out->c[i] = lookup_value(trgb[0], lut->steps,
				   lut->red, lut->shiftval,
				   lut->bin_size, lut->bin_shift);
	  out->m[i] = lookup_value(trgb[1], lut->steps,
				   lut->green, lut->shiftval,
				   lut->bin_size, lut->bin_shift);
	  out->y[i] = lookup_value(trgb[2], lut->steps,
				   lut->blue, lut->shiftval,
				   lut->bin_size, lut->bin_shift);
	  if (density != 1.0)
	    {
	      out->c[i] = .5 + out->c[i] * density;
	      out->m[i] = .5 + out->m[i] * density;
	      out->y[i] = .5 + out->y[i] * density;
	    }
	  o0 = out->c[i];
	  o1 = out->m[i];
	  o2 = out->y[i];
	}
      grayin += bpp;
      i++;
      width --;
    }
}

static void
fast_indexed_to_cmy(const unsigned char *indexed,
		    stp_color_output_t *out,
		    int    width,
		    int    bpp,
		    const unsigned char *cmap,
		    const stp_vars_t   vars,
		    const double *hue_map,
		    const double *lum_map,
		    const double *sat_map)
{
  double density = stp_get_density(vars);
  int i0 = -1;
  int i1 = -1;
  int o0 = 0;
  int o1 = 0;
  int o2 = 0;
  lut_t *lut = stp_get_lut(vars);
  int use_previous = 0;
  double isat = 1.0;
  double saturation = stp_get_saturation(vars);
  if (saturation > 1)
    isat = 1.0 / saturation;
  int i = 0;
  while (width > 0)
    {
      double h, s, v;
      if (bpp == 1)
	{
	  /*
	   * No alpha in image...
	   */
	  if (i0 == indexed[0])
	    use_previous = 1;
	  else
	    {
	      use_previous = 0;
	      i0 = indexed[0];
	      out->c[i] = lut->red[cmap[i0 * 3 + 0]];
	      out->m[i] = lut->green[cmap[i0 * 3 + 1]];
	      out->y[i] = lut->blue[cmap[i0 * 3 + 2]];
	    }
	}
      else
	{
	  if (i0 == indexed[0] && i1 == indexed[1])
	    use_previous = 1;
	  else
	    {
	      use_previous = 0;
	      i0 = indexed[0];
	      i1 = indexed[1];
	      out->c[i] = lut->red[cmap[i0 * 3 + 0] * i1 / 255];
	      out->m[i] = lut->green[cmap[i0 * 3 + 1] * i1 / 255];
	      out->y[i] = lut->blue[cmap[i0 * 3 + 2] * i1 / 255];
	    }
	}
      if (use_previous)
	{
	  out->c[i] = o0;
	  out->m[i] = o1;
	  out->y[i] = o2;
	}
      else
	{
	  if (saturation != 1.0)
	    {
	      calc_rgb_to_hsl(out, i, &h, &s, &v);
	      if (saturation < 1)
		s *= saturation;
	      else if (saturation > 1)
		{
		  double s1 = s * saturation;
		  double s2 = 1.0 - ((1.0 - s) * isat);
		  s = FMIN(s1, s2);
		}
	      if (s > 1)
		s = 1.0;
	      calc_hsl_to_cmy(out, i, h, s, v);
	    }
	  if (density != 1.0)
	    {
	      out->c[i] = .5 + out->c[i] * density;
	      out->m[i] = .5 + out->m[i] * density;
	      out->y[i] = .5 + out->y[i] * density;
	    }
	  o0 = out->c[i];
	  o1 = out->m[i];
	  o2 = out->y[i];
	}
      indexed += bpp;
      i++;
      width --;
    }
}

/*
 * 'rgb_to_cmy()' - Convert rgb image data to RGB.
 */

static void
fast_rgb_to_cmy(const unsigned char *rgbin,
		stp_color_output_t *out,
		int width,
		int bpp,
		const unsigned char *cmap,
		const stp_vars_t vars,
		const double *hue_map,
		const double *lum_map,
		const double *sat_map)
{
  double density = stp_get_density(vars);
  unsigned ld = density * 65536;
  int i0 = -1;
  int i1 = -1;
  int i2 = -1;
  int i3 = -1;
  int o0 = 0;
  int o1 = 0;
  int o2 = 0;
  lut_t *lut = (lut_t *)(stp_get_lut(vars));
  int use_previous = 0;
  double isat = 1.0;
  double saturation = stp_get_saturation(vars);
  if (saturation > 1)
    isat = 1.0 / saturation;
  int i = 0;
  while (width > 0)
    {
      double h, s, v;
      if (bpp == 3)
	{
	  /*
	   * No alpha in image...
	   */
	  if (i0 == rgbin[0] && i1 == rgbin[1] && i2 == rgbin[2])
	    use_previous = 1;
	  else
	    {
	      use_previous = 0;
	      i0 = rgbin[0];
	      i1 = rgbin[1];
	      i2 = rgbin[2];
	      out->c[i] = lut->red[rgbin[0]];
	      out->m[i] = lut->green[rgbin[1]];
	      out->y[i] = lut->blue[rgbin[2]];
	    }
	}
      else
	{
	  if (i0 == rgbin[0] && i1 == rgbin[1] && i2 == rgbin[2] &&
	      i3 == rgbin[3])
	    use_previous = 1;
	  else
	    {
	      use_previous = 0;
	      i0 = rgbin[0];
	      i1 = rgbin[1];
	      i2 = rgbin[2];
	      i3 = rgbin[3];
	      out->c[i] = lut->red[i0 * i3 / 255];
	      out->m[i] = lut->green[i1 * i3 / 255];
	      out->y[i] = lut->blue[i2 * i3 / 255];
	    }
	}
      if (use_previous)
	{
	  out->c[i] = o0;
	  out->m[i] = o1;
	  out->y[i] = o2;
	}
      else
	{
	  if (saturation != 1.0)
	    {
	      calc_rgb_to_out(rgbout, &h, &s, &v);
	      if (saturation < 1)
		s *= saturation;
	      else if (saturation > 1)
		{
		  double s1 = s * saturation;
		  double s2 = 1.0 - ((1.0 - s) * isat);
		  s = FMIN(s1, s2);
		}
	      if (s > 1)
		s = 1.0;
	      calc_hsl_to_cmy(out, i, h, s, v);
	    }
	  if (density != 1.0)
	    {
	      out->c[i] = .5 + out->c[i] * density;
	      out->m[i] = .5 + out->m[i] * density;
	      out->y[i] = .5 + out->y[i] * density;
	    }
	  o0 = out->c[i];
	  o1 = out->m[i];
	  o2 = out->y[i];
	}
      rgbin += bpp;
      i++;
      width --;
    }
}

/*
 * 'gray_to_cmy()' - Convert gray image data to RGB.
 */

static void
fast_gray_to_cmy(unsigned char	*grayin,	/* I - grayscale pixels */
		 stp_color_output_t *out,
		 int    	width,		/* I - Width of row */
		 int    	bpp,		/* I - Bytes/pix in indexed */
		 const unsigned char 	*cmap,		/* I - Colormap */
		 const stp_vars_t  	vars,
		 const double *hue_map,
		 const double *lum_map,
		 const double *sat_map
		 )
{
  double density = stp_get_density(vars);
  int use_previous = 0;
  int i0 = -1;
  int i1 = -1;
  int o0 = 0;
  int o1 = 0;
  int o2 = 0;
  lut_t *lut = (lut_t *)(stp_get_lut(vars));
  int i = 0;
  while (width > 0)
    {
      if (bpp == 1)
	{
	  /*
	   * No alpha in image...
	   */
	  if (i0 == grayin[0])
	    use_previous = 1;
	  else
	    {
	      use_previous = 0;
	      i0 = grayin[0];
	      out->c[i] = lut->red[grayin[0]];
	      out->m[i] = lut->green[grayin[0]];
	      out->y[i] = lut->blue[grayin[0]];
	    }
	}
      else
	{
	  if (i0 == grayin[0] && i1 == grayin[1])
	    use_previous = 1;
	  else
	    {
	      int lookup = (grayin[0] * grayin[1] / 255);
	      use_previous = 0;
	      i0 = grayin[0];
	      i1 = grayin[1];
	      out->c[i] = lut->red[lookup];
	      out->m[i] = lut->green[lookup];
	      out->y[i] = lut->blue[lookup];
	    }
	}
      if (use_previous)
	{
	  out->c[i] = o0;
	  out->m[i] = o1;
	  out->y[i] = o2;
	}
      else
	{
	  if (density != 1.0)
	    {
	      out->c[i] = .5 + out->c[i] * density;
	      out->m[i] = .5 + out->m[i] * density;
	      out->y[i] = .5 + out->y[i] * density;
	    }
      	  o0 = out->c[i];
	  o1 = out->m[i];
	  o2 = out->y[i];
	}
      grayin += bpp;
      i++;
      width --;
    }
}

static lut_t *
allocate_lut(size_t steps)
{
  int i;
  lut_t *ret = stp_malloc(sizeof(lut_t));
  ret->steps = steps;
  ret->composite = stp_malloc(sizeof(unsigned short) * steps);
  ret->red = stp_malloc(sizeof(unsigned short) * steps);
  ret->green = stp_malloc(sizeof(unsigned short) * steps);
  ret->blue = stp_malloc(sizeof(unsigned short) * steps);
  ret->shiftval = 0;
  for (i = 1; i < steps; i += i)
    ret->shiftval++;
  ret->bin_size = 65536 / steps;
  ret->bin_shift = 16 - ret->shiftval;
  return ret;
}

void
stp_free_lut(stp_vars_t v)
{
  if (stp_get_lut(v))
    {
      lut_t *lut = (lut_t *)(stp_get_lut(v));
      if (lut->composite)
	free(lut->composite);
      if (lut->red)
	free(lut->red);
      if (lut->green)
	free(lut->green);
      if (lut->blue)
	free(lut->blue);
      lut->steps = 0;
      lut->composite = NULL;
      lut->red = NULL;
      lut->green = NULL;
      lut->blue = NULL;
      free(stp_get_lut(v));
      stp_set_lut(v, NULL);
    }
}

/* #define PRINT_LUT */

void
stp_compute_lut(size_t steps, stp_vars_t v)
{
  double	pixel,		/* Pixel value */
		red_pixel,	/* Pixel value */
		green_pixel,	/* Pixel value */
		blue_pixel;	/* Pixel value */
  int i;
#ifdef PRINT_LUT
  FILE *ltfile = fopen("/mnt1/lut", "w");
#endif
  /*
   * Got an output file/command, now compute a brightness lookup table...
   */

  double cyan = stp_get_cyan(v);
  double magenta = stp_get_magenta(v);
  double yellow = stp_get_yellow(v);
  double print_gamma = stp_get_gamma(v);
  double contrast = stp_get_contrast(v);
  double app_gamma = stp_get_app_gamma(v);
  double brightness = stp_get_brightness(v);
  double screen_gamma = app_gamma / 1.7;	/* Why 1.7??? */
  lut_t *lut;

  /*
   * Monochrome mode simply thresholds the input
   * to decide whether to print at all.  The printer gamma
   * is intended to represent the analog response of the printer.
   * Using it shifts the threshold, which is not the intent
   * of how this works.
   */
  if (stp_get_image_type(v) == IMAGE_MONOCHROME)
    print_gamma = 1.0;

  lut = allocate_lut(steps);
  stp_set_lut(v, lut);
  for (i = 0; i < steps; i ++)
    {
      double temp_pixel;
      pixel = (double) i / (double) (steps - 1);

      /*
       * First, correct contrast
       */
      if (pixel >= .5)
	temp_pixel = 1.0 - pixel;
      else
	temp_pixel = pixel;
      if (temp_pixel <= .000001 && contrast <= .0001)
	temp_pixel = .5;
      else if (temp_pixel > 1)
	temp_pixel = .5 * pow(2 * temp_pixel, pow(contrast, contrast));
      else if (temp_pixel < 1)
	temp_pixel = 0.5 -
	  ((0.5 - .5 * pow(2 * temp_pixel, contrast)) * contrast);
      if (temp_pixel > .5)
	temp_pixel = .5;
      else if (temp_pixel < 0)
	temp_pixel = 0;
      if (pixel < .5)
	pixel = temp_pixel;
      else
	pixel = 1 - temp_pixel;

      /*
       * Second, do brightness
       */
      if (brightness < 1)
	pixel = pixel * brightness;
      else
	pixel = 1 - ((1 - pixel) * (2 - brightness));

      /*
       * Third, correct for the screen gamma
       */
      pixel = 1.0 - pow(pixel, screen_gamma);

      /*
       * Third, fix up cyan, magenta, yellow values
       */
      if (pixel < 0.0)
	pixel = 0.0;
      else if (pixel > 1.0)
	pixel = 1.0;

      if (pixel > .9999 && cyan < .00001)
	red_pixel = 0;
      else
	red_pixel = 1 - pow(1 - pixel, cyan);
      if (pixel > .9999 && magenta < .00001)
	green_pixel = 0;
      else
	green_pixel = 1 - pow(1 - pixel, magenta);
      if (pixel > .9999 && yellow < .00001)
	blue_pixel = 0;
      else
	blue_pixel = 1 - pow(1 - pixel, yellow);

      /*
       * Finally, fix up print gamma and scale
       */

      pixel = 65535 * (1 - pow(pixel, print_gamma)) + .5;
      red_pixel = 65535 * (1 - pow(red_pixel, print_gamma)) + .5;
      green_pixel = 65535 * (1 - pow(green_pixel, print_gamma)) + .5;
      blue_pixel = 65535 * (1 - pow(blue_pixel, print_gamma)) + .5;

      if (pixel <= 0.0)
	lut->composite[i] = 0;
      else if (pixel >= 65535.0)
	lut->composite[i] = 65535;
      else
	lut->composite[i] = (unsigned)(pixel);

      if (red_pixel <= 0.0)
	lut->red[i] = 0;
      else if (red_pixel >= 65535.0)
	lut->red[i] = 65535;
      else
	lut->red[i] = (unsigned)(red_pixel);

      if (green_pixel <= 0.0)
	lut->green[i] = 0;
      else if (green_pixel >= 65535.0)
	lut->green[i] = 65535;
      else
	lut->green[i] = (unsigned)(green_pixel);

      if (blue_pixel <= 0.0)
	lut->blue[i] = 0;
      else if (blue_pixel >= 65535.0)
	lut->blue[i] = 65535;
      else
	lut->blue[i] = (unsigned)(blue_pixel);

      lut->composite[i] = 65535 - lut->composite[i];
      lut->red[i] = 65535 - lut->red[i];
      lut->green[i] = 65535 - lut->green[i];
      lut->blue[i] = 65535 - lut->blue[i];

#ifdef PRINT_LUT
      fprintf(ltfile, "%3i  %5d  %5d  %5d  %5d  %f %f %f %f  %f %f %f  %f\n",
	      i, lut->composite[i], lut->red[i],
	      lut->green[i], lut->blue[i], pixel, red_pixel,
	      green_pixel, blue_pixel, print_gamma, screen_gamma,
	      print_gamma, app_gamma);
#endif
    }

#ifdef PRINT_LUT
  fclose(ltfile);
#endif
}

stp_convert_t
stp_choose_colorfunc(int output_type,
		 int image_bpp,
		 const unsigned char *cmap,
		 int *out_bpp,
		 const stp_vars_t v)
{
  if (stp_get_image_type(v) == IMAGE_MONOCHROME)
    {
      *out_bpp = 1;

      if (image_bpp >= 3)
	return rgb_to_monochrome;
      else if (cmap == NULL)
	return gray_to_monochrome;
      else
	return indexed_to_monochrome;
    }
  else if (output_type == OUTPUT_COLOR)
    {
      *out_bpp = 3;

      if (image_bpp >= 3)
	{
	  if (stp_get_image_type(v) == IMAGE_CONTINUOUS)
	    return rgb_to_cmy;
	  else
	    return fast_rgb_to_cmy;
	}
      else if (cmap == NULL)
        {
          if (stp_get_image_type(v) == IMAGE_CONTINUOUS)
	    return gray_to_cmy;
          else
	    return fast_gray_to_cmy;
        }
      else
	{
	  if (stp_get_image_type(v) == IMAGE_CONTINUOUS)
	    return indexed_to_cmy;
	  else
	    return fast_indexed_to_cmy;
	}
    }
  else
    {
      *out_bpp = 1;

      if (image_bpp >= 3)
	return rgb_to_gray;
      else if (cmap == NULL)
	return gray_to_gray;
      else
	return indexed_to_gray;
    }
}
