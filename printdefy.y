/*
 * "$Id: printdefy.y,v 1.7 2000/08/05 16:50:35 rlk Exp $"
 *
 *   Parse printer definition pseudo-XML
 *
 *   Copyright 2000 Robert Krawitz (rlk@alum.mit.edu)
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

%{

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "printdef.h"

extern int mylineno;
printer_t thePrinter;
char *quotestrip(const char *i);
char *endstrip(const char *i);

extern int yylex(void);

#define ENONE 0
#define ECANON 1
#define EESCP2 2
#define EPCL 3
#define EPS 4

void
initialize_the_printer(const char *name, const char *driver)
{
  strncpy(thePrinter.printvars.output_to, name, 63);
  strncpy(thePrinter.printvars.driver, driver, 63);
  thePrinter.printvars.linear = ENONE;
  thePrinter.model = -1;
  thePrinter.printvars.brightness = 100;
  thePrinter.printvars.gamma = 1.0;
  thePrinter.printvars.contrast = 100;
  thePrinter.printvars.red = 100;
  thePrinter.printvars.green = 100;
  thePrinter.printvars.blue = 100;
  thePrinter.printvars.saturation = 1.0;
  thePrinter.printvars.density = 1.0;
}

void
output_the_printer(void)
{
  printf("  {\n");
  printf("    %s,\n", thePrinter.printvars.output_to);
  printf("    %s,\n", thePrinter.printvars.driver);
  printf("    %d,\n", thePrinter.model);
  switch (thePrinter.printvars.linear)
    {
    case 1:
      printf("    %s,\n", "canon_parameters");
      printf("    %s,\n", "default_media_size");
      printf("    %s,\n", "canon_imageable_area");
      printf("    %s,\n", "canon_limit");
      printf("    %s,\n", "canon_print");
      printf("    %s,\n", "canon_default_resolution");
      break;
    case 2:
      printf("    %s,\n", "escp2_parameters");
      printf("    %s,\n", "default_media_size");
      printf("    %s,\n", "escp2_imageable_area");
      printf("    %s,\n", "escp2_limit");
      printf("    %s,\n", "escp2_print");
      printf("    %s,\n", "escp2_default_resolution");
      break;
    case 3:
      printf("    %s,\n", "pcl_parameters");
      printf("    %s,\n", "default_media_size");
      printf("    %s,\n", "pcl_imageable_area");
      printf("    %s,\n", "pcl_limit");
      printf("    %s,\n", "pcl_print");
      printf("    %s,\n", "pcl_default_resolution");
      break;
    case 4:
      printf("    %s,\n", "ps_parameters");
      printf("    %s,\n", "ps_media_size");
      printf("    %s,\n", "ps_imageable_area");
      printf("    %s,\n", "ps_limit");
      printf("    %s,\n", "ps_print");
      printf("    %s,\n", "ps_default_resolution");
      break;
    default:
      printf("    %s,\n", "NULL");
      printf("    %s,\n", "NULL");
      printf("    %s,\n", "NULL");
      printf("    %s,\n", "NULL");
      printf("    %s,\n", "NULL");
      printf("    %s,\n", "NULL");
      printf("    %s,\n", "NULL");
      break;
    }
  printf("    {\n");
  printf("      \"\",\n");	/* output_to */
  printf("      \"\",\n");	/* driver */
  printf("      \"\",\n");	/* ppd_file */
  printf("      %d,\n", thePrinter.printvars.output_type);
  printf("      \"\",\n");	/* resolution */
  printf("      \"\",\n");	/* media_size */
  printf("      \"\",\n");	/* media_type */
  printf("      \"\",\n");	/* media_source */
  printf("      \"\",\n");	/* ink_type */
  printf("      \"\",\n");	/* dither_algorithm */
  printf("      %d,\n", thePrinter.printvars.brightness);
  printf("      1.0,\n");	/* scaling */
  printf("      -1,\n");	/* orientation */
  printf("      0,\n");		/* top */
  printf("      0,\n");		/* left */
  printf("      %.3f,\n", thePrinter.printvars.gamma);
  printf("      %d,\n", thePrinter.printvars.contrast);
  printf("      %d,\n", thePrinter.printvars.red);
  printf("      %d,\n", thePrinter.printvars.green);
  printf("      %d,\n", thePrinter.printvars.blue);
  printf("      0,\n");		/* linear */
  printf("      %.3f,\n", thePrinter.printvars.saturation);
  printf("      %.3f,\n", thePrinter.printvars.density);
  printf("    }\n");
  printf("  },\n");
}

extern int mylineno;
extern char* yytext;

int yyerror( const char *s )
{
	fprintf(stderr,"stdin:%d: %s before '%s'\n",mylineno,s,yytext);
	return 0;
}

%}

%token <ival> tINT
%token <dval> tDOUBLE
%token <sval> tSTRING tCLASS
%token tBEGIN tEND ASSIGN PRINTER NAME DRIVER COLOR NOCOLOR MODEL
%token LANGUAGE BRIGHTNESS GAMMA CONTRAST
%token RED GREEN BLUE SATURATION DENSITY ENDPRINTER VALUE

%start Printers

%%

printerstart:	tBEGIN PRINTER NAME ASSIGN tSTRING DRIVER ASSIGN tSTRING tEND
	{ initialize_the_printer($5, $8); }
;
printerstartalt: tBEGIN PRINTER DRIVER ASSIGN tSTRING NAME ASSIGN tSTRING tEND
	{ initialize_the_printer($8, $5); }
;
printerend: 		tBEGIN ENDPRINTER tEND
	{ output_the_printer(); }
;
color:			tBEGIN COLOR tEND 
	{ thePrinter.printvars.output_type = OUTPUT_COLOR; }
;
nocolor:		tBEGIN NOCOLOR tEND
	{ thePrinter.printvars.output_type = OUTPUT_GRAY; }
;
model:			tBEGIN MODEL VALUE ASSIGN tINT tEND
	{ thePrinter.model = $5; }
;
language:		tBEGIN LANGUAGE VALUE ASSIGN tCLASS tEND
	{
	  if (!strcmp($5, "canon"))
	    thePrinter.printvars.linear = ECANON;
	  else if (!strcmp($5, "escp2"))
	    thePrinter.printvars.linear = EESCP2;
	  else if (!strcmp($5, "pcl"))
	    thePrinter.printvars.linear = EPCL;
	  else if (!strcmp($5, "ps"))
	    thePrinter.printvars.linear = EPS;
	}
;
brightness:		tBEGIN BRIGHTNESS VALUE ASSIGN tINT tEND
	{ thePrinter.printvars.brightness = $5; }
;
gamma:			tBEGIN GAMMA VALUE ASSIGN tDOUBLE tEND
	{ thePrinter.printvars.gamma = $5; }
;
contrast:		tBEGIN CONTRAST VALUE ASSIGN tINT tEND
	{ thePrinter.printvars.contrast = $5; }
;
red:			tBEGIN RED VALUE ASSIGN tINT tEND
	{ thePrinter.printvars.red = $5; }
;
green:			tBEGIN GREEN VALUE ASSIGN tINT tEND
	{ thePrinter.printvars.green = $5; }
;
blue:			tBEGIN BLUE VALUE ASSIGN tINT tEND
	{ thePrinter.printvars.blue = $5; }
;
saturation:		tBEGIN SATURATION VALUE ASSIGN tDOUBLE tEND
	{ thePrinter.printvars.saturation = $5; }
;
density:		tBEGIN DENSITY VALUE ASSIGN tDOUBLE tEND
	{ thePrinter.printvars.density = $5; }
;

Empty:

pstart: printerstart | printerstartalt
;

parg: color | nocolor | model | language | brightness | gamma | contrast
	| red | green | blue | saturation | density

pargs: pargs parg | parg

Printer: pstart pargs printerend | pstart printerend

Printers: Printers Printer | Empty

%%

int
main(int argc, char **argv)
{
  int retval;
  printf("/* This file is automatically generated.  See printers.xml.\n");
  printf("   DO NOT EDIT! */\n\n");
  printf("const static printer_t printers[] =\n");
  printf("{\n");
  retval = yyparse();
  printf("};\n");
  printf("const static int printer_count = sizeof(printers) / sizeof(printer_t);\n");
  return retval;
}
