/*
 * "$Id: print-util.c,v 1.72.2.1 2002/10/21 01:15:31 rlk Exp $"
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

/*
 * This file must include only standard C header files.  The core code must
 * compile on generic platforms that don't support glib, gimp, gtk, etc.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <gimp-print/gimp-print.h>
#include "gimp-print-internal.h"
#include <gimp-print/gimp-print-intl-internal.h>
#include <math.h>
#include <limits.h>
#if defined(HAVE_VARARGS_H) && !defined(HAVE_STDARG_H)
#include <varargs.h>
#else
#include <stdarg.h>
#endif
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define FMIN(a, b) ((a) < (b) ? (a) : (b))

static size_t
c_strlen(const char *s)
{
  return strlen(s);
}

static char *
c_strndup(const char *s, int n)
{
  char *ret;
  if (!s || n < 0)
    {
      ret = stp_malloc(1);
      ret[0] = 0;
      return ret;
    }
  else
    {
      ret = stp_malloc(n + 1);
      memcpy(ret, s, n);
      ret[n] = 0;
      return ret;
    }
}

static char *
c_strdup(const char *s)
{
  char *ret;
  if (!s)
    {
      ret = stp_malloc(1);
      ret[0] = 0;
      return ret;
    }
  else
    return c_strndup(s, c_strlen(s));
}

void
stp_compute_page_parameters(const stp_printer_t printer,
			    const stp_vars_t v,
			    stp_image_t *image, /* IO */
			    int *page_width, /* O */
			    int *page_height, /* O */
			    int *out_width,	/* O */
			    int *out_height, /* O */
			    int *left, /* O */
			    int *top) /* O */
{
  int page_right;
  int page_left;
  int page_top;
  int page_bottom;
  int image_width = image->width(image);
  int image_height = image->height(image);
  double scaling = stp_get_scaling(v);
  int orientation = stp_get_orientation(v);
  const stp_printfuncs_t *printfuncs = stp_printer_get_printfuncs(printer);

  (printfuncs->imageable_area)(printer, v, &page_left, &page_right,
			       &page_bottom, &page_top);
  *page_width  = page_right - page_left;
  *page_height = page_top - page_bottom;

  /* In AUTO orientation, just orient the paper the same way as the image. */

  if (orientation == ORIENT_AUTO)
    {
      if ((*page_width >= *page_height && image_width >= image_height)
         || (*page_height >= *page_width && image_height >= image_width))
        orientation = ORIENT_PORTRAIT;
      else
        orientation = ORIENT_LANDSCAPE;
    }

  if (orientation == ORIENT_LANDSCAPE)
      image->rotate_ccw(image);
  else if (orientation == ORIENT_UPSIDEDOWN)
      image->rotate_180(image);
  else if (orientation == ORIENT_SEASCAPE)
      image->rotate_cw(image);

  image_width  = image->width(image);
  image_height = image->height(image);

  /*
   * Calculate width/height...
   */

  if (scaling == 0.0)
    {
      *out_width  = *page_width;
      *out_height = *page_height;
    }
  else if (scaling < 0.0)
    {
      /*
       * Scale to pixels per inch...
       */

      *out_width  = image_width * -72.0 / scaling;
      *out_height = image_height * -72.0 / scaling;
    }
  else
    {
      /*
       * Scale by percent...
       */

      /*
       * Decide which orientation gives the proper fit
       * If we ask for 50%, we do not want to exceed that
       * in either dimension!
       */

      int twidth0 = *page_width * scaling / 100.0;
      int theight0 = twidth0 * image_height / image_width;
      int theight1 = *page_height * scaling / 100.0;
      int twidth1 = theight1 * image_width / image_height;

      *out_width = FMIN(twidth0, twidth1);
      *out_height = FMIN(theight0, theight1);
    }

  if (*out_width == 0)
    *out_width = 1;
  if (*out_height == 0)
    *out_height = 1;

  /*
   * Adjust offsets depending on the page orientation...
   */

  if (orientation == ORIENT_LANDSCAPE || orientation == ORIENT_SEASCAPE)
    {
      int x;

      x     = *left;
      *left = *top;
      *top  = x;
    }

  if ((orientation == ORIENT_UPSIDEDOWN || orientation == ORIENT_SEASCAPE)
      && *left >= 0)
    {
      *left = *page_width - *left - *out_width;
      if (*left < 0)
	*left = 0;
    }

  if ((orientation == ORIENT_UPSIDEDOWN || orientation == ORIENT_LANDSCAPE)
      && *top >= 0)
    {
      *top = *page_height - *top - *out_height;
      if (*top < 0)
	*top = 0;
    }

  if (*left < 0)
    *left = (*page_width - *out_width) / 2;

  if (*top < 0)
    *top  = (*page_height - *out_height) / 2;
}

void
stp_set_printer_defaults(stp_vars_t v, const stp_printer_t p,
			 const char *ppd_file)
{
  const stp_printfuncs_t *printfuncs = stp_printer_get_printfuncs(p);
  stp_set_resolution(v, ((printfuncs->default_parameters)
			 (p, ppd_file, "Resolution")));
  stp_set_ink_type(v, ((printfuncs->default_parameters)
		       (p, ppd_file, "InkType")));
  stp_set_media_type(v, ((printfuncs->default_parameters)
			 (p, ppd_file, "MediaType")));
  stp_set_media_source(v, ((printfuncs->default_parameters)
			   (p, ppd_file, "InputSlot")));
  stp_set_media_size(v, ((printfuncs->default_parameters)
			 (p, ppd_file, "PageSize")));
  stp_set_dither_algorithm(v, stp_default_dither_algorithm());
  stp_set_driver(v, stp_printer_get_driver(p));
}

static int
verify_param(const char *checkval, stp_param_t *vptr,
	     int count, const char *what, const stp_vars_t v)
{
  int answer = 0;
  int i;
  if (count > 0)
    {
      for (i = 0; i < count; i++)
	if (!strcmp(checkval, vptr[i].name))
	  {
	    answer = 1;
	    break;
	  }
      if (!answer)
	stp_eprintf(v, _("%s is not a valid parameter of type %s\n"),
		    checkval, what);
      for (i = 0; i < count; i++)
	{
	  stp_free((void *)vptr[i].name);
	  stp_free((void *)vptr[i].text);
	}
    }
  else
    stp_eprintf(v, _("%s is not a valid parameter of type %s\n"),
		checkval, what);
  if (vptr)
    free(vptr);
  return answer;
}

#define CHECK_FLOAT_RANGE(v, component)					\
do									\
{									\
  const stp_vars_t max = stp_maximum_settings();			\
  const stp_vars_t min = stp_minimum_settings();			\
  if (stp_get_##component((v)) < stp_get_##component(min) ||		\
      stp_get_##component((v)) > stp_get_##component(max))		\
    {									\
      answer = 0;							\
      stp_eprintf(v, _("%s out of range (value %f, min %f, max %f)\n"),	\
		  #component, stp_get_##component(v),			\
		  stp_get_##component(min), stp_get_##component(max));	\
    }									\
} while (0)

#define CHECK_INT_RANGE(v, component)					\
do									\
{									\
  const stp_vars_t max = stp_maximum_settings();			\
  const stp_vars_t min = stp_minimum_settings();			\
  if (stp_get_##component((v)) < stp_get_##component(min) ||		\
      stp_get_##component((v)) > stp_get_##component(max))		\
    {									\
      answer = 0;							\
      stp_eprintf(v, _("%s out of range (value %d, min %d, max %d)\n"),	\
		  #component, stp_get_##component(v),			\
		  stp_get_##component(min), stp_get_##component(max));	\
    }									\
} while (0)

int
stp_verify_printer_params(const stp_printer_t p, const stp_vars_t v)
{
  stp_param_t *vptr;
  int count;
  int i;
  int answer = 1;
  const stp_printfuncs_t *printfuncs = stp_printer_get_printfuncs(p);
  const stp_vars_t printvars = stp_printer_get_printvars(p);
  const char *ppd_file = stp_get_ppd_file(v);

  /*
   * Note that in raw CMYK mode the user is responsible for not sending
   * color output to black & white printers!
   */
  if (stp_get_output_type(printvars) == OUTPUT_GRAY &&
      (stp_get_output_type(v) == OUTPUT_COLOR ||
       stp_get_output_type(v) == OUTPUT_RAW_CMYK))
    {
      answer = 0;
      stp_eprintf(v, _("Printer does not support color output\n"));
    }
  if (c_strlen(stp_get_media_size(v)) > 0)
    {
      const char *checkval = stp_get_media_size(v);
      vptr = (*printfuncs->parameters)(p, ppd_file, "PageSize", &count);
      answer &= verify_param(checkval, vptr, count, "page size", v);
    }
  else
    {
      int height, width;
      int min_height, min_width;
      (*printfuncs->limit)(p, v, &width, &height, &min_width, &min_height);
      if (stp_get_page_height(v) <= min_height ||
	  stp_get_page_height(v) > height ||
	  stp_get_page_width(v) <= min_width || stp_get_page_width(v) > width)
	{
	  answer = 0;
	  stp_eprintf(v, _("Image size is not valid\n"));
	}
    }

  if (stp_get_top(v) < 0)
    {
      answer = 0;
      stp_eprintf(v, _("Top margin must not be less than zero\n"));
    }

  if (stp_get_left(v) < 0)
    {
      answer = 0;
      stp_eprintf(v, _("Left margin must not be less than zero\n"));
    }

  CHECK_FLOAT_RANGE(v, gamma);
  CHECK_FLOAT_RANGE(v, contrast);
  CHECK_FLOAT_RANGE(v, cyan);
  CHECK_FLOAT_RANGE(v, magenta);
  CHECK_FLOAT_RANGE(v, yellow);
  CHECK_FLOAT_RANGE(v, brightness);
  CHECK_FLOAT_RANGE(v, density);
  CHECK_FLOAT_RANGE(v, saturation);
  if (stp_get_scaling(v) > 0)
    {
      CHECK_FLOAT_RANGE(v, scaling);
    }

  CHECK_INT_RANGE(v, image_type);
  CHECK_INT_RANGE(v, unit);
  CHECK_INT_RANGE(v, output_type);
  CHECK_INT_RANGE(v, input_color_model);
  CHECK_INT_RANGE(v, output_color_model);

  if (c_strlen(stp_get_media_type(v)) > 0)
    {
      const char *checkval = stp_get_media_type(v);
      vptr = (*printfuncs->parameters)(p, ppd_file, "MediaType", &count);
      answer &= verify_param(checkval, vptr, count, "media type", v);
    }

  if (c_strlen(stp_get_media_source(v)) > 0)
    {
      const char *checkval = stp_get_media_source(v);
      vptr = (*printfuncs->parameters)(p, ppd_file, "InputSlot", &count);
      answer &= verify_param(checkval, vptr, count, "media source", v);
    }

  if (c_strlen(stp_get_resolution(v)) > 0)
    {
      const char *checkval = stp_get_resolution(v);
      vptr = (*printfuncs->parameters)(p, ppd_file, "Resolution", &count);
      answer &= verify_param(checkval, vptr, count, "resolution", v);
    }

  if (c_strlen(stp_get_ink_type(v)) > 0)
    {
      const char *checkval = stp_get_ink_type(v);
      vptr = (*printfuncs->parameters)(p, ppd_file, "InkType", &count);
      answer &= verify_param(checkval, vptr, count, "ink type", v);
    }

  for (i = 0; i < stp_dither_algorithm_count(); i++)
    if (!strcmp(stp_get_dither_algorithm(v), stp_dither_algorithm_name(i)))
      {
	stp_set_verified(v, answer);
	return answer;
      }

  stp_eprintf(v, _("%s is not a valid dither algorithm\n"),
	      stp_get_dither_algorithm(v));
  stp_set_verified(v, 0);
  return 0;
}

/*
 * We cannot avoid use of the (non-ANSI) vsnprintf here; ANSI does
 * not provide a safe, length-limited sprintf function.
 */

#define STP_VASPRINTF(result, bytes, format)				\
{									\
  int current_allocation = 64;						\
  result = stp_malloc(current_allocation);				\
  while (1)								\
    {									\
      va_list args;							\
      va_start(args, format);						\
      bytes = vsnprintf(result, current_allocation, format, args);	\
      va_end(args);							\
      if (bytes >= 0 && bytes < current_allocation)			\
	break;								\
      else								\
	{								\
	  free (result);						\
	  if (bytes < 0)						\
	    current_allocation *= 2;					\
	  else								\
	    current_allocation = bytes + 1;				\
	  result = stp_malloc(current_allocation);			\
	}								\
    }									\
}

void
stp_zprintf(const stp_vars_t v, const char *format, ...)
{
  char *result;
  int bytes;
  STP_VASPRINTF(result, bytes, format);
  (stp_get_outfunc(v))((void *)(stp_get_outdata(v)), result, bytes);
  free(result);
}

void
stp_zfwrite(const char *buf, size_t bytes, size_t nitems, const stp_vars_t v)
{
  (stp_get_outfunc(v))((void *)(stp_get_outdata(v)), buf, bytes * nitems);
}

void
stp_putc(int ch, const stp_vars_t v)
{
  char a = (char) ch;
  (stp_get_outfunc(v))((void *)(stp_get_outdata(v)), &a, 1);
}

void
stp_puts(const char *s, const stp_vars_t v)
{
  (stp_get_outfunc(v))((void *)(stp_get_outdata(v)), s, c_strlen(s));
}

void
stp_eprintf(const stp_vars_t v, const char *format, ...)
{
  int bytes;
  if (stp_get_errfunc(v))
    {
      char *result;
      STP_VASPRINTF(result, bytes, format);
      (stp_get_errfunc(v))((void *)(stp_get_errdata(v)), result, bytes);
      free(result);
    }
}

void
stp_erputc(int ch)
{
  putc(ch, stderr);
}

void
stp_erprintf(const char *format, ...)
{
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
}

unsigned long stp_debug_level = 0;

static void
stp_init_debug(void)
{
  static int debug_initialized = 0;
  if (!debug_initialized)
    {
      const char *dval = getenv("STP_DEBUG");
      debug_initialized = 1;
      if (dval)
	{
	  stp_debug_level = strtoul(dval, 0, 0);
	  stp_erprintf("Gimp-Print %s %s\n", VERSION, RELEASE_DATE);
	}
    }
}

void
stp_dprintf(unsigned long level, const stp_vars_t v, const char *format, ...)
{
  int bytes;
  stp_init_debug();
  if ((level & stp_debug_level) && stp_get_errfunc(v))
    {
      char *result;
      STP_VASPRINTF(result, bytes, format);
      (stp_get_errfunc(v))((void *)(stp_get_errdata(v)), result, bytes);
      free(result);
    }
}

void
stp_deprintf(unsigned long level, const char *format, ...)
{
  va_list args;
  va_start(args, format);
  stp_init_debug();
  if (level & stp_debug_level)
    vfprintf(stderr, format, args);
  va_end(args);
}

void *
stp_malloc (size_t size)
{
  register void *memptr = NULL;

  if ((memptr = malloc (size)) == NULL)
    {
      fputs("Virtual memory exhausted.\n", stderr);
      exit (EXIT_FAILURE);
    }
  return (memptr);
}

void *
stp_zalloc (size_t size)
{
  register void *memptr = stp_malloc(size);
  (void) memset(memptr, 0, size);
  return (memptr);
}

void *
stp_realloc (void *ptr, size_t size)
{
  register void *memptr = NULL;

  if (size > 0 && ((memptr = realloc (ptr, size)) == NULL))
    {
      fputs("Virtual memory exhausted.\n", stderr);
      exit (EXIT_FAILURE);
    }
  return (memptr);
}

void
stp_free(void *ptr)
{
  free(ptr);
}

int
stp_init(void)
{
  static int stp_is_initialised = 0;
  if (!stp_is_initialised)
    {
      /* Things that are only initialised once */
      /* Set up gettext */
#ifdef ENABLE_NLS
      setlocale (LC_ALL, "");
      bindtextdomain (PACKAGE, PACKAGE_LOCALE_DIR);
#endif
      stp_init_debug();
    }
  stp_is_initialised = 1;
  return (0);
}

const char *
stp_set_output_codeset(const char *codeset)
{
#ifdef ENABLE_NLS
  return (const char *)(bind_textdomain_codeset(PACKAGE, codeset));
#else
  return "US-ASCII";
#endif
}

#ifdef QUANTIFY
unsigned quantify_counts[NUM_QUANTIFY_BUCKETS] = {0};
struct timeval quantify_buckets[NUM_QUANTIFY_BUCKETS] = {{0,0}};
int quantify_high_index = 0;
int quantify_first_time = 1;
struct timeval quantify_cur_time;
struct timeval quantify_prev_time;

void print_timers(const stp_vars_t v)
{
  int i;

  stp_eprintf(v, "%s", "Quantify timers:\n");
  for (i = 0; i <= quantify_high_index; i++)
    {
      if (quantify_counts[i] > 0)
	{
	  stp_eprintf(v,
		      "Bucket %d:\t%ld.%ld s\thit %u times\n", i,
		      quantify_buckets[i].tv_sec, quantify_buckets[i].tv_usec,
		      quantify_counts[i]);
	  quantify_buckets[i].tv_sec = 0;
	  quantify_buckets[i].tv_usec = 0;
	  quantify_counts[i] = 0;
	}
    }
}
#endif
