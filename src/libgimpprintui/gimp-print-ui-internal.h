/*
 * "$Id: gimp-print-ui-internal.h,v 1.4 2003/01/12 22:38:30 rlk Exp $"
 *
 *   Print plug-in for the GIMP.
 *
 *   Copyright 1997-2000 Michael Sweet (mike@easysw.com),
 *	Robert Krawitz (rlk@alum.mit.edu). and Steve Miller (smiller@rni.net
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
 *
 * Revision History:
 *
 *   See ChangeLog
 */

#ifndef __GIMP_PRINT_UI_INTERNAL_H__
#define __GIMP_PRINT_UI_INTERNAL_H__

#ifdef __GNUC__
#define inline __inline__
#endif

#include <gtk/gtk.h>

#ifdef INCLUDE_GIMP_PRINT_H
#include INCLUDE_GIMP_PRINT_H
#else
#include <gimp-print/gimp-print.h>
#endif

typedef struct
{
  const char *name;
  void (*extra)(const gchar *);
  gint callback_id;
  const stp_parameter_t *fast_desc;
  const char *default_val;
  stp_string_list_t params;
  GtkWidget *combo;
  GtkWidget *label;
} list_option_t;

typedef struct
{
  const char *name;
  const char *help;
  gdouble scale;
  GtkWidget *checkbox;
  const char *format;
} unit_t;

typedef struct
{
  const char *name;
  const char *help;
  gint value;
  GtkWidget *button;
} radio_group_t;

typedef struct
{
  const char *name;
  GtkObject *adjustment;
  gfloat scale;
  gint is_active;
  gint update_thumbnail;
} color_option_t;

typedef struct
{
  unsigned char *base_addr;
  off_t offset;
  off_t limit;
} priv_t;

#define PLUG_IN_VERSION		VERSION " - " RELEASE_DATE
#define PLUG_IN_NAME		"Print"

#define INVALID_TOP 1
#define INVALID_LEFT 2

#define SCALE_ENTRY_LABEL(adj) \
        GTK_LABEL (gtk_object_get_data (GTK_OBJECT(adj), "label"))

#define SCALE_ENTRY_SCALE(adj) \
        GTK_HSCALE (gtk_object_get_data (GTK_OBJECT(adj), "scale"))
#define SCALE_ENTRY_SCALE_ADJ(adj) \
        gtk_range_get_adjustment \
        (GTK_RANGE (gtk_object_get_data (GTK_OBJECT (adj), "scale")))

#define SCALE_ENTRY_SPINBUTTON(adj) \
        GTK_SPIN_BUTTON (gtk_object_get_data (GTK_OBJECT (adj), "spinbutton"))
#define SCALE_ENTRY_SPINBUTTON_ADJ(adj) \
        gtk_spin_button_get_adjustment \
        (GTK_SPIN_BUTTON (gtk_object_get_data (GTK_OBJECT (adj), "spinbutton")))

/*
 * Function prototypes
 */
extern void stpui_plist_set_output_to(stpui_plist_t *p, const char *val);
extern void stpui_plist_set_output_to_n(stpui_plist_t *p, const char *val, int n);
extern const char *stpui_plist_get_output_to(const stpui_plist_t *p);
extern void stpui_plist_set_name(stpui_plist_t *p, const char *val);
extern void stpui_plist_set_name_n(stpui_plist_t *p, const char *val, int n);
extern const char *stpui_plist_get_name(const stpui_plist_t *p);
extern void stpui_plist_copy(stpui_plist_t *vd, const stpui_plist_t *vs);
extern gint stpui_plist_count;	   /* Number of system printers */
extern gint stpui_plist_current;     /* Current system printer */
extern stpui_plist_t *stpui_plist;		  /* System printers */

extern int stpui_plist_add(const stpui_plist_t *key, int add_only);
extern void stpui_printer_initialize(stpui_plist_t *printer);
extern const char *stpui_combo_get_name(GtkWidget   *combo,
				  const stp_string_list_t options);
extern void stpui_set_adjustment_tooltip(GtkObject *adjustment,
					 const gchar *tip);
extern void stpui_set_help_data(GtkWidget *widget, const gchar *tooltip);
extern GtkWidget *stpui_table_attach_aligned(GtkTable *table, gint column,
					     gint row, const gchar *label_text,
					     gfloat xalign, gfloat yalign,
					     GtkWidget *widget, gint colspan,
					     gboolean left_align);

extern GtkWidget *stpui_create_entry(GtkWidget *table, int hpos, int vpos,
				     const char *text, const char *help,
				     GtkSignalFunc callback);
extern GSList *stpui_create_radio_button(radio_group_t *radio, GSList *group,
					 GtkWidget *table, int hpos, int vpos,
					 GtkSignalFunc callback);
extern void stpui_set_adjustment_tooltip (GtkObject *adj, const gchar *tip);
extern void stpui_create_new_combo(list_option_t *list_option,
				   GtkWidget *table, int hpos, int vpos);
extern void stpui_help_init (void);
extern void stpui_help_free (void);
extern void stpui_enable_help (void);
extern void stpui_disable_help (void);
extern void stpui_set_help_data (GtkWidget *widget, const gchar *tooltip);

extern GtkWidget *stpui_dialog_new(const gchar       *title,
				   const gchar       *wmclass_name,
				   GtkWindowPosition  position,
				   gint               allow_shrink,
				   gint               allow_grow,
				   gint               auto_shrink,
				   /* specify action area buttons as va_list:
				    *  const gchar    *label,
				    *  GtkSignalFunc   callback,
				    *  gpointer        data,
				    *  GtkObject      *slot_object,
				    *  GtkWidget     **widget_ptr,
				    *  gboolean        default_action,
				    *  gboolean        connect_delete,
				    */
				   ...);

extern GtkWidget *stpui_option_menu_new(gboolean            menu_only,
					/* specify menu items as va_list:
					 *  const gchar    *label,
					 *  GtkSignalFunc   callback,
					 *  gpointer        data,
					 *  gpointer        user_data,
					 *  GtkWidget     **widget_ptr,
					 *  gboolean        active
					 */
					...);
extern GtkObject *stpui_scale_entry_new(GtkTable    *table,
					gint         column,
					gint         row,
					const gchar *text,
					gint         scale_usize,
					gint         spinbutton_usize,
					gfloat       value,
					gfloat       lower,
					gfloat       upper,
					gfloat       step_increment,
					gfloat       page_increment,
					guint        digits,
					gboolean     constrain,
					gfloat       unconstrained_lower,
					gfloat       unconstrained_upper,
					const gchar *tooltip);


/* Thumbnails -- keep it simple! */

stp_image_t *stpui_image_thumbnail_new(const guchar *data, gint w, gint h,
				       gint bpp);

#endif  /* __GIMP_PRINT_UI_INTERNAL_H__ */
