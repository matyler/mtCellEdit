/*
	Copyright (C) 2008-2016 Mark Tyler

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

#include "tk_gtk2.h"




enum	// File selector quarks
{
	_FS_XTRA_HBOX		= 1,
	_FS_SPIN_FONT_SIZE,
	_FS_BMENU_BOX_FILETYPE
};



static int	last_export_type	= CUI_SHEET_EXPORT_TSV_QUOTED,
		last_graph_type		= CUI_GRAPH_TYPE_PDF;



void pressed_help (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data ),
	gint		const	ARG_UNUSED ( item )
	)
{
	char	const	* browser;
	char	const	* file;
	int		res;


	browser = prefs_get_string ( GUI_INIFILE_HELP_BROWSER );

	if ( ! browser[0] )
	{
		browser = getenv ( "BROWSER" );
	}

	if ( ! browser || ! browser[0] )
	{
		browser = "firefox";
	}

	file = prefs_get_string ( GUI_INIFILE_HELP_FILE );

	if ( ! mtkit_file_readable ( file ) )
	{
		mtgex_alert_box ( _( "Error" ),
			_( "I am unable to find the documentation.  "
			"You need to set the correct location in the "
			"Preferences." ),
			_("OK"), NULL, NULL, global.main_window );

		return;
	}


	gchar		* argv_free[3] = {
				strdup ( browser ),
				strdup ( file ),
				NULL
				};


	res = g_spawn_async ( NULL, argv_free, NULL, G_SPAWN_SEARCH_PATH, NULL,
		NULL, NULL, NULL );

	free ( argv_free[0] );
	free ( argv_free[1] );
	argv_free[0] = NULL;
	argv_free[1] = NULL;


	if ( ! res )
	{
		mtgex_alert_box( _("Error"),
			_( "There was a problem running the HTML browser.  "
			"You need to set the correct program name in the "
			"Preferences window." ),
			_("OK"), NULL, NULL, global.main_window );

		return;
	}
}

static gboolean click_about_end (
	GtkWidget	* const	widget,
	GdkEvent	* const	ARG_UNUSED ( event ),
	gpointer	const	ARG_UNUSED ( data )
	)
{
	gtk_widget_destroy ( widget );

	return FALSE;
}

void pressed_about (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data ),
	gint		const	ARG_UNUSED ( item )
	)
{
	GtkWidget	* window,
			* vbox,
			* button,
			* textview,
			* sw;
	GtkAccelGroup	* ag;
	GtkTextBuffer	* buffer;
	gchar	const	* about_text = VERSION"\n"
	"\n"
	"Copyright (C) 2008-2016 Mark Tyler.\n"
	"\n"
	"This program is free software: you can redistribute it and/or modify "
	"it under the terms of the GNU General Public License as published by "
	"the Free Software Foundation, either version 3 of the License, or "
	"(at your option) any later version.\n"
	"\n"
	"This program is distributed in the hope that it will be useful, "
	"but WITHOUT ANY WARRANTY; without even the implied warranty of "
	"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
	"GNU General Public License for more details.\n"
	"\n"
	"You should have received a copy of the GNU General Public License "
	"along with this program.  If not, see http://www.gnu.org/licenses/\n"
	;


	ag = gtk_accel_group_new ();

	window = mtgex_add_a_window ( GTK_WINDOW_TOPLEVEL, _("About"),
		GTK_WIN_POS_CENTER, TRUE );
	gtk_window_set_transient_for ( GTK_WINDOW ( window ),
		GTK_WINDOW ( global.main_window ) );

	vbox = gtk_vbox_new ( FALSE, 0 );
	gtk_container_add ( GTK_CONTAINER ( window ), vbox );
	gtk_widget_show ( vbox );

	sw = gtk_scrolled_window_new ( NULL, NULL );
	gtk_widget_show ( sw );
	gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( sw ),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
	gtk_box_pack_start ( GTK_BOX ( vbox ), sw, TRUE, TRUE, 0 );

	textview = gtk_text_view_new ();
	gtk_widget_show ( textview );
	gtk_container_add ( GTK_CONTAINER ( sw ), textview );
	gtk_text_view_set_editable ( GTK_TEXT_VIEW ( textview ), FALSE );
	gtk_text_view_set_cursor_visible ( GTK_TEXT_VIEW ( textview ), FALSE );
	gtk_text_view_set_wrap_mode ( GTK_TEXT_VIEW ( textview ),
		GTK_WRAP_WORD );

	buffer = gtk_text_view_get_buffer ( GTK_TEXT_VIEW ( textview ) );
	gtk_text_buffer_set_text ( GTK_TEXT_BUFFER ( buffer ), about_text, -1 );

	mtgex_add_hseparator ( vbox, -2, 10 );

	button = mtgex_pack ( vbox, gtk_button_new_with_label ( _("Close") ) );
	gtk_widget_add_accelerator ( button, "clicked", ag, GDK_KP_Enter, 0,
		(GtkAccelFlags) 0 );
	gtk_widget_add_accelerator ( button, "clicked", ag, GDK_Return, 0,
		(GtkAccelFlags) 0 );
	gtk_widget_add_accelerator ( button, "clicked", ag, GDK_Escape, 0,
		(GtkAccelFlags) 0 );
	GTK_WIDGET_SET_FLAGS ( button, GTK_CAN_DEFAULT );

	gtk_signal_connect_object ( GTK_OBJECT ( button ), "clicked",
		GTK_SIGNAL_FUNC ( click_about_end ), GTK_OBJECT ( window ) );
	gtk_signal_connect ( GTK_OBJECT ( window ), "delete_event",
		GTK_SIGNAL_FUNC ( click_about_end ), NULL);
	gtk_widget_show ( button );

	gtk_window_set_default_size ( GTK_WINDOW ( window ), 512, 400 );

	gtk_widget_show ( window );
	gtk_window_add_accel_group ( GTK_WINDOW ( window ), ag );
}



///	FILE SELECTION WINDOW

static int check_file (
	char	const	* const	fname
	)		// Does file already exist?  Ask if OK to overwrite
{
	char		* msg,
			* f8;
	int		res = 0;


	if ( mtkit_file_readable ( fname ) )
	{
		f8 = mtgex_gtkuncpy ( NULL, fname, 0 );

		msg = g_strdup_printf (
		_("File: %s already exists. Do you want to overwrite it?"),
			f8);

		res = mtgex_alert_box ( _("File Found"), msg,
			_("NO"), _("YES"), NULL, global.main_window ) != 2;

		g_free ( msg );
		g_free ( f8 );
	}

	return res;
}

static gboolean fs_destroy (
	GtkWidget	* const	fs
	)
{
	mtgex_store_window_position ( fs, prefs_file (),
		GUI_INIFILE_MAIN_FPICK"_window" );

	mtgex_fpick_destroy ( fs );

	return FALSE;
}

static void fs_ok (
	GtkWidget	* const	fs
	)
{
	GtkWidget	* xtra_hbox,
			* w;
	char		fname [ PATHBUF ];
	int		action_type,
			s;
	gpointer	gp;


	gp = gtk_object_get_user_data ( GTK_OBJECT ( fs ) );
	action_type = (int)(intptr_t) gp;
	xtra_hbox = (GtkWidget *) g_object_get_qdata ( G_OBJECT ( fs ),
		_FS_XTRA_HBOX );

	/* Needed to show progress in Windows GTK+2 */
	gtk_window_set_modal ( GTK_WINDOW ( fs ), FALSE);

	/* Looks better if no dialog under progressbar */
	mtgex_store_window_position ( fs, prefs_file (),
		GUI_INIFILE_MAIN_FPICK"_window" );
	gtk_widget_hide ( fs );

	/* File extension */
	mtkit_strnncpy ( fname, mtgex_fpick_get_filename ( fs, TRUE ),
		sizeof ( fname ) );
	mtgex_fpick_set_filename ( fs, fname, TRUE );

	mtkit_strnncpy ( fname, mtgex_fpick_get_filename ( fs, FALSE),
		sizeof ( fname ) );

	switch (action_type)
	{
	case FS_LOAD_PROJECT:
		if ( project_load ( fname ) )
		{
			goto redo;
		}
		break;

	case FS_SAVE_PROJECT:
		if ( check_file ( fname ) )
		{
			goto redo;
		}

		w = (GtkWidget *) g_object_get_qdata ( G_OBJECT ( xtra_hbox ),
			_FS_BMENU_BOX_FILETYPE );

		if ( w )
		{
			s = 1 + mtgex_bmenu_get_value ( w );
		}
		else
		{
			s = global.file->type;
		}

		if ( project_save ( fname, s ) )
		{
			goto redo;
		}
		break;

	case FS_IMPORT_PROJECT:
		if ( project_import ( fname ) )
		{
			goto redo;
		}
		break;

	case FS_EXPORT_SHEET:
		if ( check_file ( fname ) )
		{
			goto redo;
		}

		w = (GtkWidget *) g_object_get_qdata ( G_OBJECT ( xtra_hbox ),
			_FS_BMENU_BOX_FILETYPE );

		if ( w )
		{
			s = 1 + mtgex_bmenu_get_value ( w );
		}
		else
		{
			s = global.file->type;
		}

		if ( project_export ( fname, s ) )
		{
			goto redo;
		}
		break;

	case FS_EXPORT_SHEET_OUTPUT:
		if ( check_file ( fname ) )
		{
			goto redo;
		}

		w = (GtkWidget *) g_object_get_qdata ( G_OBJECT ( xtra_hbox ),
			_FS_BMENU_BOX_FILETYPE );
		if ( w )
		{
			last_export_type = mtgex_bmenu_get_value ( w );
			if ( project_export_output ( fname, last_export_type ) )
			{
				goto redo;
			}
		}
		break;

	case FS_EXPORT_GRAPH:
		if ( check_file ( fname ) )
		{
			goto redo;
		}

		w = (GtkWidget *) g_object_get_qdata ( G_OBJECT ( xtra_hbox ),
			_FS_BMENU_BOX_FILETYPE );
		if ( w )
		{
			last_graph_type = mtgex_bmenu_get_value ( w );
			if ( project_export_graph ( fname, last_graph_type ) )
			{
				goto redo;
			}
		}
		break;
	}

	mtgex_fpick_destroy ( fs );

	return;

redo:
	gtk_widget_show ( fs );
	gtk_window_set_modal ( GTK_WINDOW ( fs ), TRUE );
}

static void fs_setup (
	GtkWidget	* const	fs,
	GtkWidget	* const	xtra_hbox,
	int		const	action_type,
	char	const	* const	filename
	)
{
	char		txt [ PATHBUF ];
	char	const	* last = NULL;


	if (	(	action_type == FS_SAVE_PROJECT ||
			action_type == FS_EXPORT_SHEET ) &&
		filename &&
		filename[0]
		)
	{
		// If we have a filename and saving

		mtkit_strnncpy ( txt, filename, sizeof ( txt ) );
	}
	else
	{
		if ( filename && filename[0] )
		{
			char		* c;


			mtkit_strnncpy ( txt, filename, sizeof ( txt ) );
			c = strrchr ( txt, MTKIT_DIR_SEP );

			if ( c )
			{
				c[1] = 0;
			}
		}
		else
		{
			last = prefs_get_string ( GUI_INIFILE_LAST_DIR );

			if ( ! last[0] )
			{
				last = mtkit_file_home ();
			}

			snprintf ( txt, sizeof ( txt ), "%s%c", last,
				MTKIT_DIR_SEP );
		}
	}

	gtk_window_set_modal ( GTK_WINDOW ( fs ), TRUE);
	mtgex_restore_window_position ( fs, prefs_file (),
		GUI_INIFILE_MAIN_FPICK"_window" );

	gtk_object_set_user_data ( GTK_OBJECT ( fs ),
		(gpointer)(intptr_t)action_type );
	mtgex_fpick_setup ( fs, xtra_hbox, GTK_SIGNAL_FUNC ( fs_ok ),
		GTK_SIGNAL_FUNC ( fs_destroy ) );

	mtgex_fpick_set_filename ( fs, txt, FALSE );

	if ( xtra_hbox )
	{
		g_object_set_qdata ( G_OBJECT ( fs ), _FS_XTRA_HBOX,
			xtra_hbox );
	}

	gtk_widget_show ( fs );
	gtk_window_set_transient_for ( GTK_WINDOW ( fs ),
		GTK_WINDOW ( global.main_window ) );

	// Needed to ensure window is at the top
	gdk_window_raise ( fs->window );
}

void file_selector (
	int		const	action_type,
	GtkWidget	* const	main_window,
	char	const	* const	filename
	)
{
	GtkWidget	* hbox = NULL,
			* label,
			* bmenu;
	char	const	* title = NULL;
	int		fpick_flags = MTGEX_FPICK_ENTRY,
			num;


	switch ( action_type )
	{
	case FS_LOAD_PROJECT:
		title = _("Load Project File");
		fpick_flags = MTGEX_FPICK_LOAD;
		break;

	case FS_SAVE_PROJECT:
		title = _("Save Project File");
		break;

	case FS_IMPORT_PROJECT:
		title = _("Import Project File");
		fpick_flags = MTGEX_FPICK_LOAD;
		break;

	case FS_EXPORT_SHEET:
		title = _("Export Sheet");
		break;

	case FS_EXPORT_SHEET_OUTPUT:
		title = _("Export Sheet Output");
		break;

	case FS_EXPORT_GRAPH:
		title = _("Export Graph");
		break;
	}

	if (	action_type == FS_SAVE_PROJECT ||
		action_type == FS_EXPORT_SHEET
		)
	{
		hbox = gtk_hbox_new ( FALSE, 0 );
		gtk_widget_show ( hbox );

		label = gtk_label_new ( _("File Type:") );
		gtk_widget_show ( label );
		gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 5 );

		bmenu = mtgex_bmenu_new ();
		gtk_box_pack_start ( GTK_BOX ( hbox ), bmenu, FALSE, FALSE, 5 );

		for (	num = CED_FILE_TYPE_NONE + 1;
			num <= CED_FILE_TYPE_LEDGER_VAL_BOOK;
			num ++
			)
		{
			mtgex_bmenu_add_item ( bmenu,
				ced_file_type_text ( num ) );
		}

		mtgex_bmenu_set_value ( bmenu, global.file->type - 1 );

		g_object_set_qdata ( G_OBJECT ( hbox ), _FS_BMENU_BOX_FILETYPE,
			bmenu );
	}

	if ( action_type == FS_EXPORT_SHEET_OUTPUT )
	{
		char	const * formats[CUI_SHEET_EXPORT_TOTAL] = {
			"EPS",
			"HTML",
			"PDF",
			"PDF Paged",
			"PNG",
			"PS",
			"SVG",
			"TSV",
			"TSV Quoted"
			};

		hbox = gtk_hbox_new ( FALSE, 0 );
		gtk_widget_show ( hbox );

		label = gtk_label_new ( _("File Type:") );
		gtk_widget_show ( label );
		gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 5 );

		bmenu = mtgex_bmenu_new ();
		gtk_box_pack_start ( GTK_BOX ( hbox ), bmenu, FALSE, FALSE, 5 );

		for ( num = 0; num < CUI_SHEET_EXPORT_TOTAL; num ++ )
		{
			mtgex_bmenu_add_item ( bmenu, formats[num] );
		}

		mtgex_bmenu_set_value ( bmenu, last_export_type );

		g_object_set_qdata ( G_OBJECT ( hbox ), _FS_BMENU_BOX_FILETYPE,
			bmenu );
	}

	if ( action_type == FS_EXPORT_GRAPH )
	{
		char	const	* formats[CUI_GRAPH_TYPE_TOTAL] = {
				"EPS",
				"PDF",
				"PNG",
				"PS",
				"SVG",
				};


		hbox = gtk_hbox_new ( FALSE, 0 );
		gtk_widget_show ( hbox );

		label = gtk_label_new ( _("File Type:") );
		gtk_widget_show ( label );
		gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 5 );

		bmenu = mtgex_bmenu_new ();
		gtk_box_pack_start ( GTK_BOX ( hbox ), bmenu, FALSE, FALSE, 5 );

		for ( num = 0; num < CUI_GRAPH_TYPE_TOTAL; num ++ )
		{
			mtgex_bmenu_add_item ( bmenu, formats[num] );
		}

		mtgex_bmenu_set_value ( bmenu, last_graph_type );

		g_object_set_qdata ( G_OBJECT ( hbox ), _FS_BMENU_BOX_FILETYPE,
			bmenu );
	}

	fs_setup ( mtgex_fpick_new ( title, fpick_flags, main_window,
		prefs_file (), GUI_INIFILE_MAIN_FPICK_PRE ),
		hbox, action_type, filename );
}

