/*
 * "$Id: rastertoprinter.c,v 1.64 2003/06/20 23:34:58 rlk Exp $"
 *
 *   GIMP-print based raster filter for the Common UNIX Printing System.
 *
 *   Copyright 1993-2003 by Easy Software Products.
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License,
 *   version 2, as published by the Free Software Foundation.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, please contact Easy Software
 *   Products at:
 *
 *       Attn: CUPS Licensing Information
 *       Easy Software Products
 *       44141 Airport View Drive, Suite 204
 *       Hollywood, Maryland 20636-3111 USA
 *
 *       Voice: (301) 373-9603
 *       EMail: cups-info@cups.org
 *         WWW: http://www.cups.org
 *
 * Contents:
 *
 *   main()                    - Main entry and processing of driver.
 *   cups_writefunc()          - Write data to a file...
 *   cancel_job()              - Cancel the current job...
 *   Image_bpp()               - Return the bytes-per-pixel of an image.
 *   Image_get_appname()       - Get the application we are running.
 *   Image_get_row()           - Get one row of the image.
 *   Image_height()            - Return the height of an image.
 *   Image_init()              - Initialize an image.
 *   Image_note_progress()     - Notify the user of our progress.
 *   Image_progress_conclude() - Close the progress display.
 *   Image_progress_init()     - Initialize progress display.
 *   Image_width()             - Return the width of an image.
 */

/*
 * Include necessary headers...
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <cups/cups.h>
#include <cups/ppd.h>
#include <cups/raster.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#ifdef INCLUDE_GIMP_PRINT_H
#include INCLUDE_GIMP_PRINT_H
#else
#include <gimp-print/gimp-print.h>
#endif
#include "../../lib/libprintut.h"
#include "gimp-print-cups.h"

/* Solaris with gcc has problems because gcc's limits.h doesn't #define */
/* this */
#ifndef CHAR_BIT
#define CHAR_BIT 8
#endif

/*
 * Structure for page raster data...
 */

typedef struct
{
  cups_raster_t		*ras;		/* Raster stream to read from */
  int			page;		/* Current page number */
  int			row;		/* Current row number */
  int			left;
  int			right;
  int			bottom;
  int			top;
  int			width;
  int			height;
  cups_page_header_t	header;		/* Page header from file */
} cups_image_t;

static void	cups_writefunc(void *file, const char *buf, size_t bytes);
static void	cancel_job(int sig);
static const char *Image_get_appname(stp_image_t *image);
static void	 Image_progress_conclude(stp_image_t *image);
static void	Image_note_progress(stp_image_t *image,
				    double current, double total);
static void	Image_progress_init(stp_image_t *image);
static stp_image_status_t Image_get_row(stp_image_t *image,
					unsigned char *data,
					size_t byte_limit, int row);
static int	Image_height(stp_image_t *image);
static int	Image_width(stp_image_t *image);
static int	Image_bpp(stp_image_t *image);
static void	Image_init(stp_image_t *image);

static stp_image_t theImage =
{
  Image_init,
  NULL,				/* reset */
  Image_bpp,
  Image_width,
  Image_height,
  Image_get_row,
  Image_get_appname,
  Image_progress_init,
  Image_note_progress,
  Image_progress_conclude,
  NULL
};

static volatile stp_image_status_t Image_status = STP_IMAGE_STATUS_OK;

static void
set_special_parameter(stp_vars_t v, const char *name, int choice)
{
  stp_parameter_t desc;
  stp_describe_parameter(v, name, &desc);
  if (desc.p_type == STP_PARAMETER_TYPE_STRING_LIST)
    {
      if (choice >= stp_string_list_count(desc.bounds.str))
	fprintf(stderr, "ERROR: Unable to set %s!\n", name);
      else
	stp_set_string_parameter
	  (v, name, stp_string_list_param(desc.bounds.str, choice)->name);
    }
  stp_parameter_description_free(&desc);
}

static void
print_debug_block(cups_image_t *cups)
{
  fprintf(stderr, "DEBUG: StartPage...\n");
  fprintf(stderr, "DEBUG: MediaClass = \"%s\"\n", cups->header.MediaClass);
  fprintf(stderr, "DEBUG: MediaColor = \"%s\"\n", cups->header.MediaColor);
  fprintf(stderr, "DEBUG: MediaType = \"%s\"\n", cups->header.MediaType);
  fprintf(stderr, "DEBUG: OutputType = \"%s\"\n", cups->header.OutputType);

  fprintf(stderr, "DEBUG: AdvanceDistance = %d\n", cups->header.AdvanceDistance);
  fprintf(stderr, "DEBUG: AdvanceMedia = %d\n", cups->header.AdvanceMedia);
  fprintf(stderr, "DEBUG: Collate = %d\n", cups->header.Collate);
  fprintf(stderr, "DEBUG: CutMedia = %d\n", cups->header.CutMedia);
  fprintf(stderr, "DEBUG: Duplex = %d\n", cups->header.Duplex);
  fprintf(stderr, "DEBUG: HWResolution = [ %d %d ]\n", cups->header.HWResolution[0],
	  cups->header.HWResolution[1]);
  fprintf(stderr, "DEBUG: ImagingBoundingBox = [ %d %d %d %d ]\n",
	  cups->header.ImagingBoundingBox[0], cups->header.ImagingBoundingBox[1],
	  cups->header.ImagingBoundingBox[2], cups->header.ImagingBoundingBox[3]);
  fprintf(stderr, "DEBUG: InsertSheet = %d\n", cups->header.InsertSheet);
  fprintf(stderr, "DEBUG: Jog = %d\n", cups->header.Jog);
  fprintf(stderr, "DEBUG: LeadingEdge = %d\n", cups->header.LeadingEdge);
  fprintf(stderr, "DEBUG: Margins = [ %d %d ]\n", cups->header.Margins[0],
	  cups->header.Margins[1]);
  fprintf(stderr, "DEBUG: ManualFeed = %d\n", cups->header.ManualFeed);
  fprintf(stderr, "DEBUG: MediaPosition = %d\n", cups->header.MediaPosition);
  fprintf(stderr, "DEBUG: MediaWeight = %d\n", cups->header.MediaWeight);
  fprintf(stderr, "DEBUG: MirrorPrint = %d\n", cups->header.MirrorPrint);
  fprintf(stderr, "DEBUG: NegativePrint = %d\n", cups->header.NegativePrint);
  fprintf(stderr, "DEBUG: NumCopies = %d\n", cups->header.NumCopies);
  fprintf(stderr, "DEBUG: Orientation = %d\n", cups->header.Orientation);
  fprintf(stderr, "DEBUG: OutputFaceUp = %d\n", cups->header.OutputFaceUp);
  fprintf(stderr, "DEBUG: PageSize = [ %d %d ]\n", cups->header.PageSize[0],
	  cups->header.PageSize[1]);
  fprintf(stderr, "DEBUG: Separations = %d\n", cups->header.Separations);
  fprintf(stderr, "DEBUG: TraySwitch = %d\n", cups->header.TraySwitch);
  fprintf(stderr, "DEBUG: Tumble = %d\n", cups->header.Tumble);
  fprintf(stderr, "DEBUG: cupsWidth = %d\n", cups->header.cupsWidth);
  fprintf(stderr, "DEBUG: cupsHeight = %d\n", cups->header.cupsHeight);
  fprintf(stderr, "DEBUG: cupsMediaType = %d\n", cups->header.cupsMediaType);
  fprintf(stderr, "DEBUG: cupsBitsPerColor = %d\n", cups->header.cupsBitsPerColor);
  fprintf(stderr, "DEBUG: cupsBitsPerPixel = %d\n", cups->header.cupsBitsPerPixel);
  fprintf(stderr, "DEBUG: cupsBytesPerLine = %d\n", cups->header.cupsBytesPerLine);
  fprintf(stderr, "DEBUG: cupsColorOrder = %d\n", cups->header.cupsColorOrder);
  fprintf(stderr, "DEBUG: cupsColorSpace = %d\n", cups->header.cupsColorSpace);
  fprintf(stderr, "DEBUG: cupsCompression = %d\n", cups->header.cupsCompression);
  fprintf(stderr, "DEBUG: cupsRowCount = %d\n", cups->header.cupsRowCount);
  fprintf(stderr, "DEBUG: cupsRowFeed = %d\n", cups->header.cupsRowFeed);
  fprintf(stderr, "DEBUG: cupsRowStep = %d\n", cups->header.cupsRowStep);
}

static stp_vars_t
initialize_page(cups_image_t *cups, stp_const_vars_t default_settings)
{
  int i;
  const stp_papersize_t	*size;		/* Paper size */
  stp_parameter_list_t params;
  int nparams;
  stp_vars_t v = stp_vars_create_copy(default_settings);

  stp_set_page_width(v, cups->header.PageSize[0]);
  stp_set_page_height(v, cups->header.PageSize[1]);
  stp_set_outfunc(v, cups_writefunc);
  stp_set_errfunc(v, cups_writefunc);
  stp_set_outdata(v, stdout);
  stp_set_errdata(v, stderr);

  switch (cups->header.cupsColorSpace)
    {
    case CUPS_CSPACE_W :
      stp_set_output_type(v, OUTPUT_GRAY);
      break;
    case CUPS_CSPACE_K :
      stp_set_output_type(v, OUTPUT_GRAY);
      break;
    case CUPS_CSPACE_RGB :
      stp_set_output_type(v, OUTPUT_COLOR);
      break;
    case CUPS_CSPACE_CMYK :
      stp_set_output_type(v, OUTPUT_RAW_CMYK);
      break;
    default :
      fprintf(stderr, "ERROR: Bad colorspace %d!",
	      cups->header.cupsColorSpace);
      break;
    }

  set_special_parameter(v, "Resolution", cups->header.cupsCompression);

  stp_set_string_parameter(v, "InputSlot", cups->header.MediaClass);
  stp_set_string_parameter(v, "MediaType", cups->header.MediaType);

  fprintf(stderr, "DEBUG: PageSize = %dx%d\n", cups->header.PageSize[0],
	  cups->header.PageSize[1]);

  if ((size = stp_get_papersize_by_size(cups->header.PageSize[1],
					cups->header.PageSize[0])) != NULL)
    stp_set_string_parameter(v, "PageSize", size->name);
  else
    fprintf(stderr, "ERROR: Unable to get media size!\n");


  params = stp_get_parameter_list(v);
  nparams = stp_parameter_list_count(params);
  for (i = 0; i < nparams; i++)
    {
      const stp_parameter_t *p = stp_parameter_list_param(params, i);
      switch (p->p_type)
	{
	case STP_PARAMETER_TYPE_STRING_LIST:
	  fprintf(stderr, "DEBUG: stp_get_string %s(v) |%s| %d\n",
		  p->name, stp_get_string_parameter(v, p->name) ?
		  stp_get_string_parameter(v, p->name) : "NULL",
		  stp_get_string_parameter_active(v, p->name));
	  break;
	case STP_PARAMETER_TYPE_DOUBLE:
	  fprintf(stderr, "DEBUG: stp_get_float %s(v) |%.3f| %d\n",
		  p->name, stp_get_float_parameter(v, p->name),
		  stp_get_float_parameter_active(v, p->name));
	  break;
	case STP_PARAMETER_TYPE_INT:
	  fprintf(stderr, "DEBUG: stp_get_int %s(v) |%.d| %d\n",
		  p->name, stp_get_int_parameter(v, p->name),
		  stp_get_int_parameter_active(v, p->name));
	  break;
	case STP_PARAMETER_TYPE_BOOLEAN:
	  fprintf(stderr, "DEBUG: stp_get_boolean %s(v) |%.d| %d\n",
		  p->name, stp_get_boolean_parameter(v, p->name),
		  stp_get_boolean_parameter_active(v, p->name));
	  break;
	  /*
	   * We don't handle raw, curve, or filename arguments.
	   */
	default:
	  break;
	}
    }
  stp_parameter_list_free(params);
  stp_set_job_mode(v, STP_JOB_MODE_JOB);
  fprintf(stderr, "DEBUG: stp_get_driver(v) |%s|\n", stp_get_driver(v));
  fprintf(stderr, "DEBUG: stp_get_output_type(v) |%d|\n", stp_get_output_type(v));
  fprintf(stderr, "DEBUG: stp_get_left(v) |%d|\n", stp_get_left(v));
  fprintf(stderr, "DEBUG: stp_get_top(v) |%d|\n", stp_get_top(v));
  fprintf(stderr, "DEBUG: stp_get_page_width(v) |%d|\n", stp_get_page_width(v));
  fprintf(stderr, "DEBUG: stp_get_page_height(v) |%d|\n", stp_get_page_height(v));
  fprintf(stderr, "DEBUG: stp_get_input_color_model(v) |%d|\n", stp_get_input_color_model(v));
  stp_get_media_size(v, &(cups->width), &(cups->height));
  stp_get_imageable_area(v, &(cups->left), &(cups->right),
			 &(cups->bottom), &(cups->top));
  fprintf(stderr, "DEBUG: GIMP-PRINT %d %d %d  %d %d %d\n",
	  cups->width, cups->left, cups->right, cups->height, cups->top, cups->bottom);
  stp_set_width(v, cups->right - cups->left);
  stp_set_height(v, cups->bottom - cups->top);
  stp_set_left(v, cups->left);
  stp_set_top(v, cups->top);
  cups->right = cups->width - cups->right;
  cups->width = cups->width - cups->left - cups->right;
  cups->width = cups->header.HWResolution[0] * cups->width / 72;
  cups->left = cups->header.HWResolution[0] * cups->left / 72;
  cups->right = cups->header.HWResolution[0] * cups->right / 72;

  cups->bottom = cups->height - cups->bottom;
  cups->height = cups->height - cups->top - cups->bottom;
  cups->height = cups->header.HWResolution[1] * cups->height / 72;
  cups->top = cups->header.HWResolution[1] * cups->top / 72;
  cups->bottom = cups->header.HWResolution[1] * cups->bottom / 72;
  fprintf(stderr, "DEBUG: GIMP-PRINT %d %d %d  %d %d %d\n",
	  cups->width, cups->left, cups->right, cups->height, cups->top, cups->bottom);

  return v;
}

static void
purge_excess_data(cups_image_t *cups)
{
  char *buffer = xmalloc(cups->header.cupsBytesPerLine);
  if (buffer)
    while (cups->row < cups->header.cupsHeight)
      {
	cupsRasterReadPixels(cups->ras, (unsigned char *)buffer,
			     cups->header.cupsBytesPerLine);
	cups->row ++;
      }
  free(buffer);
}

static void
set_all_options(stp_vars_t v, cups_option_t *options, int num_options,
		ppd_file_t *ppd)
{
  stp_parameter_list_t params = stp_get_parameter_list(v);
  int nparams = stp_parameter_list_count(params);
  int i;
  for (i = 0; i < nparams; i++)
    {
      const stp_parameter_t *param = stp_parameter_list_param(params, i);
      stp_parameter_t desc;
      char *ppd_option_name = xmalloc(strlen(param->name) + 8);	/* StpFineFOO\0 */

      const char *val;		/* CUPS option value */
      ppd_option_t *ppd_option;
      stp_describe_parameter(v, param->name, &desc);
      if (desc.p_type == STP_PARAMETER_TYPE_DOUBLE)
	{
	  sprintf(ppd_option_name, "Stp%s", desc.name);
	  val = cupsGetOption(ppd_option_name, num_options, options);
	  if (!val)
	    {
	      ppd_option = ppdFindOption(ppd, ppd_option_name);
	      if (ppd_option)
		val = ppd_option->defchoice;
	    }
	  if (val && strcmp(val, "DEFAULT") != 0)
	    {
	      double coarse_val = atof(val) * 0.001;
	      double fine_val = 0;
	      sprintf(ppd_option_name, "StpFine%s", desc.name);
	      val = cupsGetOption(ppd_option_name, num_options, options);
	      if (!val)
		{
		  ppd_option = ppdFindOption(ppd, ppd_option_name);
		  if (ppd_option)
		    val = ppd_option->defchoice;
		}
	      if (val && strcmp(val, "DEFAULT") != 0)
		fine_val = atof(val) * 0.001;
	      fprintf(stderr, "DEBUG: Gimp-Print set float %s to %f + %f\n",
		      desc.name, coarse_val, fine_val);
	      fine_val += coarse_val;
	      if (fine_val > desc.bounds.dbl.upper)
		fine_val = desc.bounds.dbl.upper;
	      stp_set_float_parameter(v, desc.name, fine_val);
	    }
	}
      else
	{
	  sprintf(ppd_option_name, "Stp%s", desc.name);
	  val = cupsGetOption(ppd_option_name, num_options, options);
	  if (!val)
	    {
	      ppd_option = ppdFindOption(ppd, ppd_option_name);
	      if (ppd_option)
		val = ppd_option->defchoice;
	    }
	  if (val && strcmp(val, "DEFAULT") != 0)
	    {
	      switch (desc.p_type)
		{
		case STP_PARAMETER_TYPE_STRING_LIST:
		  fprintf(stderr, "DEBUG: Gimp-Print set string %s to %s\n",
			  desc.name, val);
		  stp_set_string_parameter(v, desc.name, val);
		  break;
		case STP_PARAMETER_TYPE_INT:
		  fprintf(stderr, "DEBUG: Gimp-Print set int %s to %s\n",
			  desc.name, val);
		  stp_set_int_parameter(v, desc.name, atoi(val));
		  break;
		case STP_PARAMETER_TYPE_BOOLEAN:
		  fprintf(stderr, "DEBUG: Gimp-Print set bool %s to %s\n",
			  desc.name, val);
		  stp_set_boolean_parameter
		    (v, desc.name, strcmp(val, "True") == 0 ? 1 : 0);
		  break;
		case STP_PARAMETER_TYPE_CURVE: /* figure this out later... */
		case STP_PARAMETER_TYPE_FILE: /* Probably not, security hole */
		case STP_PARAMETER_TYPE_RAW: /* figure this out later, too */
		  break;
		default:
		  break;
		}
	    }	  
	}
      stp_parameter_description_free(&desc);
      free(ppd_option_name);
    }
  stp_parameter_list_free(params);
}

/*
 * 'main()' - Main entry and processing of driver.
 */

int					/* O - Exit status */
main(int  argc,				/* I - Number of command-line arguments */
     char *argv[])			/* I - Command-line arguments */
{
  int			fd;		/* File descriptor */
  cups_image_t		cups;		/* CUPS image */
  const char		*ppdfile;	/* PPD environment variable */
  ppd_file_t		*ppd;		/* PPD file */
  stp_const_printer_t	printer;	/* Printer driver */
  int			num_options;	/* Number of CUPS options */
  cups_option_t		*options;	/* CUPS options */
  stp_vars_t		v = NULL;
  stp_vars_t		default_settings = stp_vars_create();
  int			initialized_job = 0;

 /*
  * Initialise libgimpprint
  */

  theImage.rep = &cups;

  stp_init();

 /*
  * Check for valid arguments...
  */

  if (argc < 6 || argc > 7)
  {
   /*
    * We don't have the correct number of arguments; write an error message
    * and return.
    */

    fputs("ERROR: rastertoprinter job-id user title copies options [file]\n", stderr);
    return (1);
  }

 /*
  * Get the PPD file...
  */

  if ((ppdfile = getenv("PPD")) == NULL)
  {
    fputs("ERROR: Fatal error: PPD environment variable not set!\n", stderr);
    return (1);
  }

  if ((ppd = ppdOpenFile(ppdfile)) == NULL)
  {
    fprintf(stderr, "ERROR: Fatal error: Unable to load PPD file \"%s\"!\n",
            ppdfile);
    return (1);
  }

  if (ppd->modelname == NULL)
  {
    fprintf(stderr, "ERROR: Fatal error: No ModelName attribute in PPD file \"%s\"!\n",
            ppdfile);
    ppdClose(ppd);
    return (1);
  }

 /*
  * Get the STP options, if any...
  */

  fprintf(stderr, "DEBUG: Gimp-Print options are %s\n", argv[5]);

  num_options = cupsParseOptions(argv[5], 0, &options);

 /*
  * Figure out which driver to use...
  */

  if ((printer = stp_get_printer_by_long_name(ppd->modelname)) == NULL)
    if ((printer = stp_get_printer_by_driver(ppd->modelname)) == NULL)
    {
      fprintf(stderr, "ERROR: Fatal error: Unable to find driver named \"%s\"!\n",
              ppd->modelname);
      ppdClose(ppd);
      return (1);
    }

 /*
  * Open the page stream...
  */

  if (argc == 7)
  {
    if ((fd = open(argv[6], O_RDONLY)) == -1)
    {
      perror("ERROR: Unable to open raster file - ");
      sleep(1);
      return (1);
    }
  }
  else
    fd = 0;

  stp_set_printer_defaults(default_settings, printer);
  stp_set_float_parameter(default_settings, "AppGamma", 1.0);
  set_all_options(default_settings, options, num_options, ppd);
  stp_merge_printvars(default_settings, stp_printer_get_defaults(printer));
  ppdClose(ppd);

  cups.ras = cupsRasterOpen(fd, CUPS_RASTER_READ);

 /*
  * Process pages as needed...
  */

  cups.page = 0;

  /*
   * Read the first page header, which we need in order to set up
   * the page.
   */
  signal(SIGTERM, cancel_job);
  while (cupsRasterReadHeader(cups.ras, &cups.header))
    {
      /*
       * We don't know how many pages we're going to print, and
       * we need to call stp_end_job at the completion of the job.
       * Therefore, we need to keep v in scope after the termination
       * of the loop to permit calling stp_end_job then.  Therefore,
       * we have to free the previous page's stp_vars_t at the start
       * of the loop.
       */
      if (v)
	stp_vars_free(v);

      /*
       * Setup printer driver variables...
       */
      v = initialize_page(&cups, default_settings);
      stp_set_page_number(v, cups.page);
      cups.row = 0;
      fprintf(stderr, "PAGE: %d 1\n", cups.page);
      print_debug_block(&cups);
      if (!stp_verify(v))
        goto cups_abort;

      if (!initialized_job)
	{
	  stp_start_job(v, &theImage);
	  initialized_job = 1;
	}

      if (!stp_print(v, &theImage))
        goto cups_abort;

      fflush(stdout);

      /*
       * Purge any remaining bitmap data...
       */
      if (cups.row < cups.header.cupsHeight)
	purge_excess_data(&cups);
      cups.page ++;
    }
  if (v)
    {
      stp_end_job(v, &theImage);
      stp_vars_free(v);
    }
  cupsRasterClose(cups.ras);
  if (fd != 0)
    close(fd);
  fputs("INFO: Ready to print.\n", stderr);
  return 0;

cups_abort:
  fputs("ERROR: Invalid printer settings!\n", stderr);
  stp_end_job(v, &theImage);
  stp_vars_free(v);
  cupsRasterClose(cups.ras);
  if (fd != 0)
    close(fd);
  fputs("ERROR: No pages found!\n", stderr);
  return 1;
}


/*
 * 'cups_writefunc()' - Write data to a file...
 */

static void
cups_writefunc(void *file, const char *buf, size_t bytes)
{
  FILE *prn = (FILE *)file;
  fwrite(buf, 1, bytes, prn);
}


/*
 * 'cancel_job()' - Cancel the current job...
 */

void
cancel_job(int sig)			/* I - Signal */
{
  (void)sig;
  Image_status = STP_IMAGE_STATUS_ABORT;
}


/*
 * 'Image_bpp()' - Return the bytes-per-pixel of an image.
 */

static int				/* O - Bytes per pixel */
Image_bpp(stp_image_t *image)		/* I - Image */
{
  cups_image_t	*cups;		/* CUPS image */

  if ((cups = (cups_image_t *)(image->rep)) == NULL)
    return (0);

 /*
  * For now, we only support RGB and grayscale input from the
  * raster filters.
  */

  switch (cups->header.cupsColorSpace)
  {
    default :
        return (1);
    case CUPS_CSPACE_RGB :
        return (3);
    case CUPS_CSPACE_CMYK :
        return (4);
  }
}


/*
 * 'Image_get_appname()' - Get the application we are running.
 */

static const char *				/* O - Application name */
Image_get_appname(stp_image_t *image)		/* I - Image */
{
  (void)image;

  return ("CUPS 1.1.x driver based on GIMP-print");
}


/*
 * 'Image_get_row()' - Get one row of the image.
 */

static void
throwaway_data(int amount, cups_image_t *cups)
{
  unsigned char trash[4096];	/* Throwaway */
  int block_count = amount / 4096;
  int leftover = amount % 4096;
  while (block_count > 0)
    {
      cupsRasterReadPixels(cups->ras, trash, 4096);
      block_count--;
    }
  if (leftover)
    cupsRasterReadPixels(cups->ras, trash, leftover);
}

stp_image_status_t
Image_get_row(stp_image_t   *image,	/* I - Image */
	      unsigned char *data,	/* O - Row */
	      size_t	    byte_limit,	/* I - how many bytes in data */
	      int           row)	/* I - Row number (unused) */
{
  cups_image_t	*cups;			/* CUPS image */
  int		i;			/* Looping var */
  int 		bytes_per_line;
  int		margin;


  if ((cups = (cups_image_t *)(image->rep)) == NULL)
    return STP_IMAGE_STATUS_ABORT;
  bytes_per_line = cups->width * cups->header.cupsBitsPerPixel / CHAR_BIT;
  margin = cups->header.cupsBytesPerLine - bytes_per_line;

  if (cups->row < cups->header.cupsHeight)
  {
    fprintf(stderr, "DEBUG2: GIMP-PRINT reading %d %d\n",
	    bytes_per_line, cups->row);
    cupsRasterReadPixels(cups->ras, data, bytes_per_line);
    cups->row ++;
    if (margin > 0)
      {
	fprintf(stderr, "DEBUG2: GIMP-PRINT tossing right %d\n", margin);
	throwaway_data(margin, cups);
      }

   /*
    * Invert black data for monochrome output...
    */

    if (cups->header.cupsColorSpace == CUPS_CSPACE_K) {
      unsigned char *dp = data;
      for (i = bytes_per_line; i > 0; i --, dp++)
        *dp = ((1 << CHAR_BIT) - 1) - *dp;
    }
  }
  else
    {
      if (cups->header.cupsColorSpace == CUPS_CSPACE_CMYK)
	memset(data, 0, bytes_per_line);
      else
	memset(data, ((1 << CHAR_BIT) - 1), bytes_per_line);
    }
  return Image_status;
}


/*
 * 'Image_height()' - Return the height of an image.
 */

static int				/* O - Height in pixels */
Image_height(stp_image_t *image)	/* I - Image */
{
  cups_image_t	*cups;		/* CUPS image */


  if ((cups = (cups_image_t *)(image->rep)) == NULL)
    return (0);

  fprintf(stderr, "DEBUG: GIMP-PRINT: Image_height %d\n", cups->height);
  return (cups->height);
}


/*
 * 'Image_init()' - Initialize an image.
 */

static void
Image_init(stp_image_t *image)		/* I - Image */
{
  (void)image;
}

/*
 * 'Image_note_progress()' - Notify the user of our progress.
 */

void
Image_note_progress(stp_image_t *image,	/* I - Image */
		    double current,	/* I - Current progress */
		    double total)	/* I - Maximum progress */
{
  cups_image_t	*cups;		/* CUPS image */

  if ((cups = (cups_image_t *)(image->rep)) == NULL)
    return;

  fprintf(stderr, "INFO: Printing page %d, %.0f%%\n",
          cups->page, 100.0 * current / total);
}

/*
 * 'Image_progress_conclude()' - Close the progress display.
 */

static void
Image_progress_conclude(stp_image_t *image)	/* I - Image */
{
  cups_image_t	*cups;		/* CUPS image */


  if ((cups = (cups_image_t *)(image->rep)) == NULL)
    return;

  fprintf(stderr, "INFO: Finished page %d...\n", cups->page);
}

/*
 * 'Image_progress_init()' - Initialize progress display.
 */

static void
Image_progress_init(stp_image_t *image)/* I - Image */
{
  cups_image_t	*cups;		/* CUPS image */


  if ((cups = (cups_image_t *)(image->rep)) == NULL)
    return;

  fprintf(stderr, "INFO: Starting page %d...\n", cups->page);
}

/*
 * 'Image_width()' - Return the width of an image.
 */

static int				/* O - Width in pixels */
Image_width(stp_image_t *image)	/* I - Image */
{
  cups_image_t	*cups;		/* CUPS image */


  if ((cups = (cups_image_t *)(image->rep)) == NULL)
    return (0);

  fprintf(stderr, "DEBUG: GIMP-PRINT: Image_width %d\n", cups->width);
  return (cups->width);
}


/*
 * End of "$Id: rastertoprinter.c,v 1.64 2003/06/20 23:34:58 rlk Exp $".
 */
