/*
 * "$Id: print-vars.c,v 1.18 2003/01/03 07:51:55 mtomlinson Exp $"
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
#include <stdlib.h>

#define COOKIE_VARS      0x1a18376c

typedef struct					/* Plug-in variables */
{
  int	cookie;
  const char *driver;		/* Name of printer "driver" */
  int	output_type;		/* Color or grayscale output */
  const char *ppd_file;		/* PPD file */
  int	left;			/* Offset from left-upper corner, points */
  int	top;			/* ... */
  int	width;			/* Width of the image, points */
  int	height;			/* ... */
  int	image_type;		/* Image type (line art etc.) */
  float app_gamma;		/* Application gamma */
  int	page_width;		/* Width of page in points */
  int	page_height;		/* Height of page in points */
  int	input_color_model;	/* Color model for this device */
  int	output_color_model;	/* Color model for this device */
  int	page_number;
  stp_job_mode_t job_mode;
  stp_list_t *params[STP_PARAMETER_TYPE_INVALID];
  void  *color_data;		/* Private data of the color module */
  void	*(*copy_color_data_func)(const stp_vars_t);
  void	(*destroy_color_data_func)(stp_vars_t);
  void  *driver_data;		/* Private data of the family driver module */
  void	*(*copy_driver_data_func)(const stp_vars_t);
  void	(*destroy_driver_data_func)(stp_vars_t);
  void (*outfunc)(void *data, const char *buffer, size_t bytes);
  void *outdata;
  void (*errfunc)(void *data, const char *buffer, size_t bytes);
  void *errdata;
  int verified;			/* Ensure that params are OK! */
} stp_internal_vars_t;

static int standard_vars_initialized = 0;

static stp_internal_vars_t default_vars =
{
	COOKIE_VARS,
	N_ ("ps2"),	       	/* Name of printer "driver" */
	OUTPUT_COLOR,		/* Color or grayscale output */
	"",			/* Name of PPD file */
	-1,			/* left */
	-1,			/* top */
	-1,			/* width */
	-1,			/* height */
	IMAGE_CONTINUOUS,	/* Image type */
	1.0,			/* Application gamma placeholder */
	0,			/* Page width */
	0,			/* Page height */
	COLOR_MODEL_RGB,	/* Input color model */
	COLOR_MODEL_RGB,	/* Output color model */
	0,			/* Page number */
	STP_JOB_MODE_PAGE	/* Job mode */
};

static stp_internal_vars_t min_vars =
{
	COOKIE_VARS,
	N_ ("ps2"),		/* Name of printer "driver" */
	0,			/* Color or grayscale output */
	"",			/* Name of PPD file */
	-1,			/* left */
	-1,			/* top */
	-1,			/* width */
	-1,			/* height */
	0,			/* Image type */
	1.0,			/* Application gamma placeholder */
	0,			/* Page width */
	0,			/* Page height */
	0,			/* Input color model */
	0,			/* Output color model */
	0,			/* Page number */
	STP_JOB_MODE_PAGE	/* Job mode */
};

static stp_internal_vars_t max_vars =
{
	COOKIE_VARS,
	N_ ("ps2"),		/* Name of printer "driver" */
	OUTPUT_RAW_PRINTER,	/* Color or grayscale output */
	"",			/* Name of PPD file */
	-1,			/* left */
	-1,			/* top */
	-1,			/* width */
	-1,			/* height */
	NIMAGE_TYPES - 1,	/* Image type */
	1.0,			/* Application gamma placeholder */
	0,			/* Page width */
	0,			/* Page height */
	NCOLOR_MODELS - 1,	/* Input color model */
	NCOLOR_MODELS - 1,	/* Output color model */
	INT_MAX,		/* Page number */
	STP_JOB_MODE_JOB	/* Job mode */
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
      N_("Choose the dither algorithm to be used.\n"
	 "Adaptive Hybrid usually produces the best all-around quality.\n"
	 "EvenTone is a new, experimental algorithm that often produces excellent results.\n"
	 "Ordered is faster and produces almost as good quality on photographs.\n"
	 "Fast and Very Fast are considerably faster, and work well for text and line art.\n"
	 "Hybrid Floyd-Steinberg generally produces inferior output."),
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
      N_("Adjust the density (amount of ink) of the print. "
	 "Reduce the density if the ink bleeds through the "
	 "paper or smears; increase the density if black "
	 "regions are not solid."),
      STP_PARAMETER_TYPE_DOUBLE, STP_PARAMETER_CLASS_OUTPUT,
      STP_PARAMETER_LEVEL_BASIC
    },
    {
      "Gamma", N_("Gamma"),
      N_("Adjust the gamma of the print. Larger values will "
	 "produce a generally brighter print, while smaller "
	 "values will produce a generally darker print. "
	 "Black and white will remain the same, unlike with "
	 "the brightness adjustment."),
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
      N_("Adjust the saturation (color balance) of the print\n"
	 "Use zero saturation to produce grayscale output "
	 "using color and black inks"),
      STP_PARAMETER_TYPE_DOUBLE, STP_PARAMETER_CLASS_OUTPUT,
      STP_PARAMETER_LEVEL_BASIC
    },
  };

const stp_vars_t
stp_default_settings(void)
{
  return (stp_vars_t) &default_vars;
}

const stp_vars_t
stp_maximum_settings(void)
{
  return (stp_vars_t) &max_vars;
}

const stp_vars_t
stp_minimum_settings(void)
{
  return (stp_vars_t) &min_vars;
}

typedef struct
{
  char *name;
  stp_parameter_type_t typ;
  union
  {
    int ival;
    double dval;
    stp_curve_t cval;
    stp_raw_t rval;
  } value;
} value_t;

static const char *
value_namefunc(const stp_list_item_t *item)
{
  const value_t *v = (value_t *)stp_list_item_get_data(item);
  return v->name;
}

static void
value_freefunc(stp_list_item_t *item)
{
  value_t *v = (value_t *)stp_list_item_get_data(item);
  switch (v->typ)
    {
    case STP_PARAMETER_TYPE_STRING_LIST:
    case STP_PARAMETER_TYPE_FILE:
    case STP_PARAMETER_TYPE_RAW:
      stp_free(v->value.rval.data);
      break;
    case STP_PARAMETER_TYPE_CURVE:
      stp_curve_destroy(v->value.cval);
      break;
    default:
      break;
    }
  stp_free(v->name);
  stp_free(v);
}

static value_t *
value_copy(const stp_list_item_t *item)
{
  value_t *ret = stp_malloc(sizeof(value_t));
  const value_t *v = (value_t *)stp_list_item_get_data(item);
  ret->name = stp_strdup(v->name);
  ret->typ = v->typ;
  switch (v->typ)
    {
    case STP_PARAMETER_TYPE_CURVE:
      ret->value.cval = stp_curve_allocate_copy(ret->value.cval);
      break;
    case STP_PARAMETER_TYPE_STRING_LIST:
    case STP_PARAMETER_TYPE_FILE:
    case STP_PARAMETER_TYPE_RAW:
      ret->value.rval.bytes = v->value.rval.bytes;
      ret->value.rval.data = stp_malloc(ret->value.rval.bytes + 1);
      memcpy(ret->value.rval.data, v->value.rval.data, v->value.rval.bytes);
      ((char *) (ret->value.rval.data))[v->value.rval.bytes] = '\0';
      break;
    case STP_PARAMETER_TYPE_INT:
      ret->value.ival = v->value.ival;
      break;
    case STP_PARAMETER_TYPE_DOUBLE:
      ret->value.dval = v->value.dval;
      break;
    default:
      break;
    }
  return ret;
}

static stp_list_t *
create_vars_list(void)
{
  stp_list_t *ret = stp_list_create();
  stp_list_set_freefunc(ret, value_freefunc);
  stp_list_set_namefunc(ret, value_namefunc);
  return ret;
}
  
static stp_list_t *
copy_value_list(const stp_list_t *src)
{
  stp_list_t *ret = create_vars_list();
  stp_list_item_t *item = stp_list_get_start((stp_list_t *)src);
  while (item)
    {
      stp_list_item_create(ret, NULL, value_copy(item));
      item = stp_list_item_next(item);
    }
  return ret;
}

static void
initialize_standard_vars(void)
{
  if (!standard_vars_initialized)
    {
      int i;
      for (i = 0; i < STP_PARAMETER_TYPE_INVALID; i++)
	{
	  min_vars.params[i] = create_vars_list();
	  max_vars.params[i] = create_vars_list();
	  default_vars.params[i] = create_vars_list();
	}
      stp_set_float_parameter((stp_vars_t) &min_vars, "Brightness", 0.0);
      stp_set_float_parameter((stp_vars_t) &max_vars, "Brightness", 2.0);
      stp_set_float_parameter((stp_vars_t) &default_vars, "Brightness", 1.0);
      stp_set_float_parameter((stp_vars_t) &min_vars, "Gamma", 0.1);
      stp_set_float_parameter((stp_vars_t) &max_vars, "Gamma", 4.0);
      stp_set_float_parameter((stp_vars_t) &default_vars, "Gamma", 1.0);
      stp_set_float_parameter((stp_vars_t) &min_vars, "Contrast", 0.0);
      stp_set_float_parameter((stp_vars_t) &max_vars, "Contrast", 4.0);
      stp_set_float_parameter((stp_vars_t) &default_vars, "Contrast", 1.0);
      stp_set_float_parameter((stp_vars_t) &min_vars, "Cyan", 0.0);
      stp_set_float_parameter((stp_vars_t) &max_vars, "Cyan", 4.0);
      stp_set_float_parameter((stp_vars_t) &default_vars, "Cyan", 1.0);
      stp_set_float_parameter((stp_vars_t) &min_vars, "Magenta", 0.0);
      stp_set_float_parameter((stp_vars_t) &max_vars, "Magenta", 4.0);
      stp_set_float_parameter((stp_vars_t) &default_vars, "Magenta", 1.0);
      stp_set_float_parameter((stp_vars_t) &min_vars, "Yellow", 0.0);
      stp_set_float_parameter((stp_vars_t) &max_vars, "Yellow", 4.0);
      stp_set_float_parameter((stp_vars_t) &default_vars, "Yellow", 1.0);
      stp_set_float_parameter((stp_vars_t) &min_vars, "Saturation", 0.0);
      stp_set_float_parameter((stp_vars_t) &max_vars, "Saturation", 9.0);
      stp_set_float_parameter((stp_vars_t) &default_vars, "Saturation", 1.0);
      stp_set_float_parameter((stp_vars_t) &min_vars, "Density", 0.1);
      stp_set_float_parameter((stp_vars_t) &max_vars, "Density", 2.0);
      stp_set_float_parameter((stp_vars_t) &default_vars, "Density", 1.0);
    }
}

stp_vars_t
stp_allocate_vars(void)
{
  int i;
  stp_internal_vars_t *retval = stp_zalloc(sizeof(stp_internal_vars_t));
  initialize_standard_vars();
  retval->cookie = COOKIE_VARS;
  for (i = 0; i < STP_PARAMETER_TYPE_INVALID; i++)
    retval->params[i] = create_vars_list();
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

static void
check_vars(const stp_internal_vars_t *v)
{
  if (v->cookie != COOKIE_VARS)
    {
      stp_erprintf("Bad stp_vars_t!\n");
      stp_abort();
    }
}

void
stp_vars_free(stp_vars_t vv)
{
  int i;
  stp_internal_vars_t *v = (stp_internal_vars_t *) vv;
  check_vars(v);
  if (stp_get_destroy_color_data_func(vv))
    (*stp_get_destroy_color_data_func(vv))(vv);
  if (stp_get_destroy_driver_data_func(vv))
    (*stp_get_destroy_driver_data_func(vv))(vv);
  for (i = 0; i < STP_PARAMETER_TYPE_INVALID; i++)
    stp_list_destroy(v->params[i]);
  SAFE_FREE(v->driver);
  SAFE_FREE(v->ppd_file);
  stp_free(v);
}

#define DEF_STRING_FUNCS(s)				\
void							\
stp_set_##s(stp_vars_t vv, const char *val)		\
{							\
  stp_internal_vars_t *v = (stp_internal_vars_t *) vv;	\
  check_vars(v);					\
  if (v->s == val)					\
    return;						\
  SAFE_FREE(v->s);					\
  v->s = stp_strdup(val);				\
  v->verified = 0;					\
}							\
							\
void							\
stp_set_##s##_n(stp_vars_t vv, const char *val, int n)	\
{							\
  stp_internal_vars_t *v = (stp_internal_vars_t *) vv;	\
  check_vars(v);					\
  if (v->s == val)					\
    return;						\
  SAFE_FREE(v->s);					\
  v->s = stp_strndup(val, n);				\
  v->verified = 0;					\
}							\
							\
const char *						\
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

DEF_STRING_FUNCS(driver)
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
DEF_FUNCS(page_number, int, )
DEF_FUNCS(job_mode, stp_job_mode_t, )
DEF_FUNCS(outdata, void *, )
DEF_FUNCS(errdata, void *, )
DEF_FUNCS(color_data, void *, )
DEF_FUNCS(copy_color_data_func, copy_data_func_t, )
DEF_FUNCS(destroy_color_data_func, destroy_data_func_t, )
DEF_FUNCS(driver_data, void *, )
DEF_FUNCS(copy_driver_data_func, copy_data_func_t, )
DEF_FUNCS(destroy_driver_data_func, destroy_data_func_t, )
DEF_FUNCS(outfunc, stp_outfunc_t, )
DEF_FUNCS(errfunc, stp_outfunc_t, )

void
stp_set_ppd_file(stp_vars_t v, const char *ppd_file)
{
  stp_set_file_parameter(v, "PPDFile", ppd_file);
}

void
stp_set_ppd_file_n(stp_vars_t v, const char *ppd_file, int bytes)
{
  stp_set_file_parameter_n(v, "PPDFile", ppd_file, bytes);
}

const char *
stp_get_ppd_file(stp_vars_t v)
{
  return stp_get_file_parameter(v, "PPDFile");
}


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

static void
set_raw_parameter(stp_list_t *list, const char *parameter, const char *value,
		  int bytes, int typ)
{
  stp_list_item_t *item = stp_list_get_item_by_name(list, parameter);
  if (value)
    {
      value_t *v;
      if (item)
	{
	  v = (value_t *) stp_list_item_get_data(item);
	  stp_free(v->value.rval.data);
	}
      else
	{
	  v = stp_malloc(sizeof(value_t));
	  v->name = stp_strdup(parameter);
	  v->typ = typ;
	  stp_list_item_create(list, NULL, v);
	}
      v->value.rval.data = stp_malloc(bytes + 1);
      memcpy(v->value.rval.data, value, bytes);
      ((char *) v->value.rval.data)[bytes] = '\0';
      v->value.rval.bytes = bytes;
    }
  else if (item)
    stp_list_item_destroy(list, item);
}

void
stp_set_string_parameter_n(stp_vars_t v, const char *parameter,
			   const char *value, int bytes)
{
  stp_internal_vars_t *vv = (stp_internal_vars_t *)v;
  stp_list_t *list = vv->params[STP_PARAMETER_TYPE_STRING_LIST];
  set_raw_parameter(list, parameter, value, bytes,
		    STP_PARAMETER_TYPE_STRING_LIST);
}

void
stp_set_string_parameter(stp_vars_t v, const char *parameter,
			 const char *value)
{
  int byte_count = 0;
  if (value)
    byte_count = strlen(value);
  stp_set_string_parameter_n(v, parameter, value, byte_count);
}

const char *
stp_get_string_parameter(const stp_vars_t v, const char *parameter)
{
  stp_internal_vars_t *vv = (stp_internal_vars_t *)v;
  stp_list_t *list = vv->params[STP_PARAMETER_TYPE_STRING_LIST];
  value_t *val;
  stp_list_item_t *item = stp_list_get_item_by_name(list, parameter);
  if (item)
    {
      val = (value_t *) stp_list_item_get_data(item);
      return val->value.rval.data;
    }
  else
    return NULL;
}

void
stp_set_raw_parameter(stp_vars_t v, const char *parameter,
		      const void *value, int bytes)
{
  stp_internal_vars_t *vv = (stp_internal_vars_t *)v;
  stp_list_t *list = vv->params[STP_PARAMETER_TYPE_RAW];
  set_raw_parameter(list, parameter, value, bytes, STP_PARAMETER_TYPE_RAW);
}

const stp_raw_t *
stp_get_raw_parameter(const stp_vars_t v, const char *parameter)
{
  stp_internal_vars_t *vv = (stp_internal_vars_t *)v;
  stp_list_t *list = vv->params[STP_PARAMETER_TYPE_RAW];
  value_t *val;
  stp_list_item_t *item = stp_list_get_item_by_name(list, parameter);
  if (item)
    {
      val = (value_t *) stp_list_item_get_data(item);
      return &(val->value.rval);
    }
  else
    return NULL;
}

void
stp_set_file_parameter(stp_vars_t v, const char *parameter,
		       const char *value)
{
  stp_internal_vars_t *vv = (stp_internal_vars_t *)v;
  stp_list_t *list = vv->params[STP_PARAMETER_TYPE_FILE];
  int byte_count = 0;
  if (value)
    byte_count = strlen(value);
  set_raw_parameter(list, parameter, value, byte_count,
		    STP_PARAMETER_TYPE_FILE);
}

void
stp_set_file_parameter_n(stp_vars_t v, const char *parameter,
			 const char *value, int byte_count)
{
  stp_internal_vars_t *vv = (stp_internal_vars_t *)v;
  stp_list_t *list = vv->params[STP_PARAMETER_TYPE_FILE];
  set_raw_parameter(list, parameter, value, byte_count,
		    STP_PARAMETER_TYPE_FILE);
}

const char *
stp_get_file_parameter(const stp_vars_t v, const char *parameter)
{
  stp_internal_vars_t *vv = (stp_internal_vars_t *)v;
  stp_list_t *list = vv->params[STP_PARAMETER_TYPE_FILE];
  value_t *val;
  stp_list_item_t *item = stp_list_get_item_by_name(list, parameter);
  if (item)
    {
      val = (value_t *) stp_list_item_get_data(item);
      return val->value.rval.data;
    }
  else
    return NULL;
}

void
stp_set_curve_parameter(stp_vars_t v, const char *parameter,
			const stp_curve_t curve)
{
  stp_internal_vars_t *vv = (stp_internal_vars_t *)v;
  stp_list_t *list = vv->params[STP_PARAMETER_TYPE_CURVE];
  stp_list_item_t *item = stp_list_get_item_by_name(list, parameter);
  if (curve)
    {
      value_t *val;
      if (item)
	{
	  val = (value_t *) stp_list_item_get_data(item);
	  stp_curve_destroy(val->value.cval);
	}
      else
	{
	  val = stp_malloc(sizeof(value_t));
	  val->name = stp_strdup(parameter);
	  val->typ = STP_PARAMETER_TYPE_CURVE;
	  stp_list_item_create(list, NULL, val);
	}
      val->value.cval = stp_curve_allocate_copy(curve);
    }
  else if (item)
    stp_list_item_destroy(list, item);
}

const stp_curve_t
stp_get_curve_parameter(const stp_vars_t v, const char *parameter)
{
  stp_internal_vars_t *vv = (stp_internal_vars_t *)v;
  stp_list_t *list = vv->params[STP_PARAMETER_TYPE_CURVE];
  value_t *val;
  stp_list_item_t *item = stp_list_get_item_by_name(list, parameter);
  if (item)
    {
      val = (value_t *) stp_list_item_get_data(item);
      return val->value.cval;
    }
  else
    return NULL;
}

void
stp_set_int_parameter(stp_vars_t v, const char *parameter, int ival)
{
  stp_internal_vars_t *vv = (stp_internal_vars_t *)v;
  stp_list_t *list = vv->params[STP_PARAMETER_TYPE_INT];
  value_t *val;
  stp_list_item_t *item = stp_list_get_item_by_name(list, parameter);
  if (item)
    {
      val = (value_t *) stp_list_item_get_data(item);
    }
  else
    {
      val = stp_malloc(sizeof(value_t));
      val->name = stp_strdup(parameter);
      val->typ = STP_PARAMETER_TYPE_INT;
      stp_list_item_create(list, NULL, val);
    }
  val->value.ival = ival;
}

const int
stp_get_int_parameter(const stp_vars_t v, const char *parameter)
{
  stp_internal_vars_t *vv = (stp_internal_vars_t *)v;
  stp_list_t *list = vv->params[STP_PARAMETER_TYPE_INT];
  value_t *val;
  stp_list_item_t *item = stp_list_get_item_by_name(list, parameter);
  if (item)
    {
      val = (value_t *) stp_list_item_get_data(item);
      return val->value.ival;
    }
  else
    return 0;
}

void
stp_set_float_parameter(stp_vars_t v, const char *parameter, double dval)
{
  stp_internal_vars_t *vv = (stp_internal_vars_t *)v;
  stp_list_t *list = vv->params[STP_PARAMETER_TYPE_DOUBLE];
  value_t *val;
  stp_list_item_t *item = stp_list_get_item_by_name(list, parameter);
  if (item)
    {
      val = (value_t *) stp_list_item_get_data(item);
    }
  else
    {
      val = stp_malloc(sizeof(value_t));
      val->name = stp_strdup(parameter);
      val->typ = STP_PARAMETER_TYPE_DOUBLE;
      stp_list_item_create(list, NULL, val);
    }
  val->value.dval = dval;
}

const double
stp_get_float_parameter(const stp_vars_t v, const char *parameter)
{
  stp_internal_vars_t *vv = (stp_internal_vars_t *)v;
  stp_list_t *list = vv->params[STP_PARAMETER_TYPE_DOUBLE];
  value_t *val;
  stp_list_item_t *item = stp_list_get_item_by_name(list, parameter);
  if (item)
    {
      val = (value_t *) stp_list_item_get_data(item);
      return val->value.dval;
    }
  else
    return 1.0;
}

int
stp_check_string_parameter(const stp_vars_t v, const char *parameter)
{
  stp_internal_vars_t *vv = (stp_internal_vars_t *)v;
  stp_list_t *list = vv->params[STP_PARAMETER_TYPE_STRING_LIST];
  stp_list_item_t *item = stp_list_get_item_by_name(list, parameter);
  return item ? 1 : 0;
}

int
stp_check_file_parameter(const stp_vars_t v, const char *parameter)
{
  stp_internal_vars_t *vv = (stp_internal_vars_t *)v;
  stp_list_t *list = vv->params[STP_PARAMETER_TYPE_FILE];
  stp_list_item_t *item = stp_list_get_item_by_name(list, parameter);
  return item ? 1 : 0;
}

int
stp_check_float_parameter(const stp_vars_t v, const char *parameter)
{
  stp_internal_vars_t *vv = (stp_internal_vars_t *)v;
  stp_list_t *list = vv->params[STP_PARAMETER_TYPE_DOUBLE];
  stp_list_item_t *item = stp_list_get_item_by_name(list, parameter);
  return item ? 1 : 0;
}

int
stp_check_int_parameter(const stp_vars_t v, const char *parameter)
{
  stp_internal_vars_t *vv = (stp_internal_vars_t *)v;
  stp_list_t *list = vv->params[STP_PARAMETER_TYPE_INT];
  stp_list_item_t *item = stp_list_get_item_by_name(list, parameter);
  return item ? 1 : 0;
}

int
stp_check_curve_parameter(const stp_vars_t v, const char *parameter)
{
  stp_internal_vars_t *vv = (stp_internal_vars_t *)v;
  stp_list_t *list = vv->params[STP_PARAMETER_TYPE_CURVE];
  stp_list_item_t *item = stp_list_get_item_by_name(list, parameter);
  return item ? 1 : 0;
}

int
stp_check_raw_parameter(const stp_vars_t v, const char *parameter)
{
  stp_internal_vars_t *vv = (stp_internal_vars_t *)v;
  stp_list_t *list = vv->params[STP_PARAMETER_TYPE_RAW];
  stp_list_item_t *item = stp_list_get_item_by_name(list, parameter);
  return item ? 1 : 0;
}

void
stp_fill_parameter_settings(stp_parameter_t *desc, const char *name)
{
  const stp_parameter_t *param = global_parameters;
  while (param->name)
    {
      if (strcmp(name, param->name) == 0)
	{
	  desc->type = param->type;
	  desc->level = param->level;
	  desc->class = param->class;
	  desc->name = stp_strdup(param->name);
	  desc->text = stp_strdup(param->text);
	  desc->help = stp_strdup(param->help);
	  return;
	}
      param++;
    }
}

void
stp_copy_vars(stp_vars_t vd, const stp_vars_t vs)
{
  int count;
  int i;
  stp_parameter_list_t params;
  stp_internal_vars_t *vvd = (stp_internal_vars_t *)vd;
  const stp_internal_vars_t *vvs = (const stp_internal_vars_t *)vs;

  if (vs == vd)
    return;
  stp_set_driver(vd, stp_get_driver(vs));
  if (stp_get_copy_driver_data_func(vs))
    stp_set_driver_data(vd, (stp_get_copy_driver_data_func(vs))(vs));
  else
    stp_set_driver_data(vd, stp_get_driver_data(vs));
  if (stp_get_copy_color_data_func(vs))
    stp_set_color_data(vd, (stp_get_copy_color_data_func(vs))(vs));
  else
    stp_set_color_data(vd, stp_get_color_data(vs));
  for (i = 0; i < STP_PARAMETER_TYPE_INVALID; i++)
    {
      stp_list_destroy(vvd->params[i]);
      vvd->params[i] = copy_value_list(vvs->params[i]);
    }

  stp_set_copy_driver_data_func(vd, stp_get_copy_driver_data_func(vs));
  stp_set_copy_color_data_func(vd, stp_get_copy_color_data_func(vs));
  stp_set_ppd_file(vd, stp_get_ppd_file(vs));
  stp_set_output_type(vd, stp_get_output_type(vs));
  stp_set_left(vd, stp_get_left(vs));
  stp_set_top(vd, stp_get_top(vs));
  stp_set_width(vd, stp_get_width(vs));
  stp_set_height(vd, stp_get_height(vs));
  stp_set_image_type(vd, stp_get_image_type(vs));
  stp_set_page_width(vd, stp_get_page_width(vs));
  stp_set_page_height(vd, stp_get_page_height(vs));
  stp_set_input_color_model(vd, stp_get_input_color_model(vd));
  stp_set_output_color_model(vd, stp_get_output_color_model(vd));
  stp_set_outdata(vd, stp_get_outdata(vs));
  stp_set_errdata(vd, stp_get_errdata(vs));
  stp_set_outfunc(vd, stp_get_outfunc(vs));
  stp_set_errfunc(vd, stp_get_errfunc(vs));
  params = stp_list_parameters(vs);
  count = stp_parameter_list_count(params);
  stp_set_verified(vd, stp_get_verified(vs));
}

stp_vars_t
stp_allocate_copy(const stp_vars_t vs)
{
  stp_vars_t vd = stp_allocate_vars();
  stp_copy_vars(vd, vs);
  return (vd);
}

void
stp_merge_printvars(stp_vars_t user, const stp_vars_t print)
{
  int count;
  int i;
  stp_parameter_list_t params = stp_list_parameters(print);
  count = stp_parameter_list_count(params);
  for (i = 0; i < count; i++)
    {
      const stp_parameter_t *p = stp_parameter_list_param(params, i);
      if (p->type == STP_PARAMETER_TYPE_DOUBLE &&
	  p->class == STP_PARAMETER_CLASS_OUTPUT &&
	  p->level == STP_PARAMETER_LEVEL_BASIC)
	{
	  stp_parameter_t desc;
	  double usrval = stp_get_float_parameter(user, p->name);
	  double prnval = stp_get_float_parameter(print, p->name);
	  stp_describe_parameter(print, p->name, &desc);
	  if (strcmp(p->name, "Gamma") == 0)
	    usrval /= prnval;
	  else
	    usrval *= prnval;
	  if (usrval < desc.bounds.dbl.lower)
	    usrval = desc.bounds.dbl.lower;
	  else if (usrval > desc.bounds.dbl.upper)
	    usrval = desc.bounds.dbl.upper;
	  stp_set_float_parameter(user, p->name, usrval);
	}
    }
  if (stp_get_output_type(print) == OUTPUT_GRAY &&
      (stp_get_output_type(user) == OUTPUT_COLOR ||
       stp_get_output_type(user) == OUTPUT_RAW_CMYK))
    stp_set_output_type(user, OUTPUT_GRAY);
}

void
stp_set_printer_defaults(stp_vars_t v, const stp_printer_t p)
{
  stp_parameter_list_t *params;
  int count;
  int i;
  stp_parameter_t desc;
  stp_set_driver(v, stp_printer_get_driver(p));
  params = stp_list_parameters(v);
  count = stp_parameter_list_count(params);
  for (i = 0; i < count; i++)
    {
      const stp_parameter_t *p = stp_parameter_list_param(params, i);
      if (p->type == STP_PARAMETER_TYPE_STRING_LIST)
	{
	  stp_describe_parameter(v, p->name, &desc);
	  stp_set_string_parameter(v, p->name, desc.deflt.str);
	  stp_string_list_free(desc.bounds.str);
	}
    }
}

void
stp_describe_internal_parameter(const stp_vars_t v, const char *name,
				stp_parameter_t *description)
{
  stp_parameter_list_t list;
  const stp_parameter_t *param;
  if (strcmp(name, "DitherAlgorithm") == 0)
    {
      stp_fill_parameter_settings(description, name);
      description->bounds.str = stp_string_list_allocate();
      stp_dither_algorithms(description->bounds.str);
      description->deflt.str =
	stp_string_list_param(description->bounds.str, 0)->name;
      return;
    }
  list = stp_list_parameters(v);
  param = stp_parameter_find(list, name);
  stp_parameter_list_destroy(list);

  if (param && param->type == STP_PARAMETER_TYPE_DOUBLE &&
      strcmp(name, param->name) == 0)
    {
      stp_fill_parameter_settings(description, name);
      if (description->type == STP_PARAMETER_TYPE_DOUBLE)
	{
	  description->bounds.dbl.lower =
	    stp_get_float_parameter(stp_minimum_settings(), name);
	  description->bounds.dbl.upper =
	    stp_get_float_parameter(stp_maximum_settings(), name);
	  description->deflt.dbl =
	    stp_get_float_parameter(stp_default_settings(), name);
	}
      return;
    }
  description->type = STP_PARAMETER_TYPE_INVALID;
}

static const char *
param_namefunc(const stp_list_item_t *item)
{
  const stp_parameter_t *param =
    (const stp_parameter_t *) stp_list_item_get_data(item);
  return param->name;
}

static const char *
param_longnamefunc(const stp_list_item_t *item)
{
  const stp_parameter_t *param =
    (const stp_parameter_t *) stp_list_item_get_data(item);
  return param->text;
}

static stp_list_t *
stp_parameter_list_create(void)
{
  stp_list_t *ret = stp_list_create();
  stp_list_set_namefunc(ret, param_namefunc);
  stp_list_set_long_namefunc(ret, param_longnamefunc);
  return ret;
}

stp_parameter_list_t
stp_list_parameters(const stp_vars_t v)
{
  stp_list_t *ret = stp_parameter_list_create();
  int i;
  for (i = 0; i < (sizeof(global_parameters) / sizeof(const stp_parameter_t));
       i++)
    stp_list_item_create(ret, NULL, (void *) &(global_parameters[i]));
  return ret;
}

size_t
stp_parameter_list_count(const stp_parameter_list_t list)
{
  stp_list_t *ilist = (stp_list_t *)list;
  return stp_list_get_length(ilist);
}

const stp_parameter_t *
stp_parameter_find(const stp_parameter_list_t list, const char *name)
{
  stp_list_t *ilist = (stp_list_t *)list;
  stp_list_item_t *item = stp_list_get_item_by_name(ilist, name);
  if (item)
    return (stp_parameter_t *) stp_list_item_get_data(item);
  else
    return NULL;
}

const stp_parameter_t *
stp_parameter_list_param(const stp_parameter_list_t list, size_t item)
{
  stp_list_t *ilist = (stp_list_t *)list;
  if (item >= stp_list_get_length(ilist))
    return NULL;
  else
    return (stp_parameter_t *)
      stp_list_item_get_data(stp_list_get_item_by_index(ilist, item));
}

void
stp_parameter_list_destroy(stp_parameter_list_t list)
{
  stp_list_destroy((stp_list_t *)list);
}

stp_parameter_list_t
stp_parameter_list_copy(const stp_parameter_list_t list)
{
  stp_list_t *ret = stp_parameter_list_create();
  int i;
  size_t count = stp_parameter_list_count(list);
  for (i = 0; i < count; i++)
    stp_list_item_create(ret, NULL, (void *)stp_parameter_list_param(list, i));
  return ret;
}

void
stp_parameter_list_append(stp_parameter_list_t list,
			  const stp_parameter_list_t append)
{
  int i;
  stp_list_t *ilist = (stp_list_t *)list;
  size_t count = stp_parameter_list_count(append);
  for (i = 0; i < count; i++)
    stp_list_item_create(ilist, NULL,
			 (void *) stp_parameter_list_param(list, i));
}
