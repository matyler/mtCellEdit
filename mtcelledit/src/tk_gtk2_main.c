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
#include "icon.xpm"



GUI_Global	global		= { 0 };



static void delete_event (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	int		i;


	if ( ! GTK_WIDGET_SENSITIVE ( global.main_window ) )
	{
		return;
	}

	i = check_for_changes ();
	if ( i != -10 && i != 2 )
	{
		return;
	}

	i = prefs_get_int ( GUI_INIFILE_MAIN_WINDOW"_state" );

	if ( i != 0 )
	{
		// Don't store the maximized geometry

		mtgex_store_window_position ( global.main_window,
			prefs_file (), GUI_INIFILE_MAIN_WINDOW );
	}

	find_gui_save_settings ();
	pane_pair_set ( PANE_PAIR_STORE_POSITION );

	global.cedview->ren.sheet = NULL;
	gtk_main_quit ();
}


static gboolean window_state_event (
	GtkWidget	* const	ARG_UNUSED ( widget ),
	GdkEventWindowState * const event,
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	int		i = 1;


	if ( event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED )
	{
		i = 0;
	}

	prefs_set_int ( GUI_INIFILE_MAIN_WINDOW"_state", i );

	return FALSE;			// Allow event to propagate
}

static void gui_main_init (
	char	const	* arg_filename
	)
{
	CedView		* view;
	int		i;
	char		* menu_refdef[_MENU_TOTAL] = {
			_("/File/sepR"),
			_("/File/1"),
			_("/File/2"),
			_("/File/3"),
			_("/File/4"),
			_("/File/5"),
			_("/File/6"),
			_("/File/7"),
			_("/File/8"),
			_("/File/9"),
			_("/File/10"),
			_("/File/11"),
			_("/File/12"),
			_("/File/13"),
			_("/File/14"),
			_("/File/15"),
			_("/File/16"),
			_("/File/17"),
			_("/File/18"),
			_("/File/19"),
			_("/File/20"),
			_("/Edit/Undo"),
			_("/Edit/Redo"),
			_("/Edit/Cut"),
			_("/Edit/Copy"),
			_("/Edit/Paste"),
			_("/Edit/Clear"),
			_("/Edit/Select All"),
			_("/Edit/Clear Content"),
			_("/Edit/Clear Preferences"),
			_("/Row/Insert"),
			_("/Row/Delete"),
			_("/Column/Insert"),
			_("/Column/Delete"),
			_("/Sheet/Lock"),
			_("/Sheet/Freeze Panes")
		};
	GtkWidget	* menubar,
			* vbox_main,
			* vbox,
			* table,
			* entry,
			* hbox,
			* hbox_top,
			* label,
			* pane;
	GtkAccelGroup	* accel_group;

	GtkItemFactory	* item_factory;
	GtkItemFactoryEntry menu_items[] = {
{ _("/_File"),			NULL,		NULL, 0, "<Branch>", NULL },
{ _("/File/tear"),		NULL,		NULL, 0, "<Tearoff>", NULL },
{ _("/File/New"),		"<control>N",	pressed_new_project,0, "<StockItem>", GTK_STOCK_NEW },
{ _("/File/Open ..."),		"<control>O",	pressed_open_project, 0, "<StockItem>", GTK_STOCK_OPEN },
{ _("/File/sep0"),		NULL,		NULL, 0, "<Separator>", NULL },
{ _("/File/Import ..."),	"",	pressed_import_project, 0, "<StockItem>", GTK_STOCK_OPEN },
{ _("/File/sep1"),		NULL,		NULL, 0, "<Separator>", NULL },
{ _("/File/Save"),		"<control>S",	pressed_save_project,0, "<StockItem>", GTK_STOCK_SAVE },
{ _("/File/Save As ..."),	"<shift><control>S", pressed_save_project_as, 0, "<StockItem>", GTK_STOCK_SAVE_AS },
{ menu_refdef[_MENU_REC_SEP],	NULL,		NULL, 0, "<Separator>", NULL },
{ menu_refdef[_MENU_REC_1],	"<control>1",	pressed_recent, 1, NULL, NULL },
{ menu_refdef[_MENU_REC_2],	"<control>2",	pressed_recent, 2, NULL, NULL },
{ menu_refdef[_MENU_REC_3],	"<control>3",	pressed_recent, 3, NULL, NULL },
{ menu_refdef[_MENU_REC_4],	"<control>4",	pressed_recent, 4, NULL, NULL },
{ menu_refdef[_MENU_REC_5],	"<control>5",	pressed_recent, 5, NULL, NULL },
{ menu_refdef[_MENU_REC_6],	"<control>6",	pressed_recent, 6, NULL, NULL },
{ menu_refdef[_MENU_REC_7],	"<control>7",	pressed_recent, 7, NULL, NULL },
{ menu_refdef[_MENU_REC_8],	"<control>8",	pressed_recent, 8, NULL, NULL },
{ menu_refdef[_MENU_REC_9],	"<control>9",	pressed_recent, 9, NULL, NULL },
{ menu_refdef[_MENU_REC_10],	"<control>0",	pressed_recent, 10, NULL, NULL },
{ menu_refdef[_MENU_REC_11],	NULL,		pressed_recent, 11, NULL, NULL },
{ menu_refdef[_MENU_REC_12],	NULL,		pressed_recent, 12, NULL, NULL },
{ menu_refdef[_MENU_REC_13],	NULL,		pressed_recent, 13, NULL, NULL },
{ menu_refdef[_MENU_REC_14],	NULL,		pressed_recent, 14, NULL, NULL },
{ menu_refdef[_MENU_REC_15],	NULL,		pressed_recent, 15, NULL, NULL },
{ menu_refdef[_MENU_REC_16],	NULL,		pressed_recent, 16, NULL, NULL },
{ menu_refdef[_MENU_REC_17],	NULL,		pressed_recent, 17, NULL, NULL },
{ menu_refdef[_MENU_REC_18],	NULL,		pressed_recent, 18, NULL, NULL },
{ menu_refdef[_MENU_REC_19],	NULL,		pressed_recent, 19, NULL, NULL },
{ menu_refdef[_MENU_REC_20],	NULL,		pressed_recent, 20, NULL, NULL },
{ _("/File/sep3"),		NULL,		NULL, 0, "<Separator>", NULL },
{ _("/File/Quit"),		"<control>Q",	delete_event, 0, "<StockItem>", GTK_STOCK_QUIT },

{ _("/_Edit"),			NULL,		NULL, 0, "<Branch>", NULL },
{ _("/Edit/tear"),		NULL,		NULL, 0, "<Tearoff>", NULL },
{ menu_refdef[_MENU_UNDO],	"<control>Z",	pressed_undo, 0, "<StockItem>", GTK_STOCK_UNDO },
{ menu_refdef[_MENU_REDO],	"<control>R", pressed_redo, 0, "<StockItem>", GTK_STOCK_REDO },
{ _("/Edit/sep1"),		NULL,		NULL, 0, "<Separator>", NULL },
{ menu_refdef[_MENU_CUT],	"<control>X",	pressed_cut, 0, "<StockItem>", GTK_STOCK_CUT },
{ menu_refdef[_MENU_COPY],	"<control>C",	pressed_copy, 0, "<StockItem>", GTK_STOCK_COPY },
{ _("/Edit/Copy As Values"),	"<shift><control>C", pressed_copy_values, 0, NULL, NULL },
{ _("/Edit/Copy As Output"),	NULL,		pressed_copy_output, 0, NULL, NULL },
{ _("/Edit/sep2"),		NULL,		NULL, 0, "<Separator>", NULL },
{ _("/Edit/Transform Clipboard"), NULL,		NULL, 0, "<Branch>", NULL },
{ _("/Edit/Transform Clipboard/tear"), NULL,	NULL, 0, "<Tearoff>", NULL },
{ _("/Edit/Transform Clipboard/Transpose"), NULL, pressed_transform_clipboard, 0, NULL, NULL },
{ _("/Edit/Transform Clipboard/Flip Horizontally"), NULL, pressed_transform_clipboard, 1, NULL, NULL },
{ _("/Edit/Transform Clipboard/Flip Vertically"), NULL, pressed_transform_clipboard, 2, NULL, NULL },
{ _("/Edit/Transform Clipboard/Rotate Clockwise"), NULL, pressed_transform_clipboard, 3, NULL, NULL },
{ _("/Edit/Transform Clipboard/Rotate Anticlockwise"), NULL, pressed_transform_clipboard, 4, NULL, NULL },
{ _("/Edit/sep3"),		NULL,		NULL, 0, "<Separator>", NULL },
{ menu_refdef[_MENU_PASTE],	"<control>V",	pressed_paste, 0, "<StockItem>", GTK_STOCK_PASTE },
{ _("/Edit/Paste Content"),	"<control>F7",	pressed_paste_content, 0, NULL, NULL },
{ _("/Edit/Paste Preferences"), "F7",		pressed_paste_prefs, 0, NULL, NULL },
{ _("/Edit/sep4"),		NULL,		NULL, 0, "<Separator>", NULL },
{ menu_refdef[_MENU_CLEAR],	"Delete",	pressed_clear, 0, "<StockItem>", GTK_STOCK_CLEAR },
{ menu_refdef[_MENU_CLEAR_CONTENT], "BackSpace", pressed_clear_contents, 0, NULL, NULL },
{ menu_refdef[_MENU_CLEAR_PREFS], "<control>BackSpace",	pressed_clear_prefs, 0, NULL, NULL },
{ _("/Edit/sep5"),		NULL,		NULL, 0, "<Separator>", NULL },
{ _("/Edit/Fix 2-digit Years"), NULL,		pressed_fix_2dyears, 0, NULL, NULL },
{ menu_refdef[_MENU_SELECT_ALL], "<control>A",	pressed_select_all, 0, NULL, NULL },

{ _("/_Sheet"),			NULL,		NULL, 0, "<Branch>", NULL },
{ _("/Sheet/tear"),		NULL,		NULL, 0, "<Tearoff>", NULL },
{ _("/Sheet/New"),		"<control>T",	pressed_sheet_new, 0, "<StockItem>", GTK_STOCK_NEW },
{ _("/Sheet/Duplicate"),	"<shift><control>T", pressed_sheet_duplicate, 0, "<StockItem>", GTK_STOCK_COPY },
{ _("/Sheet/sep1"),		NULL,		NULL, 0, "<Separator>", NULL },
{ _("/Sheet/Rename ..."),	"<control>F2",	pressed_sheet_rename, 0, "<StockItem>", GTK_STOCK_EDIT },
{ _("/Sheet/Delete"),		"<control>W",	pressed_sheet_delete, 0, "<StockItem>", GTK_STOCK_DELETE },
{ _("/Sheet/sep2"),		NULL,		NULL, 0, "<Separator>", NULL },
{ _("/Sheet/Export ..."),	NULL,		pressed_sheet_export, 0, "<StockItem>", GTK_STOCK_SAVE_AS },
{ _("/Sheet/Export Output ..."), NULL,		pressed_sheet_export_output, 0, "<StockItem>", GTK_STOCK_SAVE_AS },
{ _("/Sheet/sep3"),		NULL,		NULL, 0, "<Separator>", NULL },
{ menu_refdef[_MENU_SHEET_FREEZE_PANES], NULL,	pressed_sheet_freeze_panes, 0, NULL, NULL },
{ menu_refdef[_MENU_SHEET_LOCKED], "F12",	pressed_sheet_locked, 0, NULL, NULL },
{ _("/Sheet/sep4"),		NULL,		NULL, 0, "<Separator>", NULL },
{ _("/Sheet/Previous"),		"<control>Page_Up", pressed_sheet_previous, 0, NULL, NULL },
{ _("/Sheet/Next"),		"<control>Page_Down", pressed_sheet_next, 0, NULL, NULL },
{ _("/Sheet/sep5"),		NULL,		NULL, 0, "<Separator>", NULL },
{ _("/Sheet/Recalculate Book"), "F4",		pressed_recalculate_book, 0, NULL, NULL },
{ _("/Sheet/Recalculate"),	"F5",		pressed_recalculate, 0, "<StockItem>", GTK_STOCK_REFRESH },

{ _("/_Row"),			NULL,		NULL, 0, "<Branch>", NULL },
{ _("/Row/tear"),		NULL,		NULL, 0, "<Tearoff>", NULL },
{ menu_refdef[_MENU_ROW_INSERT], "<shift>Insert", pressed_row_insert, 0, "<StockItem>", GTK_STOCK_ADD },
{ _("/Row/Insert Paste Height"), NULL,		pressed_row_insert_paste_height, 0, NULL, NULL },
{ _("/Row/sep1"),		NULL,		NULL, 0, "<Separator>", NULL },
{ menu_refdef[_MENU_ROW_DELETE], "<shift>Delete", pressed_row_delete, 0, "<StockItem>", GTK_STOCK_DELETE },
{ _("/Row/sep2"),		NULL,		NULL, 0, "<Separator>", NULL },
{ _("/Row/Sort ..."),		NULL,		pressed_sort,0, "<StockItem>", GTK_STOCK_SORT_ASCENDING },

{ _("/_Column"),		NULL,		NULL, 0, "<Branch>", NULL },
{ _("/Column/tear"),		NULL,		NULL, 0, "<Tearoff>", NULL },
{ menu_refdef[_MENU_COL_INSERT], "<control>Insert", pressed_column_insert, 0, "<StockItem>", GTK_STOCK_ADD },
{ _("/Column/Insert Paste Width"), NULL,	pressed_column_insert_paste_width, 0, NULL, NULL },
{ _("/Column/sep1"),		NULL,		NULL, 0, "<Separator>", NULL },
{ menu_refdef[_MENU_COL_DELETE], "<control>Delete", pressed_column_delete,0, "<StockItem>", GTK_STOCK_DELETE },
{ _("/Column/sep2"),		NULL,		NULL, 0, "<Separator>", NULL },
{ _("/Column/Sort ..."),	NULL,		pressed_sort, 1, "<StockItem>", GTK_STOCK_SORT_ASCENDING },
{ _("/Column/sep3"),		NULL,		NULL, 0, "<Separator>", NULL },
{ _("/Column/Set Width ..."),	"F6",		pressed_column_width, 0, NULL, NULL },
{ _("/Column/Set Width Automatically"),	"<control>F6", pressed_column_width_auto, 0, NULL, NULL },

{ _("/_Options"),		NULL,		NULL, 0, "<Branch>", NULL },
{ _("/Options/tear"),		NULL,		NULL, 0, "<Tearoff>", NULL },
{ _("/Options/Full Screen"),	"F11",		pressed_set_fullscreen, 0, "<ToggleItem>", NULL },
{ _("/Options/Find ..."),	"<control>F",	pressed_find, 0, NULL, NULL },
{ _("/Options/Find Close"),	"<shift><control>F", pressed_find_close, 0, NULL, NULL },
{ _("/Options/Show,Hide Graph"), "<control>G",	pressed_graph, 0, NULL, NULL },
{ _("/Options/sep1"),		NULL,		NULL, 0, "<Separator>", NULL },
{ _("/Options/Edit Cell"),	"F2",		pressed_edit_cell,0, "<StockItem>", GTK_STOCK_EDIT },
{ _("/Options/Cell Preferences ..."), "F3",	pressed_edit_cell_prefs, 0, "<StockItem>", GTK_STOCK_PROPERTIES },
{ _("/Options/Book Preferences ..."), "<control>F3", pressed_edit_book_prefs, 0, NULL, NULL },
{ _("/Options/Program Preferences ..."), "<control>P", pressed_preferences, 0, "<StockItem>", GTK_STOCK_PREFERENCES },
{ _("/Options/sep2"),		NULL,		NULL, 0, "<Separator>", NULL },
{ _("/Options/Bold"),		"<control>B",	pressed_bold, 0, "<StockItem>", GTK_STOCK_BOLD },
{ _("/Options/Background Colour ..."), "F8",	pressed_change_color, 0, "<StockItem>", GTK_STOCK_SELECT_COLOR },
{ _("/Options/Foreground Colour ..."), "F9",	pressed_change_color, 1, "<StockItem>", GTK_STOCK_COLOR_PICKER },
{ _("/Options/Border Colour ..."), NULL,	pressed_change_color, 2, NULL, NULL },
{ _("/Options/Border Type"), NULL,		NULL, 0, "<Branch>", NULL },
{ _("/Options/Border Type/tear"), NULL,		NULL, 0, "<Tearoff>", NULL },
{ _("/Options/Border Type/None"), NULL,		pressed_border_type, (guint)CUI_CELLBORD_NONE, NULL, NULL },
{ _("/Options/Border Type/sep"), NULL,		NULL, 0, "<Separator>", NULL },
{ _("/Options/Border Type/Thin Outside"), NULL,	pressed_border_type, (guint)CUI_CELLBORD_OUT_THIN, NULL, NULL },
{ _("/Options/Border Type/Thick Outside"), NULL, pressed_border_type, (guint)CUI_CELLBORD_OUT_THICK, NULL, NULL },
{ _("/Options/Border Type/Double Outside"), NULL, pressed_border_type, (guint)CUI_CELLBORD_OUT_DOUBLE, NULL, NULL },
{ _("/Options/Border Type/sep1"), NULL,		NULL, 0, "<Separator>", NULL },
{ _("/Options/Border Type/Thin Top & Bottom"), NULL, pressed_border_type, (guint)CUI_CELLBORD_TB_THIN, NULL, NULL },
{ _("/Options/Border Type/Thick Top & Bottom"), NULL, pressed_border_type, (guint)CUI_CELLBORD_TB_THICK, NULL, NULL },
{ _("/Options/Border Type/Double Top & Bottom"), NULL, pressed_border_type, (guint)CUI_CELLBORD_TB_DOUBLE, NULL, NULL },
{ _("/Options/Border Type/sep2"), NULL,		NULL, 0, "<Separator>", NULL },
{ _("/Options/Border Type/Horizontal"), NULL,	NULL, 0, "<Branch>", NULL },
{ _("/Options/Border Type/Horizontal/tear"), NULL, NULL, 0, "<Tearoff>", NULL },
{ _("/Options/Border Type/Horizontal/Clear Top"), NULL, pressed_border_type, CUI_CELLBORD_H_CLEAR_TOP, NULL, NULL },
{ _("/Options/Border Type/Horizontal/Clear Middle"), NULL, pressed_border_type, CUI_CELLBORD_H_CLEAR_MID, NULL, NULL },
{ _("/Options/Border Type/Horizontal/Clear Bottom"), NULL, pressed_border_type, CUI_CELLBORD_H_CLEAR_BOT, NULL, NULL },
{ _("/Options/Border Type/Horizontal/sep1"), NULL, NULL,0, "<Separator>", NULL },
{ _("/Options/Border Type/Horizontal/Thin Top"), NULL, pressed_border_type, CUI_CELLBORD_H_THIN_TOP, NULL, NULL },
{ _("/Options/Border Type/Horizontal/Thin Middle"), NULL, pressed_border_type, CUI_CELLBORD_H_THIN_MID, NULL, NULL },
{ _("/Options/Border Type/Horizontal/Thin Bottom"), NULL, pressed_border_type, CUI_CELLBORD_H_THIN_BOT, NULL, NULL },
{ _("/Options/Border Type/Horizontal/sep2"), NULL, NULL, 0, "<Separator>", NULL },
{ _("/Options/Border Type/Horizontal/Thick Top"), NULL, pressed_border_type, CUI_CELLBORD_H_THICK_TOP, NULL, NULL },
{ _("/Options/Border Type/Horizontal/Thick Middle"), NULL, pressed_border_type, CUI_CELLBORD_H_THICK_MID, NULL, NULL },
{ _("/Options/Border Type/Horizontal/Thick Bottom"), NULL, pressed_border_type, CUI_CELLBORD_H_THICK_BOT, NULL, NULL },
{ _("/Options/Border Type/Horizontal/sep3"), NULL, NULL, 0, "<Separator>", NULL },
{ _("/Options/Border Type/Horizontal/Double Top"), NULL, pressed_border_type, CUI_CELLBORD_H_DOUB_TOP, NULL, NULL },
{ _("/Options/Border Type/Horizontal/Double Middle"), NULL, pressed_border_type, CUI_CELLBORD_H_DOUB_MID, NULL, NULL },
{ _("/Options/Border Type/Horizontal/Double Bottom"), NULL, pressed_border_type, CUI_CELLBORD_H_DOUB_BOT, NULL, NULL },
{ _("/Options/Border Type/Vertical"), NULL,	NULL, 0, "<Branch>", NULL },
{ _("/Options/Border Type/Vertical/tear"), NULL, NULL, 0, "<Tearoff>", NULL },
{ _("/Options/Border Type/Vertical/Clear Left"), NULL, pressed_border_type, CUI_CELLBORD_V_CLEAR_L, NULL, NULL },
{ _("/Options/Border Type/Vertical/Clear Centre"), NULL, pressed_border_type, CUI_CELLBORD_V_CLEAR_C, NULL, NULL },
{ _("/Options/Border Type/Vertical/Clear Right"), NULL, pressed_border_type, CUI_CELLBORD_V_CLEAR_R, NULL, NULL },
{ _("/Options/Border Type/Vertical/sep1"), NULL, NULL, 0, "<Separator>", NULL },
{ _("/Options/Border Type/Vertical/Thin Left"), NULL, pressed_border_type, CUI_CELLBORD_V_THIN_L, NULL, NULL },
{ _("/Options/Border Type/Vertical/Thin Centre"), NULL, pressed_border_type, CUI_CELLBORD_V_THIN_C, NULL, NULL },
{ _("/Options/Border Type/Vertical/Thin Right"), NULL, pressed_border_type, CUI_CELLBORD_V_THIN_R, NULL, NULL },
{ _("/Options/Border Type/Vertical/sep2"), NULL, NULL, 0, "<Separator>", NULL },
{ _("/Options/Border Type/Vertical/Thick Left"), NULL, pressed_border_type, CUI_CELLBORD_V_THICK_L, NULL, NULL },
{ _("/Options/Border Type/Vertical/Thick Centre"), NULL, pressed_border_type, CUI_CELLBORD_V_THICK_C, NULL, NULL },
{ _("/Options/Border Type/Vertical/Thick Right"), NULL, pressed_border_type, CUI_CELLBORD_V_THICK_R, NULL, NULL },
{ _("/Options/Border Type/Vertical/sep3"), NULL, NULL, 0, "<Separator>", NULL },
{ _("/Options/Border Type/Vertical/Double Left"), NULL, pressed_border_type, CUI_CELLBORD_V_DOUB_L, NULL, NULL },
{ _("/Options/Border Type/Vertical/Double Centre"), NULL, pressed_border_type, CUI_CELLBORD_V_DOUB_C, NULL, NULL },
{ _("/Options/Border Type/Vertical/Double Right"), NULL, pressed_border_type, CUI_CELLBORD_V_DOUB_R, NULL, NULL },
{ _("/Options/sep4"),		NULL,		NULL, 0, "<Separator>", NULL },
{ _("/Options/Help ..."),	"F1",		pressed_help, 0, "<StockItem>", GTK_STOCK_HELP },
{ _("/Options/About ..."),	NULL,		pressed_about, 0, "<StockItem>", GTK_STOCK_ABOUT },
	};


	// Needed to read non ASCII filenames in GTK+2
	putenv ( "G_BROKEN_FILENAMES=1" );
	gtk_init ( NULL, NULL );

	accel_group = gtk_accel_group_new ();

///	MAIN WINDOW

	global.main_window = gtk_window_new ( GTK_WINDOW_TOPLEVEL );

	// Set minimum width/height
	gtk_widget_set_usize ( global.main_window, 100, 100 );

	mtgex_restore_window_position ( global.main_window, prefs_file (),
		GUI_INIFILE_MAIN_WINDOW );
	gtk_window_set_title ( GTK_WINDOW ( global.main_window ), VERSION );

	if ( 0 == prefs_get_int ( GUI_INIFILE_MAIN_WINDOW"_state" ) )
	{
		gtk_window_maximize ( GTK_WINDOW ( global.main_window ) );
	}

	g_signal_connect ( G_OBJECT ( global.main_window ),
		"window-state-event",
		G_CALLBACK ( window_state_event ), NULL );

	vbox_main = gtk_vbox_new ( FALSE, 0 );
	gtk_widget_show ( vbox_main );
	gtk_container_add ( GTK_CONTAINER ( global.main_window ), vbox_main );

	g_signal_connect ( G_OBJECT ( vbox_main ), "key_press_event",
		G_CALLBACK ( key_event_escape ), NULL );

///	MENU

	hbox = gtk_hbox_new ( FALSE, 0 );
	hbox_top = hbox;
	gtk_widget_show ( hbox );
	gtk_box_pack_start ( GTK_BOX ( vbox_main ), hbox, FALSE, FALSE, 0 );

	item_factory = gtk_item_factory_new ( GTK_TYPE_MENU_BAR, "<main>",
		accel_group );
	gtk_item_factory_create_items_ac ( item_factory,
		( sizeof ( menu_items ) / sizeof ( menu_items[0] ) ),
		menu_items, NULL, 2 );

	menubar = gtk_item_factory_get_widget ( item_factory, "<main>" );
	gtk_widget_show ( menubar );
	gtk_box_pack_start ( GTK_BOX ( hbox ), menubar, FALSE, FALSE, 0 );

	for ( i = 0; i < _MENU_TOTAL; i++ )
	{
		global.menu_widgets[i] = gtk_item_factory_get_item (
			item_factory, menu_refdef[i] );
	}

	entry = mtgex_bmenu_new ();
	global.bmenu_sheet = entry;
	enable_sheet_selector_cb ();
	gtk_box_pack_start ( GTK_BOX ( hbox ), entry, TRUE, FALSE, 0 );
	gtk_widget_add_accelerator ( entry, "clicked", accel_group, GDK_Pause,
		0, (GtkAccelFlags) 0 );

///	MAIN PANE

	pane = gtk_hpaned_new ();
	global.pane_main = pane;
	gtk_widget_show ( pane );
	gtk_box_pack_start ( GTK_BOX ( vbox_main ), pane, TRUE, TRUE, 0 );

	vbox_main = gtk_vbox_new ( FALSE, 0 );
	gtk_widget_show ( vbox_main );
	gtk_paned_pack1 ( GTK_PANED ( pane ), vbox_main, FALSE, TRUE );

///	ENTRY AREA

	hbox = gtk_hbox_new ( FALSE, 0 );
	gtk_widget_show ( hbox );
	gtk_box_pack_start ( GTK_BOX ( vbox_main ), hbox, FALSE, FALSE, 0 );

	entry = gtk_entry_new ();
	gtk_widget_show ( entry );
	gtk_entry_set_alignment ( GTK_ENTRY ( entry ), 0.5 );
	gtk_box_pack_start ( GTK_BOX ( hbox ), entry, FALSE, FALSE, 0 );
	global.entry_cellref = entry;

	gtk_signal_connect ( GTK_OBJECT ( entry ), "key_press_event",
		GTK_SIGNAL_FUNC ( cellref_entry_key_event ), NULL );
	gtk_signal_connect ( GTK_OBJECT ( entry ), "activate",
		GTK_SIGNAL_FUNC (cellref_entry_activate ), NULL );
	g_signal_connect ( G_OBJECT ( entry ), "focus_in_event",
		G_CALLBACK (cellref_entry_focus_in ), NULL );
	g_signal_connect ( G_OBJECT ( entry ), "focus_out_event",
		G_CALLBACK ( cellref_entry_focus_out ), NULL );

	entry = gtk_entry_new ();
	gtk_widget_show ( entry );
	gtk_box_pack_start ( GTK_BOX ( hbox ), entry, TRUE, TRUE, 0 );
	global.entry_celltext = entry;

	gtk_signal_connect ( GTK_OBJECT ( entry ), "key_press_event",
		GTK_SIGNAL_FUNC ( celltext_entry_key_event ), NULL );
	g_signal_connect ( G_OBJECT ( entry ), "focus_in_event",
		G_CALLBACK (celltext_entry_focus_in ), NULL );
	g_signal_connect ( G_OBJECT ( entry ), "focus_out_event",
		G_CALLBACK ( celltext_entry_focus_out ), NULL );

	label = gtk_label_new ( "0" );
	gtk_label_set_width_chars ( GTK_LABEL ( label ), 15 );
	gtk_misc_set_alignment ( GTK_MISC ( label ), 1.0, 0.5 );
	gtk_box_pack_end ( GTK_BOX ( hbox ), label, FALSE, FALSE, 5 );
	global.label_sum = label;

///	MAIN AREA

	table = ced_view_new ();
	gtk_box_pack_start ( GTK_BOX ( vbox_main ), table, TRUE, TRUE, 0 );
	global.table = table;
	ced_view_get ( table, &view );

	if ( ! view )
	{
		return;
	}

	global.cedview = view;

	for ( i = 0; i < CEDVIEW_AREA_TOTAL; i++ )
	{
		g_signal_connect ( view->area[i], "scroll_event",
			G_CALLBACK ( scroll_wheel ), view );

		if (	i >= CEDVIEW_AREA_TL &&
			i <= CEDVIEW_AREA_BR
			)
		{
			g_signal_connect ( view->area[i], "button_press_event",
				G_CALLBACK ( button_press_canvas ), view );
			g_signal_connect ( view->area[i], "motion_notify_event",
				G_CALLBACK ( motion_notify_canvas ), view );
		}

		if (	i >= CEDVIEW_TITLE_C1 &&
			i <= CEDVIEW_TITLE_R2
			)
		{
			g_signal_connect ( view->area[i], "button_press_event",
				G_CALLBACK ( button_press_header ), view );
			g_signal_connect ( view->area[i], "motion_notify_event",
				G_CALLBACK ( motion_notify_header ), view );
		}
	}

	g_signal_connect ( G_OBJECT ( table ), "key_press_event",
		G_CALLBACK ( key_event ), NULL );

///	PANE PAIR

	global.pane_pair = gtk_hpaned_new ();
	gtk_paned_pack2 ( GTK_PANED ( pane ), global.pane_pair, FALSE, TRUE );

///	FIND PANE

	vbox = gtk_vbox_new ( FALSE, 0 );
	global.pane_find = vbox;
	gtk_paned_pack1 ( GTK_PANED ( global.pane_pair ), vbox, FALSE, TRUE );
	find_gui_build ( vbox );

///	GRAPH PANE
	vbox = gtk_vbox_new ( FALSE, 0 );
	global.pane_graph = vbox;
	gtk_paned_pack2 ( GTK_PANED ( global.pane_pair ), vbox, FALSE, TRUE );
	graph_gui_build ( vbox, hbox_top, item_factory );

///	QUICKSUM

	entry = mtgex_bmenu_new ();
	mtgex_bmenu_add_item ( entry, "None" );
	mtgex_bmenu_add_item ( entry, "Sum" );
	mtgex_bmenu_add_item ( entry, "Min" );
	mtgex_bmenu_add_item ( entry, "Max" );
	mtgex_bmenu_add_item ( entry, "Max - Min" );
	mtgex_bmenu_add_item ( entry, "Average" );
	mtgex_bmenu_add_item ( entry, "Median" );
	mtgex_bmenu_add_item ( entry, "Count" );
	mtgex_bmenu_add_item ( entry, "Counta" );
	gtk_box_pack_end ( GTK_BOX ( hbox_top ), entry, FALSE, FALSE, 0 );
	mtgex_bmenu_set_callback ( entry, quicksum_bmenu_changed, NULL );

///	MISC

	grab_focus_sheet ();

	gtk_signal_connect ( GTK_OBJECT ( global.main_window ), "delete_event",
		GTK_SIGNAL_FUNC ( delete_event ), NULL );

	g_signal_connect ( G_OBJECT ( view->area[CEDVIEW_AREA_CORNER] ),
		"focus_in_event", G_CALLBACK ( view_focus_in ), NULL );
	g_signal_connect ( G_OBJECT ( view->area[CEDVIEW_AREA_CORNER] ),
		"focus_out_event", G_CALLBACK ( view_focus_out ), NULL );

	pref_change_font ( NULL, 0, NULL );
	pref_init_row_pad ();
	update_recent_files ();

	// Stop dynamic allocation of accelerators during runtime
	gtk_accel_group_lock ( accel_group );
	gtk_window_add_accel_group ( GTK_WINDOW ( global.main_window ),
		accel_group );

	gtk_widget_show ( global.main_window );

	gtk_window_set_icon ( (GtkWindow *)global.main_window,
		gdk_pixbuf_new_from_xpm_data ( icon_xpm ) );

	if (	! arg_filename ||
		project_load ( arg_filename )
		)
	{
		clear_all ();
	}

	gtk_main ();
}

int main (
	int			const	argc,
	char	const * const *	const	argv
	)
{
	char	const *	prefs_filename = NULL;
	char	const *	input_filename = NULL;


	be_cline ( argc, argv, &prefs_filename, &input_filename );

	ced_init ();

	prefs_init ( prefs_filename );

	mtgex_fpick_init_prefs ( prefs_file (), GUI_INIFILE_MAIN_FPICK_PRE );
	mtgex_prefs_init_prefs ( prefs_file () );

	prefs_load ();

	global.file = cui_file_new ();
	global.clipboard = cui_clip_new ();

	if (	global.file		&&
		global.clipboard	&&
		! cui_file_book_new ( global.file )
		)
	{
		// Create main window and run the GUI

		gui_main_init ( input_filename );
	}

	prefs_save ();
	prefs_close ();

	cui_font_destroy ( global.font );

	cui_clip_free ( global.clipboard );
	cui_file_free ( global.file );

	memset ( &global, 0, sizeof ( global ) );

	return 0;
}

