/*
 * "$Id: print-papers.c,v 1.30 2003/11/25 02:46:44 rlk Exp $"
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
#include <string.h>
#include <stdlib.h>
#include "xml.h"

static stpi_list_t *paper_list = NULL;

static void
stpi_paper_freefunc(void *item)
{
  stp_papersize_t *paper = (stp_papersize_t *) (item);
  SAFE_FREE(paper->name);
  SAFE_FREE(paper->text);
  SAFE_FREE(paper->comment);
  SAFE_FREE(paper);
}

static const char *
stpi_paper_namefunc(const void *item)
{
  const stp_papersize_t *paper = (const stp_papersize_t *) (item);
  return paper->name;
}

static const char *
stpi_paper_long_namefunc(const void *item)
{
  const stp_papersize_t *paper = (const stp_papersize_t *) (item);
  return paper->text;
}

static int
stpi_paper_list_init(void)
{
  if (paper_list)
    stpi_list_destroy(paper_list);
  paper_list = stpi_list_create();
  stpi_list_set_freefunc(paper_list, stpi_paper_freefunc);
  stpi_list_set_namefunc(paper_list, stpi_paper_namefunc);
  stpi_list_set_long_namefunc(paper_list, stpi_paper_long_namefunc);
  /* stpi_list_set_sortfunc(stpi_paper_sortfunc); */

  return 0;
}

static inline void
check_paperlist(void)
{
  if (paper_list == NULL)
    {
      stpi_xml_parse_file_named("papers.xml");
      if (paper_list == NULL)
	{
	  stpi_erprintf("No papers found: is STP_MODULE_PATH correct?\n");
	  stpi_paper_list_init();
	}
    }
}

static int
stpi_paper_create(stp_papersize_t *p)
{
  stpi_list_item_t *paper_item;

  if (paper_list == NULL)
    {
      stpi_paper_list_init();
      if (stpi_debug_level & STPI_DBG_PAPER)
	stpi_erprintf("stpi_paper_create(): initialising paper_list...\n");
    }

  /* Check the paper does not already exist */
  paper_item = stpi_list_get_start(paper_list);
  while (paper_item)
    {
      const stp_papersize_t *ep =
	(const stp_papersize_t *) stpi_list_item_get_data(paper_item);
      if (ep && !strcmp(p->name, ep->name))
	{
	  stpi_paper_freefunc(p);
	  return 1;
	}
      paper_item = stpi_list_item_next(paper_item);
    }

  /* Add paper to list */
  stpi_list_item_create(paper_list, NULL, (void *) p);

  return 0;
}

static int
stpi_paper_destroy(stp_papersize_t *p)
{
  stpi_list_item_t *paper_item;
  check_paperlist();

  /* Check if paper exists */
  paper_item = stpi_list_get_start(paper_list);
  while (paper_item)
    {
      const stp_papersize_t *ep = (const stp_papersize_t *)
	stpi_list_item_get_data(paper_item);
      if (ep && !strcmp(p->name, ep->name))
	{
	  stpi_list_item_destroy (paper_list, paper_item);
	  return 0;
	}
      paper_item = stpi_list_item_next(paper_item);
    }
  /* Paper did not exist */
  return 1;
}


int
stp_known_papersizes(void)
{
  check_paperlist();
  return stpi_list_get_length(paper_list);
}

const stp_papersize_t *
stp_get_papersize_by_name(const char *name)
{
  stpi_list_item_t *paper;

  check_paperlist();
  paper = stpi_list_get_item_by_name(paper_list, name);
  if (!paper)
    return NULL;
  else
    return (const stp_papersize_t *) stpi_list_item_get_data(paper);
}

const stp_papersize_t *
stp_get_papersize_by_index(int idx)
{
  stpi_list_item_t *paper;

  check_paperlist();
  paper = stpi_list_get_item_by_index(paper_list, idx);
  if (!paper)
    return NULL;
  else
    return (const stp_papersize_t *) stpi_list_item_get_data(paper);
}

static int
paper_size_mismatch(int l, int w, const stp_papersize_t *val)
{
  int hdiff = abs(l - (int) val->height);
  int vdiff = abs(w - (int) val->width);
  return hdiff + vdiff;
}

const stp_papersize_t *
stp_get_papersize_by_size(int l, int w)
{
  int score = INT_MAX;
  const stp_papersize_t *ref = NULL;
  const stp_papersize_t *val = NULL;
  int i;
  int sizes = stp_known_papersizes();
  for (i = 0; i < sizes; i++)
    {
      val = stp_get_papersize_by_index(i);

      if (val->width == w && val->height == l)
	return val;
      else
	{
	  int myscore = paper_size_mismatch(l, w, val);
	  if (myscore < score && myscore < 20)
	    {
	      ref = val;
	      score = myscore;
	    }
	}
    }
  return ref;
}

void
stpi_default_media_size(stp_const_vars_t v,	/* I */
			int  *width,		/* O - Width in points */
			int  *height) 		/* O - Height in points */
{
  if (stp_get_page_width(v) > 0 && stp_get_page_height(v) > 0)
    {
      *width = stp_get_page_width(v);
      *height = stp_get_page_height(v);
    }
  else
    {
      const char *page_size = stp_get_string_parameter(v, "PageSize");
      const stp_papersize_t *papersize = NULL;
      if (page_size)
	papersize = stp_get_papersize_by_name(page_size);
      if (!papersize)
	{
	  *width = 1;
	  *height = 1;
	}
      else
	{
	  *width = papersize->width;
	  *height = papersize->height;
	}
      if (*width == 0)
	*width = 612;
      if (*height == 0)
	*height = 792;
    }
}

/*
 * Process the <paper> node.
 */
static stp_papersize_t *
stpi_xml_process_paper(mxml_node_t *paper) /* The paper node */
{
  mxml_node_t *prop;                              /* Temporary node pointer */
  const char *stmp;                                /* Temporary string */
  /* props[] (unused) is the correct tag sequence */
  /*  const char *props[] =
    {
      "name",
      "description",
      "width",
      "height",
      "left",
      "right",
      "bottom",
      "top",
      "unit",
      NULL
      };*/
  stp_papersize_t *outpaper;   /* Generated paper */
  int
    id = 0,			/* Check id is present */
    name = 0,			/* Check name is present */
    height = 0,			/* Check height is present */
    width = 0,			/* Check width is present */
    left = 0,			/* Check left is present */
    right = 0,			/* Check right is present */
    bottom = 0,			/* Check bottom is present */
    top = 0,			/* Check top is present */
    unit = 0;			/* Check unit is present */

  if (stpi_debug_level & STPI_DBG_XML)
    {
      stmp = stpi_mxmlElementGetAttr(paper, (const char*) "name");
      stpi_erprintf("stpi_xml_process_paper: name: %s\n", stmp);
    }

  outpaper = stpi_zalloc(sizeof(stp_papersize_t));
  if (!outpaper)
    return NULL;

  outpaper->name = stpi_strdup(stpi_mxmlElementGetAttr(paper, "name"));

  outpaper->top = 0;
  outpaper->left = 0;
  outpaper->bottom = 0;
  outpaper->right = 0;
  if (outpaper->name)
    id = 1;

  prop = paper->child;
  while(prop)
    {
      if (prop->type == MXML_ELEMENT)
	{
	  const char *prop_name = prop->value.element.name;
      
	  if (!strcmp(prop_name, "description"))
	    {
	      outpaper->text = stpi_strdup(stpi_mxmlElementGetAttr(prop, "value"));
	      name = 1;
	    }
	  if (!strcmp(prop_name, "comment"))
	    outpaper->comment = stpi_strdup(stpi_mxmlElementGetAttr(prop, "value"));
	  if (!strcmp(prop_name, "width"))
	    {
	      stmp = stpi_mxmlElementGetAttr(prop, "value");
	      if (stmp)
		{
		  outpaper->width = stpi_xmlstrtoul(stmp);
		  width = 1;
		}
	    }
	  if (!strcmp(prop_name, "height"))
	    {
	      stmp = stpi_mxmlElementGetAttr(prop, "value");
	      if (stmp)
		{
		  outpaper->height = stpi_xmlstrtoul(stmp);
		  height = 1;
		}
	    }
	  if (!strcmp(prop_name, "left"))
	    {
	      stmp = stpi_mxmlElementGetAttr(prop, "value");
	      outpaper->left = stpi_xmlstrtoul(stmp);
	      left = 1;
	    }
	  if (!strcmp(prop_name, "right"))
	    {
	      stmp = stpi_mxmlElementGetAttr(prop, "value");
	      outpaper->right = stpi_xmlstrtoul(stmp);
	      right = 1;
	    }
	  if (!strcmp(prop_name, "bottom"))
	    {
	      stmp = stpi_mxmlElementGetAttr(prop, "value");
	      outpaper->bottom = stpi_xmlstrtoul(stmp);
	      bottom = 1;
	    }
	  if (!strcmp(prop_name, "top"))
	    {
	      stmp = stpi_mxmlElementGetAttr(prop, "value");
	      outpaper->top = stpi_xmlstrtoul(stmp);
	      top = 1;
	    }
	  if (!strcmp(prop_name, "unit"))
	    {
	      stmp = stpi_mxmlElementGetAttr(prop, "value");
	      if (stmp)
		{
		  if (!strcmp(stmp, "english"))
		    outpaper->paper_unit = PAPERSIZE_ENGLISH_STANDARD;
		  else if (!strcmp(stmp, "english-extended"))
		    outpaper->paper_unit = PAPERSIZE_ENGLISH_EXTENDED;
		  else if (!strcmp(stmp, "metric"))
		    outpaper->paper_unit = PAPERSIZE_METRIC_STANDARD;
		  else if (!strcmp(stmp, "metric-extended"))
		    outpaper->paper_unit = PAPERSIZE_METRIC_EXTENDED;
		  /* Default unit */
		  else
		    outpaper->paper_unit = PAPERSIZE_METRIC_EXTENDED;
		  unit = 1;
		}
	    }
	}
      prop = prop->next;
    }
  if (id && name && width && height && unit) /* Margins are optional */
    return outpaper;
  stpi_free(outpaper);
  outpaper = NULL;
  return NULL;
}

/*
 * Parse the <paperdef> node.
 */
static int
stpi_xml_process_paperdef(mxml_node_t *paperdef, const char *file) /* The paperdef node */
{
  mxml_node_t *paper;                           /* paper node pointer */
  stp_papersize_t *outpaper;         /* Generated paper */

  paper = paperdef->child;
  while (paper)
    {
      if (paper->type == MXML_ELEMENT)
	{
	  const char *paper_name = paper->value.element.name;
	  if (!strcmp(paper_name, "paper"))
	    {
	      outpaper = stpi_xml_process_paper(paper);
	      if (outpaper)
		stpi_paper_create(outpaper);
	    }
	}
      paper = paper->next;
    }
  return 1;
}

void
stpi_init_paper(void)
{
  stpi_register_xml_parser("paperdef", stpi_xml_process_paperdef);
}