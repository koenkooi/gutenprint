/*
 * "$Id: print-vars.c,v 1.6.2.2 2002/11/10 04:46:13 rlk Exp $"
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
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#include "vars.h"
#include <string.h>
#if defined(HAVE_VARARGS_H) && !defined(HAVE_STDARG_H)
#include <varargs.h>
#else
#include <stdarg.h>
#endif

static const stp_internal_vars_t default_vars =
{
	COOKIE_VARS,
	N_ ("ps2"),	       	/* Name of printer "driver" */
	"",			/* Name of PPD file */
	"",			/* Output resolution */
	"",			/* Size of output media */
	"",			/* Type of output media */
	"",			/* Source of output media */
	"",			/* Ink type */
	"",			/* Dither algorithm */
	OUTPUT_COLOR,		/* Color or grayscale output */
	1.0,			/* Output brightness */
	-1,			/* left */
	-1,			/* top */
	-1,			/* width */
	-1,			/* height */
	1.0,			/* Screen gamma */
	1.0,			/* Contrast */
	1.0,			/* Cyan */
	1.0,			/* Magenta */
	1.0,			/* Yellow */
	1.0,			/* Output saturation */
	1.0,			/* Density */
	IMAGE_CONTINUOUS,	/* Image type */
	1.0,			/* Application gamma placeholder */
	0,			/* Page width */
	0,			/* Page height */
	COLOR_MODEL_RGB,	/* Input color model */
	COLOR_MODEL_RGB		/* Output color model */
};

static const stp_internal_vars_t min_vars =
{
	COOKIE_VARS,
	N_ ("ps2"),		/* Name of printer "driver" */
	"",			/* Name of PPD file */
	"",			/* Output resolution */
	"",			/* Size of output media */
	"",			/* Type of output media */
	"",			/* Source of output media */
	"",			/* Ink type */
	"",			/* Dither algorithm */
	0,			/* Color or grayscale output */
	0,			/* Output brightness */
	-1,			/* left */
	-1,			/* top */
	-1,			/* width */
	-1,			/* height */
	0.1,			/* Screen gamma */
	0,			/* Contrast */
	0,			/* Cyan */
	0,			/* Magenta */
	0,			/* Yellow */
	0,			/* Output saturation */
	.1,			/* Density */
	0,			/* Image type */
	1.0,			/* Application gamma placeholder */
	0,			/* Page width */
	0,			/* Page height */
	0,			/* Input color model */
	0			/* Output color model */
};

static const stp_internal_vars_t max_vars =
{
	COOKIE_VARS,
	N_ ("ps2"),		/* Name of printer "driver" */
	"",			/* Name of PPD file */
	"",			/* Output resolution */
	"",			/* Size of output media */
	"",			/* Type of output media */
	"",			/* Source of output media */
	"",			/* Ink type */
	"",			/* Dither algorithm */
	OUTPUT_RAW_PRINTER,	/* Color or grayscale output */
	2.0,			/* Output brightness */
	-1,			/* left */
	-1,			/* top */
	-1,			/* width */
	-1,			/* height */
	4.0,			/* Screen gamma */
	4.0,			/* Contrast */
	4.0,			/* Cyan */
	4.0,			/* Magenta */
	4.0,			/* Yellow */
	9.0,			/* Output saturation */
	2.0,			/* Density */
	NIMAGE_TYPES - 1,	/* Image type */
	1.0,			/* Application gamma placeholder */
	0,			/* Page width */
	0,			/* Page height */
	NCOLOR_MODELS - 1,	/* Input color model */
	NCOLOR_MODELS - 1	/* Output color model */
};

static const stp_parameter_t global_parameters[] =
  {
    {
      "PageSize", N_("Page Size"),
      N_("Size of the paper being printed to"),
      STP_PARAMETER_TYPE_STRING_LIST, STP_PARAMETER_CLASS_PAGE_SIZE,
      STP_PARAMETER_LEVEL_BASIC
    },
    {
      "MediaType", N_("Media Type"),
      N_("Type of media (plain paper, photo paper, etc.)"),
      STP_PARAMETER_TYPE_STRING_LIST, STP_PARAMETER_CLASS_FEATURE,
      STP_PARAMETER_LEVEL_BASIC
    },
    {
      "InputSlot", N_("Media Source"),
      N_("Source (input slot) of the media"),
      STP_PARAMETER_TYPE_STRING_LIST, STP_PARAMETER_CLASS_FEATURE,
      STP_PARAMETER_LEVEL_BASIC
    },
    {
      "InkType", N_("Ink Type"),
      N_("Type of ink in the printer"),
      STP_PARAMETER_TYPE_STRING_LIST, STP_PARAMETER_CLASS_FEATURE,
      STP_PARAMETER_LEVEL_BASIC
    },
    {
      "Resolution", N_("Resolutions"),
      N_("Resolution and quality of the print"),
      STP_PARAMETER_TYPE_STRING_LIST, STP_PARAMETER_CLASS_FEATURE,
      STP_PARAMETER_LEVEL_BASIC
    },
    {
      "DitherAlgorithm", N_("Dither Algorithm"),
      N_("Dithering method"),
      STP_PARAMETER_TYPE_STRING_LIST, STP_PARAMETER_CLASS_OUTPUT,
      STP_PARAMETER_LEVEL_BASIC
    },
    {
      "Brightness", N_("Brightness"),
      N_("Brightness of the print (0 is solid black, 2 is solid white)"),
      STP_PARAMETER_TYPE_DOUBLE, STP_PARAMETER_CLASS_OUTPUT,
      STP_PARAMETER_LEVEL_BASIC
    },
    {
      "Contrast", N_("Contrast"),
      N_("Contrast of the print (0 is solid gray)"),
      STP_PARAMETER_TYPE_DOUBLE, STP_PARAMETER_CLASS_OUTPUT,
      STP_PARAMETER_LEVEL_BASIC
    },
    {
      "Density", N_("Density"),
      N_("Amount of ink used in printing"),
      STP_PARAMETER_TYPE_DOUBLE, STP_PARAMETER_CLASS_OUTPUT,
      STP_PARAMETER_LEVEL_BASIC
    },
    {
      "Gamma", N_("Gamma"),
      N_("Adjust the gamma of the print (lower is darker, higher is lighter)"),
      STP_PARAMETER_TYPE_DOUBLE, STP_PARAMETER_CLASS_OUTPUT,
      STP_PARAMETER_LEVEL_BASIC
    },
    {
      "AppGamma", N_("AppGamma"),
      N_("Gamma value assumed by application"),
      STP_PARAMETER_TYPE_DOUBLE, STP_PARAMETER_CLASS_OUTPUT,
      STP_PARAMETER_LEVEL_ADVANCED
    },
    {
      "Cyan", N_("Cyan"),
      N_("Adjust the cyan balance"),
      STP_PARAMETER_TYPE_DOUBLE, STP_PARAMETER_CLASS_OUTPUT,
      STP_PARAMETER_LEVEL_BASIC
    },
    {
      "Magenta", N_("Magenta"),
      N_("Adjust the magenta balance"),
      STP_PARAMETER_TYPE_DOUBLE, STP_PARAMETER_CLASS_OUTPUT,
      STP_PARAMETER_LEVEL_BASIC
    },
    {
      "Yellow", N_("Yellow"),
      N_("Adjust the yellow balance"),
      STP_PARAMETER_TYPE_DOUBLE, STP_PARAMETER_CLASS_OUTPUT,
      STP_PARAMETER_LEVEL_BASIC
    },
    {
      "Saturation", N_("Saturation"),
      N_("Color saturation (0 is grayscale, larger numbers produce more brilliant colors"),
      STP_PARAMETER_TYPE_DOUBLE, STP_PARAMETER_CLASS_OUTPUT,
      STP_PARAMETER_LEVEL_BASIC
    },
    {
      NULL, NULL, NULL,
      STP_PARAMETER_TYPE_INVALID, STP_PARAMETER_CLASS_INVALID,
      STP_PARAMETER_LEVEL_INVALID
    }
  };

stp_vars_t
stp_allocate_vars(void)
{
  void *retval = stp_zalloc(sizeof(stp_internal_vars_t));
  stp_copy_vars(retval, (stp_vars_t)&default_vars);
  return (retval);
}

#define SAFE_FREE(x)				\
do						\
{						\
  if ((x))					\
    stp_free((char *)(x));			\
  ((x)) = NULL;					\
} while (0)

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

static void
check_vars(const stp_internal_vars_t *v)
{
  if (v->cookie != COOKIE_VARS)
    {
      stp_erprintf("Bad stp_vars_t!\n");
      exit(2);
    }
}

void
stp_free_vars(stp_vars_t vv)
{
  stp_internal_vars_t *v = (stp_internal_vars_t *) vv;
  check_vars(v);
  SAFE_FREE(v->driver);
  SAFE_FREE(v->ppd_file);
  SAFE_FREE(v->resolution);
  SAFE_FREE(v->media_size_name);
  SAFE_FREE(v->media_type);
  SAFE_FREE(v->media_source);
  SAFE_FREE(v->ink_type);
  SAFE_FREE(v->dither_algorithm);
  stp_free(v);
}

#define DEF_STRING_FUNCS(s, t)				\
t void							\
stp_set_##s(stp_vars_t vv, const char *val)		\
{							\
  stp_internal_vars_t *v = (stp_internal_vars_t *) vv;	\
  check_vars(v);					\
  if (v->s == val)					\
    return;						\
  SAFE_FREE(v->s);					\
  v->s = c_strdup(val);					\
  v->verified = 0;					\
}							\
							\
t void							\
stp_set_##s##_n(stp_vars_t vv, const char *val, int n)	\
{							\
  stp_internal_vars_t *v = (stp_internal_vars_t *) vv;	\
  check_vars(v);					\
  if (v->s == val)					\
    return;						\
  SAFE_FREE(v->s);					\
  v->s = c_strndup(val, n);				\
  v->verified = 0;					\
}							\
							\
t const char *						\
stp_get_##s(const stp_vars_t vv)			\
{							\
  stp_internal_vars_t *v = (stp_internal_vars_t *) vv;	\
  check_vars(v);					\
  return v->s;						\
}

#define DEF_FUNCS(s, t, u)				\
u void							\
stp_set_##s(stp_vars_t vv, t val)			\
{							\
  stp_internal_vars_t *v = (stp_internal_vars_t *) vv;	\
  check_vars(v);					\
  v->verified = 0;					\
  v->s = val;						\
}							\
							\
u t							\
stp_get_##s(const stp_vars_t vv)			\
{							\
  stp_internal_vars_t *v = (stp_internal_vars_t *) vv;	\
  check_vars(v);					\
  return v->s;						\
}

DEF_STRING_FUNCS(driver, )
DEF_STRING_FUNCS(ppd_file, )
DEF_STRING_FUNCS(resolution, static)
DEF_STRING_FUNCS(media_size_name, static)
DEF_STRING_FUNCS(media_type, static)
DEF_STRING_FUNCS(media_source, static)
DEF_STRING_FUNCS(ink_type, static)
DEF_STRING_FUNCS(dither_algorithm, static)
DEF_FUNCS(output_type, int, )
DEF_FUNCS(left, int, )
DEF_FUNCS(top, int, )
DEF_FUNCS(width, int, )
DEF_FUNCS(height, int, )
DEF_FUNCS(image_type, int, )
DEF_FUNCS(page_width, int, )
DEF_FUNCS(page_height, int, )
DEF_FUNCS(input_color_model, int, )
DEF_FUNCS(output_color_model, int, )
DEF_FUNCS(brightness, float, static)
DEF_FUNCS(gamma, float, static)
DEF_FUNCS(contrast, float, static)
DEF_FUNCS(cyan, float, static)
DEF_FUNCS(magenta, float, static)
DEF_FUNCS(yellow, float, static)
DEF_FUNCS(saturation, float, static)
DEF_FUNCS(density, float, static)
DEF_FUNCS(app_gamma, float, static)
DEF_FUNCS(lut, void *, )
DEF_FUNCS(outdata, void *, )
DEF_FUNCS(errdata, void *, )
DEF_FUNCS(driver_data, void *, )
DEF_FUNCS(cmap, unsigned char *, )
DEF_FUNCS(outfunc, stp_outfunc_t, )
DEF_FUNCS(errfunc, stp_outfunc_t, )

void
stp_set_verified(stp_vars_t vv, int val)
{
  stp_internal_vars_t *v = (stp_internal_vars_t *) vv;
  check_vars(v);
  v->verified = val;
}

int
stp_get_verified(const stp_vars_t vv)
{
  stp_internal_vars_t *v = (stp_internal_vars_t *) vv;
  check_vars(v);
  return v->verified;
}

void
stp_copy_options(stp_vars_t vd, const stp_vars_t vs)
{
  const stp_internal_vars_t *src = (const stp_internal_vars_t *)vs;
  stp_internal_vars_t *dest = (stp_internal_vars_t *)vd;
  stp_internal_option_t *popt = NULL;
  stp_internal_option_t *opt;
  check_vars(src);
  check_vars(dest);
  opt = (stp_internal_option_t *) src->options;
  if (opt)
    {
      stp_internal_option_t *nopt = stp_malloc(sizeof(stp_internal_option_t));
      stp_set_verified(vd, 0);
      dest->options = nopt;
      memcpy(nopt, opt, sizeof(stp_internal_option_t));
      nopt->name = stp_malloc(c_strlen(opt->name) + 1);
      strcpy(nopt->name, opt->name);
      nopt->data = stp_malloc(opt->length);
      memcpy(nopt->data, opt->data, opt->length);
      opt = opt->next;
      popt = nopt;
      while (opt)
        {
          nopt = stp_malloc(sizeof(stp_internal_option_t));
          memcpy(nopt, opt, sizeof(stp_internal_option_t));
          nopt->prev = popt;
          popt->next = nopt;
          nopt->name = stp_malloc(c_strlen(opt->name) + 1);
          strcpy(nopt->name, opt->name);
          nopt->data = stp_malloc(opt->length);
          memcpy(nopt->data, opt->data, opt->length);
          opt = opt->next;
          popt = nopt;
        }
    }
}

void
stp_set_string_parameter_n(stp_vars_t v, const char *parameter,
			   const char *value, int bytes)
{
  if      (strcmp(parameter, "Resolution") == 0)
    stp_set_resolution_n(v, value, bytes);
  else if (strcmp(parameter, "PageSize") == 0)
    stp_set_media_size_name_n(v, value, bytes);
  else if (strcmp(parameter, "MediaType") == 0)
    stp_set_media_type_n(v, value, bytes);
  else if (strcmp(parameter, "InputSlot") == 0)
    stp_set_media_source_n(v, value, bytes);
  else if (strcmp(parameter, "InkType") == 0)
    stp_set_ink_type_n(v, value, bytes);
  else if (strcmp(parameter, "DitherAlgorithm") == 0)
    stp_set_dither_algorithm_n(v, value, bytes);
  else
    stp_eprintf(v, "WARNING: Attempt to set unknown parameter %s to %s\n",
		parameter, value);
}

void
stp_set_parameter(stp_vars_t v, const char *parameter, ...)
{
  va_list args;
  va_start(args, parameter);
  if      (strcmp(parameter, "Resolution") == 0)
    stp_set_resolution(v, va_arg(args, const char *));
  else if (strcmp(parameter, "PageSize") == 0)
    stp_set_media_size_name(v, va_arg(args, const char *));
  else if (strcmp(parameter, "MediaType") == 0)
    stp_set_media_type(v, va_arg(args, const char *));
  else if (strcmp(parameter, "InputSlot") == 0)
    stp_set_media_source(v, va_arg(args, const char *));
  else if (strcmp(parameter, "InkType") == 0)
    stp_set_ink_type(v, va_arg(args, const char *));
  else if (strcmp(parameter, "DitherAlgorithm") == 0)
    stp_set_dither_algorithm(v, va_arg(args, const char *));
  else if (strcmp(parameter, "Brightness") == 0)
    stp_set_brightness(v, va_arg(args, double));
  else if (strcmp(parameter, "Contrast") == 0)
    stp_set_contrast(v, va_arg(args, double));
  else if (strcmp(parameter, "Density") == 0)
    stp_set_density(v, va_arg(args, double));
  else if (strcmp(parameter, "Gamma") == 0)
    stp_set_gamma(v, va_arg(args, double));
  else if (strcmp(parameter, "AppGamma") == 0)
    stp_set_app_gamma(v, va_arg(args, double));
  else if (strcmp(parameter, "Cyan") == 0)
    stp_set_cyan(v, va_arg(args, double));
  else if (strcmp(parameter, "Magenta") == 0)
    stp_set_magenta(v, va_arg(args, double));
  else if (strcmp(parameter, "Yellow") == 0)
    stp_set_yellow(v, va_arg(args, double));
  else if (strcmp(parameter, "Saturation") == 0)
    stp_set_saturation(v, va_arg(args, double));
  else
    {
      stp_parameter_type_t t = stp_parameter_type(v, parameter);
      switch (t)
	{
	case STP_PARAMETER_TYPE_STRING_LIST:
	case STP_PARAMETER_TYPE_FILE:
	  stp_eprintf(v,
		      "WARNING: Attempt to set unknown parameter %s to %s\n",
		      parameter, va_arg(args, const char *));
	  break;
	case STP_PARAMETER_TYPE_DOUBLE:
	  stp_eprintf(v,
		      "WARNING: Attempt to set unknown parameter %s to %s\n",
		      parameter, va_arg(args, double));
	  break;
	default:
	  stp_eprintf(v,
		      "WARNING: Attempt to set unknown parameter %s\n",
		      parameter);
	  break;
	}
    }
  va_end(args);
}

stp_parameter_value_t
stp_get_parameter(stp_vars_t v, const char *parameter)
{
  stp_parameter_value_t r;
  if      (strcmp(parameter, "Resolution") == 0)
    r.str = stp_get_resolution(v);
  else if (strcmp(parameter, "PageSize") == 0)
    r.str = stp_get_media_size_name(v);
  else if (strcmp(parameter, "MediaType") == 0)
    r.str = stp_get_media_type(v);
  else if (strcmp(parameter, "InputSlot") == 0)
    r.str = stp_get_media_source(v);
  else if (strcmp(parameter, "InkType") == 0)
    r.str = stp_get_ink_type(v);
  else if (strcmp(parameter, "DitherAlgorithm") == 0)
    r.str = stp_get_dither_algorithm(v);
  else if (strcmp(parameter, "Brightness") == 0)
    r.dbl = stp_get_brightness(v);
  else if (strcmp(parameter, "Contrast") == 0)
    r.dbl = stp_get_contrast(v);
  else if (strcmp(parameter, "Density") == 0)
    r.dbl = stp_get_density(v);
  else if (strcmp(parameter, "AppGamma") == 0)
    r.dbl = stp_get_app_gamma(v);
  else if (strcmp(parameter, "Gamma") == 0)
    r.dbl = stp_get_gamma(v);
  else if (strcmp(parameter, "Cyan") == 0)
    r.dbl = stp_get_cyan(v);
  else if (strcmp(parameter, "Magenta") == 0)
    r.dbl = stp_get_magenta(v);
  else if (strcmp(parameter, "Yellow") == 0)
    r.dbl = stp_get_yellow(v);
  else if (strcmp(parameter, "Saturation") == 0)
    r.dbl = stp_get_saturation(v);
  else
    {
      stp_eprintf(v, "WARNING: Attempt to retrieve unknown parameter %s\n",
		  parameter);
      r.str = NULL;
    }
  return r;
}

void
stp_copy_vars(stp_vars_t vd, const stp_vars_t vs)
{
  if (vs == vd)
    return;
  stp_set_driver(vd, stp_get_driver(vs));
  stp_set_driver_data(vd, stp_get_driver_data(vs));
  stp_set_ppd_file(vd, stp_get_ppd_file(vs));
  stp_set_resolution(vd, stp_get_resolution(vs));
  stp_set_media_size_name(vd, stp_get_media_size_name(vs));
  stp_set_media_type(vd, stp_get_media_type(vs));
  stp_set_media_source(vd, stp_get_media_source(vs));
  stp_set_ink_type(vd, stp_get_ink_type(vs));
  stp_set_dither_algorithm(vd, stp_get_dither_algorithm(vs));
  stp_set_output_type(vd, stp_get_output_type(vs));
  stp_set_left(vd, stp_get_left(vs));
  stp_set_top(vd, stp_get_top(vs));
  stp_set_width(vd, stp_get_width(vs));
  stp_set_height(vd, stp_get_height(vs));
  stp_set_image_type(vd, stp_get_image_type(vs));
  stp_set_page_width(vd, stp_get_page_width(vs));
  stp_set_page_height(vd, stp_get_page_height(vs));
  stp_set_brightness(vd, stp_get_brightness(vs));
  stp_set_gamma(vd, stp_get_gamma(vs));
  stp_set_contrast(vd, stp_get_contrast(vs));
  stp_set_cyan(vd, stp_get_cyan(vs));
  stp_set_magenta(vd, stp_get_magenta(vs));
  stp_set_yellow(vd, stp_get_yellow(vs));
  stp_set_saturation(vd, stp_get_saturation(vs));
  stp_set_density(vd, stp_get_density(vs));
  stp_set_app_gamma(vd, stp_get_app_gamma(vs));
  stp_set_input_color_model(vd, stp_get_input_color_model(vd));
  stp_set_output_color_model(vd, stp_get_output_color_model(vd));
  stp_set_lut(vd, stp_get_lut(vs));
  stp_set_outdata(vd, stp_get_outdata(vs));
  stp_set_errdata(vd, stp_get_errdata(vs));
  stp_set_cmap(vd, stp_get_cmap(vs));
  stp_set_outfunc(vd, stp_get_outfunc(vs));
  stp_set_errfunc(vd, stp_get_errfunc(vs));
  stp_copy_options(vd, vs);
  stp_set_verified(vd, stp_get_verified(vs));
}

stp_vars_t
stp_allocate_copy(const stp_vars_t vs)
{
  stp_vars_t vd = stp_allocate_vars();
  stp_copy_vars(vd, vs);
  return (vd);
}

#define ICLAMP(value)						\
do								\
{								\
  if (stp_get_##value(user) < stp_get_##value(min))		\
    stp_set_##value(user, stp_get_##value(min));		\
  else if (stp_get_##value(user) > stp_get_##value(max))	\
    stp_set_##value(user, stp_get_##value(max));		\
} while (0)

void
stp_merge_printvars(stp_vars_t user, const stp_vars_t print)
{
  const stp_vars_t max = stp_maximum_settings();
  const stp_vars_t min = stp_minimum_settings();
  stp_set_cyan(user, stp_get_cyan(user) * stp_get_cyan(print));
  ICLAMP(cyan);
  stp_set_magenta(user, stp_get_magenta(user) * stp_get_magenta(print));
  ICLAMP(magenta);
  stp_set_yellow(user, stp_get_yellow(user) * stp_get_yellow(print));
  ICLAMP(yellow);
  stp_set_contrast(user, stp_get_contrast(user) * stp_get_contrast(print));
  ICLAMP(contrast);
  stp_set_brightness(user, stp_get_brightness(user)*stp_get_brightness(print));
  ICLAMP(brightness);
  stp_set_gamma(user, stp_get_gamma(user) / stp_get_gamma(print));
  ICLAMP(gamma);
  stp_set_saturation(user, stp_get_saturation(user)*stp_get_saturation(print));
  ICLAMP(saturation);
  stp_set_density(user, stp_get_density(user) * stp_get_density(print));
  ICLAMP(density);
  if (stp_get_output_type(print) == OUTPUT_GRAY &&
      (stp_get_output_type(user) == OUTPUT_COLOR ||
       stp_get_output_type(user) == OUTPUT_RAW_CMYK))
    stp_set_output_type(user, OUTPUT_GRAY);
}

void
stp_set_printer_defaults(stp_vars_t v, const stp_printer_t p)
{
  stp_set_driver(v, stp_printer_get_driver(p));
  stp_set_parameter
    (v, "Resolution", stp_get_default_parameter(v, "Resolution").str);
  stp_set_parameter
    (v, "InkType", stp_get_default_parameter(v, "InkType").str);
  stp_set_parameter
    (v, "MediaType", stp_get_default_parameter(v, "MediaType").str);
  stp_set_parameter
    (v, "InputSlot", stp_get_default_parameter(v,"InputSlot").str);
  stp_set_parameter
    (v, "PageSize", stp_get_default_parameter(v, "PageSize").str);
  stp_set_parameter
    (v, "DitherAlgorithm", stp_get_default_parameter(v,"DitherAlgorithm").str);
}

void
stp_describe_internal_parameter(const stp_vars_t v, const char *name,
				stp_parameter_description_t *description)
{
  description->class = STP_PARAMETER_CLASS_OUTPUT;
  description->level = STP_PARAMETER_LEVEL_BASIC;
  if (strcmp(name, "DitherAlgorithm") == 0)
    {
      description->type = STP_PARAMETER_TYPE_STRING_LIST;
      description->restrictions.string_list = stp_param_list_allocate();
      stp_dither_algorithms(description->restrictions.string_list);
      return;
    }
  description->type = stp_parameter_type(stp_default_settings(), name);
  description->class = stp_parameter_class(stp_default_settings(), name);
  description->level = stp_parameter_level(stp_default_settings(), name);
  if (description->type != STP_PARAMETER_TYPE_INVALID &&
      description->type == STP_PARAMETER_TYPE_DOUBLE)
    {
      description->restrictions.double_bounds.lower =
	stp_get_parameter(stp_minimum_settings(), name).dbl;
      description->restrictions.double_bounds.upper =
	stp_get_parameter(stp_maximum_settings(), name).dbl;
    }
  else
    description->type = STP_PARAMETER_TYPE_INVALID;
}

stp_parameter_value_t
stp_default_internal_parameter(const stp_vars_t v, const char *name)
{
  if (strcmp(name, "DitherAlgorithm") == 0)
    return stp_get_default_dither_algorithm();
  else
    return stp_get_parameter(stp_default_settings(), name);
}

const stp_vars_t
stp_default_settings()
{
  return (stp_vars_t) &default_vars;
}

const stp_vars_t
stp_maximum_settings()
{
  return (stp_vars_t) &max_vars;
}

const stp_vars_t
stp_minimum_settings()
{
  return (stp_vars_t) &min_vars;
}

const stp_parameter_t *
stp_list_parameters(const stp_vars_t v, int *count)
{
  *count = sizeof(global_parameters) / sizeof(char *);
  return global_parameters;
}

stp_parameter_type_t
stp_parameter_type(const stp_vars_t v, const char *parameter)
{
  static const stp_parameter_t *param = global_parameters;
  while (param->name)
    {
      if (strcmp(parameter, param->name) == 0)
	return param->type;
      param++;
    }
  return STP_PARAMETER_TYPE_INVALID;
}

stp_parameter_class_t
stp_parameter_class(const stp_vars_t v, const char *parameter)
{
  static const stp_parameter_t *param = global_parameters;
  while (param->name)
    {
      if (strcmp(parameter, param->name) == 0)
	return param->class;
      param++;
    }
  return STP_PARAMETER_CLASS_INVALID;
}

stp_parameter_level_t
stp_parameter_level(const stp_vars_t v, const char *parameter)
{
  static const stp_parameter_t *param = global_parameters;
  while (param->name)
    {
      if (strcmp(parameter, param->name) == 0)
	return param->level;
      param++;
    }
  return STP_PARAMETER_LEVEL_INVALID;
}

