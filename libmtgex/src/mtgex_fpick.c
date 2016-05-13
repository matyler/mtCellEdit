/*
	Copyright (C) 2004-2015 Mark Tyler
	Copyright (C) 2006-2008 Mark Tyler and Dmitry Groshev

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program in the file COPYING.
*/

#include "private.h"

#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>

#include "graphics/xpm_case.xpm"
#include "graphics/xpm_hidden.xpm"
#include "graphics/xpm_home.xpm"
#include "graphics/xpm_new.xpm"
#include "graphics/xpm_newdir.xpm"
#include "graphics/xpm_open.xpm"
#include "graphics/xpm_up.xpm"



#define FPICK_PREFIX_BUF_SIZE	128
#define FP_DATA_KEY		"mtPaint.fp_data"

#define FPICK_ICON_UP		0
#define FPICK_ICON_HOME		1
#define FPICK_ICON_DIR		2
#define FPICK_ICON_HIDDEN	3
#define FPICK_ICON_CASE		4
#define FPICK_ICON_TOT		5

#define FPICK_COMBO_ITEMS	16

#define FPICK_CLIST_COLS	4
#define FPICK_CLIST_COLS_HIDDEN	2	// Used for sorting file/directory names

#define FPICK_CLIST_NAME	0
#define FPICK_CLIST_SIZE	1
#define FPICK_CLIST_TYPE	2
#define FPICK_CLIST_DATE	3

#define FPICK_CLIST_H_UC	4
#define FPICK_CLIST_H_C		5



typedef struct
{
	int		allow_files,	// Allow the user to select files /
					// directories
			allow_dirs,
			sort_column,	// Which column is being sorted in clist
			show_hidden
			;

	char		combo_items[FPICK_COMBO_ITEMS][PATHBUF],
					// Stored as UTF8 in GTK+2
			txt_directory[PATHBUF],
					// Current directory - Normal C string
			txt_file[PATHBUF]
					// Full filename - Normal C string
			;

	GtkWidget	* window,	// Main window
			* ok_button,	// OK button
			* cancel_button,// Cancel button
			* main_vbox,	// For extra widgets
			* toolbar,	// Toolbar
			* icons[FPICK_ICON_TOT], // Icons
			* combo,	// List at top holding recent dirs
			* combo_entry,	// Directory entry area in combo
			* clist,	// Containing list of files/directories
			* sort_arrows[FPICK_CLIST_COLS +
				FPICK_CLIST_COLS_HIDDEN],
					// Column sort arrows
			* file_entry	// Text entry box for filename
			;
	GtkSortType	sort_direction;	// Sort direction of clist

	GList		* combo_list;	// List of combo items
} fpicker;



static GtkWidget	* fpick_main_window = NULL;
static mtPrefs		* fpick_main_prefs = NULL;
static char		fpick_prefix_buf[FPICK_PREFIX_BUF_SIZE] = {0};
static int		case_insensitive;



/* *** A WORD OF WARNING ***
 * Collating string comparison functions consider letter case only *AFTER*
 * letter itself for any (usable) value of LC_COLLATE except LC_COLLATE=C, and
 * in that latter case, order by character codepoint which frequently is
 * unrelated to alphabetical ordering. And on GTK+1 with multibyte charset,
 * casefolding will break down horribly, too.
 * What this means in practice: don't expect anything sane out of alphabetical
 * sorting outside of strict ASCII, and don't expect anything sane out of
 * case-sensitive sorting at all. - WJ */

static char * isort_key (
	char		* src
	)
{
	char		* s;


	src = g_utf8_casefold ( src, -1 );
	s = g_utf8_collate_key ( src, -1 );
	g_free ( src );

	return s;
}

#define strkeycmp	strcmp
#define strcollcmp	g_utf8_collate

static void fpick_cleanse_path (	// Clean up null terminated path
	char		* const	txt
	)
{
	static char const dds[] = { MTKIT_DIR_SEP, MTKIT_DIR_SEP, 0 };
	char		* src,
			* dest;


	// Remove multiple consecutive occurences of DIR_SEP
	if ( ( dest = src = strstr ( txt, dds ) ) )
	{
		while ( src[0] )
		{
			if ( src[0] == MTKIT_DIR_SEP )
			{
				while ( src[1] == MTKIT_DIR_SEP )
				{
					src++;
				}
			}

			*dest++ = *src++;
		}

		*dest++ = 0;
	}
}

static gint fpick_compare (
	GtkCList	* const	clist,
	gconstpointer	const	ptr1,
	gconstpointer	const	ptr2
	)
{
	static signed char const sort_order[] = {
			FPICK_CLIST_NAME,
			FPICK_CLIST_DATE,
			FPICK_CLIST_SIZE,
			-1
			};
	GtkCListRow const * r1 = ptr1,
			* r2 = ptr2;
	unsigned char	* s1,
			* s2;
	int		c = clist->sort_column, bits = 0,
			lvl = 0,
			d = 0;


	/* "/ .." Directory always goes first, and conveniently it is also the
	 * one and only GTK_CELL_TEXT in an entire column */

	d = (int)(r1->cell[FPICK_CLIST_NAME].type -
		r2->cell[FPICK_CLIST_NAME].type );

	if ( GTK_CELL_TEXT > GTK_CELL_PIXTEXT )
	{
		d = -d;
	}

	if ( d )
	{
		if ( clist->sort_type == GTK_SORT_DESCENDING )
		{
			return -d;
		}

		return d;
	}

	/* Directories have empty size column, and always go before files */
	s1 = (unsigned char *)GTK_CELL_TEXT ( r1->cell[FPICK_CLIST_SIZE]
		)->text;
	s2 = (unsigned char *)GTK_CELL_TEXT ( r2->cell[FPICK_CLIST_SIZE]
		)->text;

	if ( ! s1[0] ^ ! s2[0] )
	{
		d = (int)s1[0] - s2[0];

		if ( clist->sort_type == GTK_SORT_DESCENDING )
		{
			return -d;
		}

		return d;
	}

	while ( c >= 0 )
	{
		if ( bits & (1 << c) )
		{
			c = sort_order[lvl ++];

			continue;
		}

		bits |= 1 << c;
		s1 = (unsigned char *)GTK_CELL_TEXT ( r1->cell[c] )->text;
		s2 = (unsigned char *)GTK_CELL_TEXT ( r2->cell[c] )->text;

		switch (c)
		{
		case FPICK_CLIST_TYPE:
			if ( ( d = strcollcmp ( (char *)s1, (char *)s2 ) ) )
			{
				break;
			}
			continue;

		case FPICK_CLIST_SIZE:
			if ( ( d = strcmp ( (char *)s1, (char *)s2 ) ) )
			{
				break;
			}
			continue;

		case FPICK_CLIST_DATE:
			if ( ( d = strcmp ( (char *)s2, (char *)s1 ) ) )
			{
				break;	// Newest first
			}
			continue;

		default:
		case FPICK_CLIST_NAME:
			c = case_insensitive ? FPICK_CLIST_H_UC :
				FPICK_CLIST_H_C;
			continue;

		case FPICK_CLIST_H_UC:
		case FPICK_CLIST_H_C:
			if ( ( d = strkeycmp ( (char *)s1, (char *)s2 ) ) )
			{
				break;
			}

			c = FPICK_CLIST_H_C;

			continue;
		}

		break;
	}

	return d;
}

static void fpick_sort_files (
	fpicker		* const	win
	)
{
	gtk_clist_set_sort_type ( GTK_CLIST ( win->clist ),
		win->sort_direction );
	gtk_clist_set_sort_column ( GTK_CLIST ( win->clist ),
		win->sort_column );
	gtk_clist_sort ( GTK_CLIST ( win->clist ) );
}

static void fpick_column_button (
	GtkCList	* const	ARG_UNUSED ( clist ),
	gint		const	column,
	gpointer	const	user_data
	)
{
	fpicker		* const	fp = user_data;
	GtkSortType	direction;


	if (	column < 0		||
		column >= FPICK_CLIST_COLS
		)
	{
		return;
	}

// reverse the sorting direction if the list is already sorted by this col

	if ( fp->sort_column == column )
	{
		direction = ( fp->sort_direction == GTK_SORT_ASCENDING ?
			GTK_SORT_DESCENDING : GTK_SORT_ASCENDING );
	}
	else	// Different column clicked so use default value for that column
	{
		direction = GTK_SORT_ASCENDING;

		// Hide old arrow
		gtk_widget_hide ( fp->sort_arrows[fp->sort_column] );

		// Show new arrow
		gtk_widget_show ( fp->sort_arrows[column] );

		fp->sort_column = column;
	}

	gtk_arrow_set ( GTK_ARROW ( fp->sort_arrows[column] ),
		direction == GTK_SORT_ASCENDING ? GTK_ARROW_DOWN : GTK_ARROW_UP,
		GTK_SHADOW_IN );

	fp->sort_direction = direction;

	fpick_sort_files ( fp );
}

static void fpick_directory_new (	// Register directory in combo
	fpicker		* const	win,
	char		* const	name
	)
{
	int		i;
	char		txt [ PATHBUF ];


	mtgex_gtkuncpy ( txt, name, sizeof ( txt ) );

	// !!! Shuffle list items, not strings !!!
	for ( i = 0 ; i < (FPICK_COMBO_ITEMS - 1); i++ )
	{
		// Does this text already exist in the list?
		if ( ! strcmp ( txt, win->combo_items[i] ) )
		{
			break;
		}
	}

	for ( ; i > 0; i-- )
	{
		// Shuffle items down as needed
		mtkit_strnncpy ( win->combo_items[i], win->combo_items[i - 1],
			sizeof ( win->combo_items[i] ) );
	}

	// Add item to list
	mtkit_strnncpy ( win->combo_items[0], txt,
		sizeof ( win->combo_items[0] ));

	gtk_combo_set_popdown_strings ( GTK_COMBO ( win->combo ),
		win->combo_list );
	gtk_entry_set_text ( GTK_ENTRY ( win->combo_entry ), txt );
}

static GtkWidget * mtgex_pack5 (
	GtkWidget	* box,
	GtkWidget	* widget
	)
{
	gtk_box_pack_start ( GTK_BOX ( box ), widget, FALSE, FALSE, 5 );

	return widget;
}



static char const root_dir[] = { MTKIT_DIR_SEP, 0 };



static int fpick_scan_directory (	// Scan directory, populate widgets
	fpicker		* const	win,
	char		* const	name
	)
{
	static char	updir[] = { MTKIT_DIR_SEP, ' ', '.', '.', 0 };
	static char	nothing[] = "";
	DIR		* dp;
	struct dirent	* ep;
	struct stat	buf;
	char		* cp,
			* src,
			* dest,
			full_name [ PATHBUF ],
			* row_txt [ FPICK_CLIST_COLS + FPICK_CLIST_COLS_HIDDEN],
			txt_name [ PATHBUF ],
			txt_size [ 64 ],
			txt_date [ 64 ],
			tmp_txt [ 64 ];
	GdkPixmap	* icons [ 2 ];
	GdkBitmap	* masks [ 2 ];
	int		i,
			l,
			row;
	size_t		len;


	icons[1] = gdk_pixmap_create_from_xpm_d ( fpick_main_window->window,
		&masks[1], NULL, xpm_open_xpm );
	icons[0] = gdk_pixmap_create_from_xpm_d ( fpick_main_window->window,
		&masks[0], NULL, xpm_new_xpm );

	row_txt[FPICK_CLIST_SIZE] = txt_size;
	row_txt[FPICK_CLIST_DATE] = txt_date;

	mtkit_strnncpy ( full_name, name, sizeof ( full_name ) - 1 );
	len = strlen ( full_name );

	/* Ensure the invariant */
	if ( len == 0 || (full_name[len - 1] != MTKIT_DIR_SEP) )
	{
		full_name [ len ++ ] = MTKIT_DIR_SEP;
		full_name [ len ] = 0;
	}

	dp = opendir ( full_name );

	if ( ! dp )
	{
		return FALSE;		// Directory doesn't exist so fail
	}

	mtkit_strnncpy ( win->txt_directory, full_name,
		sizeof ( win->txt_directory ) );
	fpick_directory_new ( win, full_name );	// Register directory in combo

	gtk_clist_clear ( GTK_CLIST ( win->clist ) );	// Empty the list
	gtk_clist_freeze ( GTK_CLIST ( win->clist ) );

	if ( strcmp ( full_name, root_dir ) ) // Have a parent dir to move to?
	{
		row_txt[FPICK_CLIST_NAME] = updir;
		row_txt[FPICK_CLIST_TYPE] = row_txt[FPICK_CLIST_H_UC] =
			row_txt[FPICK_CLIST_H_C] = "";
		txt_size[0] = txt_date[0] = '\0';
		gtk_clist_append ( GTK_CLIST ( win->clist ), row_txt );
	}

	while ( ( ep = readdir ( dp ) ) )
	{
		full_name[len] = 0;
		mtkit_strnncat ( full_name, ep->d_name, sizeof ( full_name ) );

		// Error getting file details
		if ( stat ( full_name, &buf ) < 0 )
		{
			continue;
		}

		if (	! win->show_hidden	&&
			( ep->d_name[0] == '.' )
			)
		{
			continue;
		}

		strftime ( txt_date, 60, "%Y-%m-%d   %H:%M.%S",
			localtime ( &buf.st_mtime ) );
		row_txt[FPICK_CLIST_TYPE] = nothing;

		if ( /*ep->d_type == DT_DIR ||*/ S_ISDIR ( buf.st_mode ) )
		{
			// Subdirectory

			if ( ! win->allow_dirs )
			{
				continue;
			}

			// Don't look at '.' or '..'

			if (	! strcmp ( ep->d_name, "." ) ||
				! strcmp ( ep->d_name, ".." )
				)
			{
				continue;
			}

			mtgex_gtkuncpy ( txt_name, ep->d_name,
				sizeof ( txt_name ) );
			txt_size[0] = '\0';
		}
		else
		{
			// File

			if ( ! win->allow_files )
			{
				continue;
			}

			mtgex_gtkuncpy ( txt_name, ep->d_name,
				sizeof ( txt_name ) );
			l = snprintf ( tmp_txt, 64, "%llu",
				(unsigned long long)buf.st_size );
			memset ( txt_size, ' ', 20 );
			dest = txt_size + 20;
			*dest-- = '\0';

			for ( src = tmp_txt + l - 1; src - tmp_txt > 2; )
			{
				*dest-- = *src--;
				*dest-- = *src--;
				*dest-- = *src--;
				*dest-- = ',';
			}

			while ( src - tmp_txt >= 0 )
			{
				*dest-- = *src--;
			}

			cp = strrchr ( txt_name, '.' );
			if ( cp && (cp != txt_name ) )
			{
				row_txt[FPICK_CLIST_TYPE] =
					g_utf8_strup ( cp + 1, -1 );
			}
		}

		row_txt[FPICK_CLIST_H_C] = g_utf8_collate_key ( txt_name, -1 );
		row_txt[FPICK_CLIST_H_UC] = isort_key ( txt_name );
		row_txt[FPICK_CLIST_NAME] = "";
		// No use to set name just to reset again

		row = gtk_clist_append ( GTK_CLIST ( win->clist ), row_txt );
		g_free ( row_txt[FPICK_CLIST_H_UC] );

		i = ! txt_size[0];
		gtk_clist_set_pixtext ( GTK_CLIST ( win->clist ), row,
			FPICK_CLIST_NAME, txt_name, 4, icons[i], masks[i] );

		if ( row_txt[FPICK_CLIST_TYPE] != nothing )
		{
			g_free ( row_txt[FPICK_CLIST_TYPE] );
		}

		g_free ( row_txt[FPICK_CLIST_H_C] );
	}

	fpick_sort_files ( win );
	gtk_clist_select_row ( GTK_CLIST ( win->clist ), 0, 0);
	gtk_clist_thaw ( GTK_CLIST ( win->clist ) );
	closedir ( dp );
	gdk_pixmap_unref ( icons[0] );
	gdk_pixmap_unref ( icons[1] );
	gdk_pixmap_unref ( masks[0] );
	gdk_pixmap_unref ( masks[1] );

	return TRUE;
}

static char * mtgex_gtkncpy (		// Wrapper for utf8->C translation
	char		* const	dest,	// Buffer, or NULL => dynamic creation
	char	const	* const	src,
	size_t		const	cnt	// Bytes in dest buffer
	)
{
	char		* c;


	c = g_locale_from_utf8 ( (const gchar *)src, -1, NULL, NULL, NULL );

	if ( c )
	{
		if ( ! dest )
		{
			return c;
		}

		mtkit_strnncpy ( dest, c, cnt );
		g_free ( c );
	}
	else
	{
		if ( ! dest )
		{
			return g_strdup ( src );
		}

		mtkit_strnncpy ( dest, src, cnt );
	}

	return dest;
}

static void fpick_enter_dir_via_list (
	fpicker		* const	fp,
	char		* const	name
	)
{
	char		ndir [ PATHBUF ],
			* c;
	size_t		l;


	mtkit_strnncpy ( ndir, fp->txt_directory, sizeof ( ndir ) );
	l = strlen ( ndir );

	if ( ! strcmp ( name, ".." ) )	// Go to parent directory
	{
		if ( l > 0 && ( ndir[l - 1] == MTKIT_DIR_SEP ) )
		{
			ndir[--l] = '\0';
		}

		c = strrchr ( ndir, MTKIT_DIR_SEP );

		if ( c )
		{
			c[0] = '\0';
		}
		else
		{
			mtkit_strnncpy ( ndir, root_dir, sizeof ( ndir ) );
		}
	}
	else
	{
		mtgex_gtkncpy ( ndir + l, name, sizeof ( ndir ) - l );
	}

	fpick_cleanse_path ( ndir );
	fpick_scan_directory ( fp, ndir );	// Enter new directory
}

static char * get_fname (
	GtkCList	* const	clist,
	int		const	row
	)
{
	char		* txt = NULL;


	if ( gtk_clist_get_cell_type ( clist, row, FPICK_CLIST_NAME) ==
		GTK_CELL_TEXT )
	{
		return ("..");
	}

	gtk_clist_get_pixtext ( clist, row, FPICK_CLIST_NAME, &txt, NULL, NULL,
		NULL );

	return txt;
}

static void fpick_select_row (
	GtkCList	* const	clist,
	gint		const	row,
	gint		const	ARG_UNUSED ( col ),
	GdkEventButton	* const	event,
	gpointer	const	user_data
	)
{
	fpicker		* const	fp = user_data;
	char		* txt_name,
			* txt_size;
	int		dclick = event && (event->type == GDK_2BUTTON_PRESS);


	txt_name = get_fname ( clist, row );

	gtk_clist_get_text ( clist, row, FPICK_CLIST_SIZE, &txt_size );
	if ( ! txt_name )
	{
		return;
	}

	if ( ! txt_size[0] )		// Directory selected
	{
		// Double click on directory so try to enter it
		if ( dclick )
		{
			fpick_enter_dir_via_list ( fp, txt_name );
		}
	}
	else				// File selected
	{
		gtk_entry_set_text ( GTK_ENTRY ( fp->file_entry ), txt_name );

		// Double click on file, so press OK button
		if ( dclick )
		{
			gtk_button_clicked ( GTK_BUTTON ( fp->ok_button ) );
		}
	}
}

static void fpick_combo_changed (
	GtkWidget	* const	ARG_UNUSED ( widget ),
	gpointer	const	user_data
	)
{
	fpicker		* const	fp = user_data;
	char		txt [ PATHBUF ];
	char	const	* ct;


	ct = gtk_entry_get_text ( GTK_ENTRY ( fp->combo_entry ) );
	mtgex_gtkncpy ( txt, ct, sizeof ( txt ) );

	fpick_cleanse_path ( txt );

	// Only do something if the directory is new
	if ( ! strcmp ( txt, fp->txt_directory ) )
	{
		return;
	}

	if ( ! fpick_scan_directory ( fp, txt ) )
	{
		char		* tt;


		// Directory doesn't exist
		tt = g_strdup_printf ( _("Could not access directory %s"), ct );
		mtgex_alert_box ( _("Error"), tt, _("OK"), NULL, NULL,
			fpick_main_window );
		g_free ( tt );

		// Revert to current directory name in the entry
		fpick_directory_new ( fp, fp->txt_directory );
	}
}

static gboolean fpick_key_event (
	GtkWidget	* const	widget,
	GdkEventKey	* const	event,
	gpointer	const	user_data
	)
{
	fpicker		* const	fp = user_data;
	GList		* list;
	char		* txt_name,
			* txt_size;
	int		row = 0;


	switch (event->keyval)
	{
	case GDK_End:
	case GDK_KP_End:
		row = GTK_CLIST ( fp->clist )->rows - 1;

	case GDK_Home:
	case GDK_KP_Home:
		GTK_CLIST ( fp->clist )->focus_row = row;
		gtk_clist_select_row ( GTK_CLIST ( fp->clist ), row, 0 );
		gtk_clist_moveto ( GTK_CLIST ( fp->clist ), row, 0, 0.5, 0.5 );

		return TRUE;

	case GDK_Return:
	case GDK_KP_Enter:
		break;

	default:
		return FALSE;
	}

	if ( ! ( list = GTK_CLIST ( fp->clist )->selection ) )
	{
		return FALSE;
	}

	row = GPOINTER_TO_INT ( list->data );

	txt_name = get_fname ( GTK_CLIST ( widget ), row );
	if ( ! txt_name )
	{
		return TRUE;
	}

	gtk_clist_get_text ( GTK_CLIST ( widget ), row, FPICK_CLIST_SIZE,
		&txt_size );

	if ( ! txt_size[0] )
	{
		/* Directory selected */
		fpick_enter_dir_via_list ( fp, txt_name );
	}
	else
	{
		/* File selected */
		gtk_button_clicked (GTK_BUTTON (fp->ok_button));
	}

	return TRUE;
}



typedef struct
{
	unsigned char	ID;
	signed char	radio;
	unsigned char	sep,
			rclick;
	int		actmap;
	char		* tooltip,
			** xpm;
	GtkWidget	* widget;
} toolbar_item;



static void fill_toolbar (
	GtkToolbar	* const	bar,
	toolbar_item	*	items,
	GtkSignalFunc	const	lclick,
	int		const	lbase,
	GtkSignalFunc	const	rclick,
	int		const	rbase
	)
{
	GtkWidget	* iconw,
			* radio[32] = {0};
	GdkPixmap	* icon,
			* mask;


	for ( ; items->xpm; items ++ )
	{
		icon = gdk_pixmap_create_from_xpm_d ( fpick_main_window->window,
			&mask, NULL, items->xpm );
		iconw = gtk_pixmap_new ( icon, mask );
		gdk_pixmap_unref ( icon );
		gdk_pixmap_unref ( mask );
		items->widget = gtk_toolbar_append_element ( bar,
			items->radio < 0 ? GTK_TOOLBAR_CHILD_BUTTON :
			items->radio ? GTK_TOOLBAR_CHILD_RADIOBUTTON :
			GTK_TOOLBAR_CHILD_TOGGLEBUTTON,
			items->radio > 0 ? radio[items->radio] : NULL,
			NULL, _(items->tooltip), "Private", iconw, lclick,
			(gpointer)(intptr_t)(items->ID + lbase ) );

		if ( items->radio > 0 )
		{
			radio[items->radio] = items->widget;
		}

		if ( items->rclick )
		{
			gtk_signal_connect ( GTK_OBJECT ( items->widget ),
				"button_press_event", rclick,
				(gpointer)(intptr_t)( items->ID + rbase ) );
		}

		if ( items->sep )
		{
			gtk_toolbar_append_space ( bar );
		}
	}
}



#undef _
#define _(X) X



static toolbar_item fpick_bar[] = {
{ FPICK_ICON_UP,	-1, 0, 0, 0, _("Up"), xpm_up_xpm, NULL },
{ FPICK_ICON_HOME,	-1, 0, 0, 0, _("Home"), xpm_home_xpm, NULL },
{ FPICK_ICON_DIR,	-1, 0, 0, 0, _("Create New Directory"), xpm_newdir_xpm, NULL },
{ FPICK_ICON_HIDDEN,	0, 0, 0, 0, _("Show Hidden Files"), xpm_hidden_xpm, NULL },
{ FPICK_ICON_CASE,	0, 0, 0, 0, _("Case Insensitive Sort"), xpm_case_xpm, NULL },
{ 0, 0, 0, 0, 0, NULL, NULL, NULL }
				};



#undef _
#define _(X) __(X)



static void fpick_iconbar_click (
	GtkWidget	* widget,
	gpointer	data
	);



static GtkWidget * fpick_toolbar (
	GtkWidget	** const wlist
	)
{
	int		i;
	GtkWidget	* toolbar;


	toolbar = gtk_toolbar_new ();
	gtk_toolbar_set_style ( GTK_TOOLBAR ( toolbar ), GTK_TOOLBAR_ICONS );
	fill_toolbar ( GTK_TOOLBAR ( toolbar ), fpick_bar,
		GTK_SIGNAL_FUNC ( fpick_iconbar_click ), 0, NULL, 0);
	gtk_widget_show ( toolbar );

	for ( i = 0; i < FPICK_ICON_TOT; i++ )
	{
		wlist[i] = fpick_bar[i].widget;
	}

	return toolbar;
}

static GtkWidget * mtgex_pack_end5 (
	GtkWidget	* const	box,
	GtkWidget	* const	widget
	)
{
	gtk_box_pack_end ( GTK_BOX ( box ), widget, FALSE, FALSE, 5 );

	return widget;
}

static GtkWidget * mtgex_xpack (
	GtkWidget	* const	box,
	GtkWidget	* const	widget
	)
{
	gtk_box_pack_start ( GTK_BOX ( box ), widget, TRUE, TRUE, 0 );

	return widget;
}

GtkWidget * mtgex_fpick_new (
	char	const	* const	title,
	int		const	flags,
	GtkWidget	* const	main_window,
	mtPrefs		* const	prefs,
	char	const	* const	prefs_prefix
	)
{
	static short const col_width[FPICK_CLIST_COLS] = {250, 64, 80, 150};
	char		txt[256];
	char	const	* col_titles[FPICK_CLIST_COLS + FPICK_CLIST_COLS_HIDDEN]
			= { "", "", "", "", "", "" };
	char	const	* txtp;
	GtkWidget	* vbox1,
			* hbox1,
			* scrolledwindow1,
			* temp_hbox;
	GtkAccelGroup	* ag = gtk_accel_group_new ();
	fpicker		* res;
	int		i,
			j;


	if ( ! main_window )
	{
		return NULL;
	}

	res = calloc ( 1, sizeof ( fpicker ) );
	if ( ! res  )
	{
		return NULL;
	}

	fpick_prefix_buf[0] = 0;

	// prefs_prefix checked by function
	mtkit_strnncpy ( fpick_prefix_buf, prefs_prefix,FPICK_PREFIX_BUF_SIZE );

	fpick_main_window = main_window;
	fpick_main_prefs = prefs;

	col_titles[FPICK_CLIST_NAME] = _("Name");
	col_titles[FPICK_CLIST_SIZE] = _("Size");
	col_titles[FPICK_CLIST_TYPE] = _("Type");
	col_titles[FPICK_CLIST_DATE] = _("Modified");

	snprintf ( txt, sizeof ( txt ), "%sfpick_case_insensitive",
		fpick_prefix_buf );
	case_insensitive = 1;
	mtkit_prefs_get_int ( fpick_main_prefs, txt, &case_insensitive );

	snprintf ( txt, sizeof ( txt), "%sfpick_show_hidden", fpick_prefix_buf);
	res->show_hidden = 0;
	mtkit_prefs_get_int ( fpick_main_prefs, txt, &res->show_hidden );

	res->sort_direction = GTK_SORT_ASCENDING;
	res->sort_column = 0;
	res->allow_files = res->allow_dirs = TRUE;
	res->txt_directory[0] = res->txt_file[0] = '\0';

	res->window = mtgex_add_a_window ( GTK_WINDOW_TOPLEVEL, title,
		GTK_WIN_POS_NONE, TRUE );
	gtk_object_set_data ( GTK_OBJECT ( res->window ), FP_DATA_KEY, res );

	vbox1 = gtk_vbox_new ( FALSE, 0 );
	gtk_container_add ( GTK_CONTAINER ( res->window ), vbox1);
	hbox1 = mtgex_pack5 ( vbox1, gtk_hbox_new ( FALSE, 0 ) );

	// ------- Combo Box -------

	res->combo = mtgex_xpack5 ( hbox1, gtk_combo_new () );
	gtk_combo_disable_activate ( GTK_COMBO ( res->combo ) );
	res->combo_entry = GTK_COMBO ( res->combo )->entry;

	for ( i = 0; i < FPICK_COMBO_ITEMS; i++ )
	{
		snprintf ( txt, sizeof ( txt ), "%sfpick_dir_%i",
			fpick_prefix_buf, i + 1 );

		txtp = NULL;
		mtkit_prefs_get_str ( fpick_main_prefs, txt, &txtp );

		if ( ! txtp )
		{
			txtp = "";
		}

		mtgex_gtkuncpy ( res->combo_items[i], txtp,
			sizeof ( res->combo_items[i] ) );

		res->combo_list = g_list_append ( res->combo_list,
			res->combo_items[i] );
	}

	gtk_combo_set_popdown_strings ( GTK_COMBO ( res->combo ),
		res->combo_list );

	gtk_signal_connect ( GTK_OBJECT ( GTK_COMBO ( res->combo )->popwin ),
		"hide", GTK_SIGNAL_FUNC ( fpick_combo_changed ), res );
	gtk_signal_connect ( GTK_OBJECT ( res->combo_entry ),
		"activate", GTK_SIGNAL_FUNC ( fpick_combo_changed ), res );

	// !!! Show things now - toolbars in GTK+1 mishandle show_all
	gtk_widget_show_all ( vbox1 );

	// ------- Toolbar -------

	res->toolbar = mtgex_pack5 ( hbox1, fpick_toolbar ( res->icons ) );

	gtk_toggle_button_set_active (
		GTK_TOGGLE_BUTTON ( res->icons[FPICK_ICON_HIDDEN] ),
		res->show_hidden );
	gtk_toggle_button_set_active (
		GTK_TOGGLE_BUTTON ( res->icons[FPICK_ICON_CASE] ),
		case_insensitive );
	gtk_object_set_data ( GTK_OBJECT ( res->toolbar ), FP_DATA_KEY, res );
	// Set this after setting the toggles so any events are ignored

	// ------- CLIST - File List -------

	hbox1 = mtgex_xpack5 ( vbox1, gtk_hbox_new ( FALSE, 0 ) );
	scrolledwindow1 = mtgex_xpack5 ( hbox1,
		gtk_scrolled_window_new ( NULL, NULL ) );
	gtk_scrolled_window_set_policy (
		GTK_SCROLLED_WINDOW ( scrolledwindow1 ),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_widget_show_all ( hbox1 );

	res->clist = gtk_clist_new (
		FPICK_CLIST_COLS + FPICK_CLIST_COLS_HIDDEN );
	gtk_clist_set_compare_func ( GTK_CLIST ( res->clist ), fpick_compare );

	for ( i = 0; i < (FPICK_CLIST_COLS + FPICK_CLIST_COLS_HIDDEN); i++ )
	{
		if ( i >= FPICK_CLIST_COLS )
		{
			// Hide the extra sorting columns
			gtk_clist_set_column_visibility (
				GTK_CLIST ( res->clist ), i, FALSE );
		}

		temp_hbox = gtk_hbox_new ( FALSE, 0 );

		if ( i == FPICK_CLIST_TYPE || i == FPICK_CLIST_SIZE )
		{
			mtgex_pack_end ( temp_hbox,
				gtk_label_new ( col_titles[i] ) );
		}
		else if ( i == FPICK_CLIST_NAME )
		{
			mtgex_pack ( temp_hbox,
				gtk_label_new ( col_titles[i] ) );
		}
		else
		{
			mtgex_xpack ( temp_hbox,
				gtk_label_new ( col_titles[i] ) );
		}

		gtk_widget_show_all ( temp_hbox );
		res->sort_arrows[i] = mtgex_pack_end ( temp_hbox,
			gtk_arrow_new ( GTK_ARROW_DOWN, GTK_SHADOW_IN ) );

		if (	i == FPICK_CLIST_SIZE ||
			i == FPICK_CLIST_TYPE
			)
		{
			gtk_clist_set_column_justification (
				GTK_CLIST ( res->clist ),
				i, GTK_JUSTIFY_RIGHT );
		}
		else if ( i == FPICK_CLIST_DATE )
		{
			gtk_clist_set_column_justification (
				GTK_CLIST ( res->clist ),
				i, GTK_JUSTIFY_CENTER );
		}

		gtk_clist_set_column_widget ( GTK_CLIST ( res->clist ), i,
			temp_hbox );
		GTK_WIDGET_UNSET_FLAGS ( GTK_CLIST ( res->clist
			)->column[i].button, (guint)GTK_CAN_FOCUS );
	}

	gtk_widget_show ( res->sort_arrows[0] );	// Show first arrow

	for ( i = 0; i < FPICK_CLIST_COLS; i++ )
	{
		snprintf ( txt, sizeof ( txt ), "%sfpick_col%i",
			fpick_prefix_buf, i + 1 );

		j = col_width[i];
		mtkit_prefs_get_int ( fpick_main_prefs, txt, &j );

		gtk_clist_set_column_width ( GTK_CLIST ( res->clist ), i, j );
	}

	gtk_clist_column_titles_show ( GTK_CLIST ( res->clist ) );
	gtk_clist_set_selection_mode ( GTK_CLIST ( res->clist ),
		GTK_SELECTION_BROWSE );

	gtk_container_add ( GTK_CONTAINER ( scrolledwindow1 ), res->clist );
	gtk_widget_show ( res->clist );

	gtk_signal_connect ( GTK_OBJECT ( res->clist ), "click_column",
		GTK_SIGNAL_FUNC ( fpick_column_button ), res );
	gtk_signal_connect ( GTK_OBJECT ( res->clist ), "select_row",
		GTK_SIGNAL_FUNC ( fpick_select_row ), res );
	gtk_signal_connect ( GTK_OBJECT ( res->clist ), "key_press_event",
		GTK_SIGNAL_FUNC ( fpick_key_event ), res );

	// ------- Extra widget section -------

	gtk_widget_show ( res->main_vbox = mtgex_pack5 ( vbox1,
		gtk_vbox_new ( FALSE, 0 ) ) );

	// ------- Entry Box -------

// !!! What the box is for?
	hbox1 = mtgex_pack5 ( vbox1, gtk_hbox_new ( FALSE, 0 ) );
	res->file_entry = mtgex_xpack5 ( hbox1,
		gtk_entry_new_with_max_length ( 256 ) );
	gtk_widget_show_all ( hbox1 );

	// ------- Buttons -------

	hbox1 = mtgex_pack5 ( vbox1, gtk_hbox_new ( FALSE, 0 ) );

	res->ok_button = mtgex_pack_end5 ( hbox1,
		gtk_button_new_with_label ( _("OK") ) );
	gtk_widget_set_usize ( res->ok_button, 100, -1 );

	res->cancel_button = mtgex_pack_end5 ( hbox1,
		gtk_button_new_with_label ( _("Cancel") ) );
	gtk_widget_set_usize ( res->cancel_button, 100, -1 );
	gtk_widget_add_accelerator ( res->cancel_button, "clicked", ag,
		GDK_Escape, 0, (GtkAccelFlags) 0 );

	gtk_widget_show_all ( hbox1 );

	gtk_window_add_accel_group ( GTK_WINDOW ( res->window ), ag);

	gtk_signal_connect_object ( GTK_OBJECT ( res->file_entry ), "activate",
		GTK_SIGNAL_FUNC ( gtk_button_clicked ),
		GTK_OBJECT ( res->ok_button ) );

	if ( flags & MTGEX_FPICK_ENTRY )
	{
		gtk_widget_grab_focus ( res->file_entry );
	}
	else
	{
		gtk_widget_grab_focus ( res->clist );
	}

	if ( flags & MTGEX_FPICK_DIRS_ONLY )
	{
		res->allow_files = FALSE;
		gtk_widget_hide ( res->file_entry );
	}

	/* Ensure enough space for pixmaps */
	gtk_widget_realize ( res->clist );
	gtk_clist_set_row_height ( GTK_CLIST ( res->clist ), 0 );

	if ( GTK_CLIST ( res->clist )->row_height < 16 )
	{
		gtk_clist_set_row_height ( GTK_CLIST ( res->clist ), 16 );
	}

	return (res->window);
}

int mtgex_fpick_init_prefs (
	mtPrefs		* const	prefs,
	char	const	* const	prefs_prefix
	)
{
	mtPrefTable const	prefs_table[] = {
	{ "fpick_case_insensitive", MTKIT_PREF_TYPE_INT, "1", NULL, NULL, 0, NULL, NULL },
	{ "fpick_show_hidden",	MTKIT_PREF_TYPE_INT, "0", NULL, NULL, 0, NULL, NULL },

	{ "fpick_window_x",	MTKIT_PREF_TYPE_INT, "0", NULL, NULL, 0, NULL, NULL },
	{ "fpick_window_y",	MTKIT_PREF_TYPE_INT, "0", NULL, NULL, 0, NULL, NULL },
	{ "fpick_window_w",	MTKIT_PREF_TYPE_INT, "600", NULL, NULL, 0, NULL, NULL },
	{ "fpick_window_h",	MTKIT_PREF_TYPE_INT, "500", NULL, NULL, 0, NULL, NULL },

	{ "fpick_dir_1",	MTKIT_PREF_TYPE_DIR, NULL, NULL, NULL, 0, NULL, NULL },
	{ "fpick_dir_2",	MTKIT_PREF_TYPE_DIR, NULL, NULL, NULL, 0, NULL, NULL },
	{ "fpick_dir_3",	MTKIT_PREF_TYPE_DIR, NULL, NULL, NULL, 0, NULL, NULL },
	{ "fpick_dir_4",	MTKIT_PREF_TYPE_DIR, NULL, NULL, NULL, 0, NULL, NULL },
	{ "fpick_dir_5",	MTKIT_PREF_TYPE_DIR, NULL, NULL, NULL, 0, NULL, NULL },
	{ "fpick_dir_6",	MTKIT_PREF_TYPE_DIR, NULL, NULL, NULL, 0, NULL, NULL },
	{ "fpick_dir_7",	MTKIT_PREF_TYPE_DIR, NULL, NULL, NULL, 0, NULL, NULL },
	{ "fpick_dir_8",	MTKIT_PREF_TYPE_DIR, NULL, NULL, NULL, 0, NULL, NULL },
	{ "fpick_dir_9",	MTKIT_PREF_TYPE_DIR, NULL, NULL, NULL, 0, NULL, NULL },
	{ "fpick_dir_10",	MTKIT_PREF_TYPE_DIR, NULL, NULL, NULL, 0, NULL, NULL },
	{ "fpick_dir_11",	MTKIT_PREF_TYPE_DIR, NULL, NULL, NULL, 0, NULL, NULL },
	{ "fpick_dir_12",	MTKIT_PREF_TYPE_DIR, NULL, NULL, NULL, 0, NULL, NULL },
	{ "fpick_dir_13",	MTKIT_PREF_TYPE_DIR, NULL, NULL, NULL, 0, NULL, NULL },
	{ "fpick_dir_14",	MTKIT_PREF_TYPE_DIR, NULL, NULL, NULL, 0, NULL, NULL },
	{ "fpick_dir_15",	MTKIT_PREF_TYPE_DIR, NULL, NULL, NULL, 0, NULL, NULL },
	{ "fpick_dir_16",	MTKIT_PREF_TYPE_DIR, NULL, NULL, NULL, 0, NULL, NULL },

	{ "fpick_col1",		MTKIT_PREF_TYPE_INT, "250", NULL, NULL, 0, NULL, NULL },
	{ "fpick_col2",		MTKIT_PREF_TYPE_INT, "64", NULL, NULL, 0, NULL, NULL },
	{ "fpick_col3",		MTKIT_PREF_TYPE_INT, "80", NULL, NULL, 0, NULL, NULL },
	{ "fpick_col4",		MTKIT_PREF_TYPE_INT, "150", NULL, NULL, 0, NULL, NULL },

	{ NULL }
	};


	return mtkit_prefs_add ( prefs, prefs_table, prefs_prefix );
}

void mtgex_fpick_set_filename (
	GtkWidget	* const	fp,
	char	const	* const	name,
	int		const	raw
	)
{
	fpicker		* const	win = gtk_object_get_data ( GTK_OBJECT ( fp ),
				FP_DATA_KEY);


	if ( raw || ! name )
	{
		gtk_entry_set_text ( GTK_ENTRY ( win->file_entry ), name );

		return;
	}


	char		* abs_filename = realpath ( name, NULL ),
			* tcp;


	if ( ! abs_filename )
	{
		// File doesn't currently exist
		abs_filename = strdup ( name );
	}

	if ( ! abs_filename )
	{
		fpick_scan_directory ( win, "" );
		gtk_entry_set_text ( GTK_ENTRY ( win->file_entry ), "" );

		return;
	}

	tcp = strrchr ( name, MTKIT_DIR_SEP );
	if ( tcp && tcp[1] == 0 )
	{
		/*
		Input has a trailing '/' so realpath has
		stripped this away.  We now know we are dealing with a
		pure directory name with no file.
		*/

		tcp = NULL;
	}
	else
	{
		// Separate the directory name from the file name

		tcp = strrchr ( abs_filename, MTKIT_DIR_SEP );
		if ( tcp )
		{
			tcp[0] = 0;
		}
	}

	if ( ! fpick_scan_directory ( win, abs_filename ) )
	{
		// Directory not found so scan empty name
		fpick_scan_directory ( win, "" );
		gtk_entry_set_text ( GTK_ENTRY ( win->file_entry ), "" );

		free ( abs_filename );

		return;
	}

	if ( tcp )
	{
		/* Name is in locale encoding on input */
		char	* alloc = mtgex_gtkuncpy ( NULL, tcp + 1, 0 );


		gtk_entry_set_text ( GTK_ENTRY ( win->file_entry ), alloc );

		free ( alloc );
	}

	free ( abs_filename );
}

void mtgex_fpick_destroy (
	GtkWidget	* const	fp
	)
{
	fpicker		* win;
	GtkCListColumn	* col;
	char		txt [ 256 ],
			buf [ PATHBUF ];
	int		i;


	win = gtk_object_get_data ( GTK_OBJECT ( fp ), FP_DATA_KEY );
	col = GTK_CLIST ( win->clist)->column;

	for ( i = 0; i < FPICK_COMBO_ITEMS; i++ )
	{
		// Remember recently used directories

		mtgex_gtkncpy ( buf, win->combo_items[i], sizeof ( buf ) );
		snprintf ( txt, sizeof ( txt ), "%sfpick_dir_%i",
			fpick_prefix_buf, i + 1 );
		mtkit_prefs_set_str ( fpick_main_prefs, txt, buf );
	}

	for ( i = 0; i < FPICK_CLIST_COLS; i++ )
	{
		// Remember column widths

		snprintf ( txt, sizeof ( txt ), "%sfpick_col%i",
			fpick_prefix_buf, i + 1 );
		mtkit_prefs_set_int ( fpick_main_prefs, txt, col[i].width );
	}

	snprintf ( txt, sizeof ( txt ), "%sfpick_case_insensitive",
		fpick_prefix_buf );
	mtkit_prefs_set_int ( fpick_main_prefs, txt, case_insensitive );

	snprintf ( txt, sizeof ( txt), "%sfpick_show_hidden", fpick_prefix_buf);
	mtkit_prefs_set_int ( fpick_main_prefs, txt, win->show_hidden );

	free ( win );
	mtgex_destroy_dialog ( fp );
}

static void fpick_newdir_destroy (
	GtkWidget	* const	widget,
	int		* const	data
	)
{
	data[0] = 10;
	gtk_widget_destroy ( widget );
}

static void fpick_newdir_cancel (
	GtkWidget	* const	ARG_UNUSED ( widget ),
	int		* const	data
	)
{
	data[0] = 2;
}

static void fpick_newdir_ok (
	GtkWidget	* const	ARG_UNUSED ( widget ),
	int		* const	data
	)
{
	data[0] = 1;
}

static void fpick_create_newdir (
	fpicker		* const	fp
	)
{
	char		fnm [ PATHBUF ];
	GtkWidget	* win,
			* button,
			* label,
			* entry;
	GtkAccelGroup	* ag;
	int		res = 0;


	ag = gtk_accel_group_new ();

	win = gtk_dialog_new ();
	gtk_window_set_title ( GTK_WINDOW ( win ), _("Create Directory") );
	gtk_window_set_modal ( GTK_WINDOW ( win ), TRUE );
	gtk_window_set_position ( GTK_WINDOW ( win ), GTK_WIN_POS_CENTER );
	gtk_container_set_border_width ( GTK_CONTAINER ( win ), 6 );
	gtk_signal_connect ( GTK_OBJECT ( win ), "destroy",
		GTK_SIGNAL_FUNC ( fpick_newdir_destroy ), &res );

	label = gtk_label_new ( _("Enter the name of the new directory") );
	gtk_label_set_line_wrap ( GTK_LABEL ( label ), TRUE );
	gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( win )->vbox ), label, TRUE,
		FALSE, 8 );

	entry = mtgex_xpack5 ( GTK_DIALOG ( win )->vbox,
		gtk_entry_new_with_max_length ( 256 ) );

	button = mtgex_add_a_button ( _("Cancel"), 2,
		GTK_DIALOG ( win )->action_area, TRUE );
	gtk_signal_connect ( GTK_OBJECT ( button ), "clicked",
		GTK_SIGNAL_FUNC ( fpick_newdir_cancel ), &res );
	gtk_widget_add_accelerator ( button, "clicked", ag, GDK_Escape, 0,
		(GtkAccelFlags) 0);

	button = mtgex_add_a_button ( _("OK"), 2,
		GTK_DIALOG ( win )->action_area, TRUE );
	gtk_signal_connect ( GTK_OBJECT ( button ), "clicked",
		GTK_SIGNAL_FUNC ( fpick_newdir_ok ), &res );
	gtk_widget_add_accelerator ( button, "clicked", ag, GDK_KP_Enter, 0,
		(GtkAccelFlags) 0);
	gtk_widget_add_accelerator ( button, "clicked", ag, GDK_Return, 0,
		(GtkAccelFlags) 0);

	gtk_window_set_transient_for ( GTK_WINDOW ( win ),
		GTK_WINDOW ( fpick_main_window ) );
	gtk_widget_show_all ( win );
	gdk_window_raise ( win->window );
	gtk_widget_grab_focus ( entry );

	gtk_window_add_accel_group ( GTK_WINDOW ( win ), ag );

	while ( ! res )
	{
		gtk_main_iteration ();
	}

	if ( res == 2 )
	{
		gtk_widget_destroy ( win );
	}
	else if ( res == 1 )
	{
		size_t		len;


		mtkit_strnncpy ( fnm, fp->txt_directory, sizeof ( fnm ) );
		len = strlen ( fnm );
		mtgex_gtkncpy ( fnm + len,
			gtk_entry_get_text ( GTK_ENTRY ( entry ) ),
			sizeof ( fnm ) - len );

		gtk_widget_destroy ( win );

		if ( mkdir ( fnm, 0777 ) )
		{
			mtgex_alert_box ( _("Error"),
				_("Unable to create directory"),
				_("OK"), NULL, NULL, fpick_main_window );
		}
		else
		{
			fpick_scan_directory ( fp, fp->txt_directory );
		}
	}
}

static void fpick_iconbar_click (
	GtkWidget	* const	widget,
	gpointer	const	data
	)
{
	fpicker		* fp = gtk_object_get_data (
				GTK_OBJECT ( widget->parent ), FP_DATA_KEY );
	char		nm [ PATHBUF ],
			fnm [ PATHBUF ];
	int		j = (int)(intptr_t)data;


	if ( ! fp )
	{
		return;
	}

	switch ( j )
	{
	case FPICK_ICON_UP:
		fpick_enter_dir_via_list ( fp, ".." );
		break;

	case FPICK_ICON_HOME:
		mtgex_gtkncpy ( nm, gtk_entry_get_text (
			GTK_ENTRY ( fp->file_entry ) ), sizeof ( nm ) );

		snprintf ( fnm, sizeof ( fnm ), "%s%c", mtkit_file_home (),
			MTKIT_DIR_SEP );

		mtgex_fpick_set_filename ( fp->window, fnm, FALSE );

		gtk_entry_set_text ( GTK_ENTRY ( fp->file_entry ), nm );
		break;

	case FPICK_ICON_DIR:
		fpick_create_newdir ( fp );
		break;

	case FPICK_ICON_HIDDEN:
		fp->show_hidden = gtk_toggle_button_get_active (
			GTK_TOGGLE_BUTTON ( widget ) );
		fpick_scan_directory ( fp, fp->txt_directory );
		break;

	case FPICK_ICON_CASE:
		case_insensitive = gtk_toggle_button_get_active (
			GTK_TOGGLE_BUTTON ( widget ) );
		fpick_sort_files ( fp );
		break;
	}
}

void mtgex_fpick_setup (
	GtkWidget	* const	fp,
	GtkWidget	* const	xtra,
	GtkSignalFunc	const	ok_fn,
	GtkSignalFunc	const	cancel_fn
	)
{
	fpicker		* fpick;


	fpick = gtk_object_get_data ( GTK_OBJECT ( fp ), FP_DATA_KEY );

	gtk_signal_connect_object ( GTK_OBJECT ( fpick->ok_button ),
		"clicked", ok_fn, GTK_OBJECT ( fp ) );
	gtk_signal_connect_object ( GTK_OBJECT ( fpick->cancel_button ),
		"clicked", cancel_fn, GTK_OBJECT ( fp ) );
	gtk_signal_connect_object ( GTK_OBJECT ( fpick->window ),
		"delete_event", cancel_fn, GTK_OBJECT ( fp ) );

	if ( xtra )
	{
		mtgex_pack ( fpick->main_vbox, xtra );
	}
}

char const * mtgex_fpick_get_filename (
	GtkWidget	* const	fp,
	int		const	raw
	)
{
	fpicker		* fpick;
	char	const	* ct;
	char		* ft,
			* dir;


	fpick = gtk_object_get_data ( GTK_OBJECT ( fp ), FP_DATA_KEY );
	ct = gtk_entry_get_text ( GTK_ENTRY ( fpick->file_entry ) );
	dir = fpick->txt_directory;

	if ( raw )
	{
		return ct;
	}

	/* Convert filename to locale */
	ft = mtgex_gtkncpy ( NULL, ct, 0 );
	snprintf ( fpick->txt_file, sizeof ( fpick->txt_file ), "%s%s", dir,
		ft );
	free ( ft );

	return fpick->txt_file;
}

